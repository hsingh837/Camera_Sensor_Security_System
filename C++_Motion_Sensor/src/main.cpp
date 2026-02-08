#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <filesystem>
#include <regex>

using namespace cv;
using namespace std;
namespace fs = std::filesystem;

// Find the next available sequential index for files like Video1.mp4, Video2.mp4, ...
static int getNextIndex(const fs::path& dir, const string& baseName, const string& extWithDot)
{
    int maxIdx = 0;

    if (!fs::exists(dir)) return 1;

    // Example pattern: ^Video(\d+)\.mp4$
    const regex pat("^" + baseName + R"((\d+))" + "\\" + extWithDot + "$");

    for (const auto& entry : fs::directory_iterator(dir))
    {
        if (!entry.is_regular_file()) continue;
        const string fname = entry.path().filename().string();

        smatch m;
        if (regex_match(fname, m, pat))
        {
            if (m.size() == 2)
            {
                int idx = 0;
                try { idx = stoi(m[1].str()); } catch (...) { idx = 0; }
                maxIdx = max(maxIdx, idx);
            }
        }
    }
    return maxIdx + 1;
}

int main(int, char**)
{
    // Ensure output folders exist (relative to the working directory / exe run directory)
    fs::path videoDir = fs::path("./Output Videos");
    fs::path dataDir  = fs::path("./Output Data");
    fs::create_directories(videoDir);
    fs::create_directories(dataDir);

    Mat src;

    // Use default camera as video source
    VideoCapture cap(0);

    // Check if camera opened successfully
    if (!cap.isOpened()) {
        cerr << "ERROR! Unable to open camera\n";
        return -1;
    }

    // Grab one frame to determine size/type
    cap >> src;
    if (src.empty()) {
        cerr << "ERROR! blank frame grabbed\n";
        return -1;
    }

    bool isColor = (src.type() == CV_8UC3);

    // Recording / motion sensor state
    bool recordingOn = false;
    bool motionOn = false;

    VideoWriter writer;
    ofstream csv;

    // Timing
    using clock_t = std::chrono::steady_clock;
    clock_t::time_point motionStartTime{};
    clock_t::time_point lastSecondTick{};

    int secondsLogged = 0;                 // 1..45
    bool motionDetectedThisSecond = false; // OR of motion detections within current second

    // Motion detection baseline
    Mat prevGray; // previous frame gray (used to measure change)
    Mat gray, diff, threshImg;

    // --- Tunables for "SIGNIFICANT movement"
    // May need to tweak these depending on camera noise/lighting. Is it possible to get these to tune automatically?
    const int    DIFF_THRESH = 25;   // pixel intensity change threshold (0..255)
    const double MOTION_RATIO = 0.02; // fraction of pixels changed to count as "motion" (2%)
    // ---

    cout << "Controls:\n"
         << "  r = start recording\n"
         << "  m = start motion sensor (only while recording; runs up to 45s then exits)\n"
         << "  ESC = exit early\n";

    for (;;)
    {
        if (!cap.read(src)) {
            cerr << "ERROR! blank frame grabbed\n";
            break;
        }

        // Always show live feed
        imshow("Live", src);

        // Handle key input
        int key = waitKey(1);

        // ESC terminates anytime
        if (key == 27) {
            cout << "ESC pressed. Exiting early.\n";
            break;
        }

        // Start recording on 'r'
        if (!recordingOn && (key == 'r' || key == 'R'))
        {
            int nextVid = getNextIndex(videoDir, "Video", ".mp4");
            fs::path videoPath = videoDir / ("Video" + to_string(nextVid) + ".mp4");

            int codec = VideoWriter::fourcc('m', 'p', '4', 'v');
            double fps = 60.0;

            writer.open(videoPath.string(), codec, fps, src.size(), isColor);
            if (!writer.isOpened()) {
                cerr << "Could not open the output video file for write\n";
                return -1;
            }

            recordingOn = true;
            cout << "Recording started: " << videoPath.string() << "\n";
        }

        // Start motion sensor on 'm' ONLY when recording
        if (!motionOn && recordingOn && (key == 'm' || key == 'M'))
        {
            int nextData = getNextIndex(dataDir, "Data", ".csv");
            fs::path dataPath = dataDir / ("Data" + to_string(nextData) + ".csv");

            csv.open(dataPath.string(), ios::out);
            if (!csv.is_open()) {
                cerr << "Could not open CSV for write\n";
                return -1;
            }

            // header row 
            csv << "Second,Status\n";

            motionOn = true;
            motionStartTime = clock_t::now();
            lastSecondTick = motionStartTime;

            secondsLogged = 0;
            motionDetectedThisSecond = false;

            // Initialize baseline
            cvtColor(src, prevGray, COLOR_BGR2GRAY);

            cout << "Motion sensor started. Logging to: " << dataPath.string() << "\n";
            cout << "Will auto-terminate after 45 seconds.\n";
        }

        // If recording, write every frame
        if (recordingOn) {
            writer.write(src);
        }

        // Motion detection + CSV logging only while motion sensor is active
        if (motionOn)
        {
            // Convert to grayscale
            cvtColor(src, gray, COLOR_BGR2GRAY);

            // Compute absolute difference vs previous frame
            absdiff(gray, prevGray, diff);

            // Threshold to keep only "meaningful" pixel changes
            threshold(diff, threshImg, DIFF_THRESH, 255, THRESH_BINARY);

            // How many pixels changed?
            int changed = countNonZero(threshImg);
            int totalPixels = threshImg.rows * threshImg.cols;

            double ratio = (totalPixels > 0) ? (double)changed / (double)totalPixels : 0.0;

            if (ratio >= MOTION_RATIO) {
                motionDetectedThisSecond = true;
            }

            // Update prev frame baseline for next loop
            prevGray = gray.clone();

            // Every 1 second: write one CSV row
            auto now = clock_t::now();
            auto elapsedSinceTick = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastSecondTick).count();

            if (elapsedSinceTick >= 1000)
            {
                secondsLogged += 1;

                csv << secondsLogged << ","
                    << (motionDetectedThisSecond ? "Motion Detected" : "No motion")
                    << "\n";
                
                //Printing what's going in the CSV in real time, to be consistent with the python Light Level Program
                cout << "[Sensor] t =" << secondsLogged
                     << "s -->"
                     << (motionDetectedThisSecond ? "Motion Detected": "No motion")
                     << endl;

                // Reset for next second window
                motionDetectedThisSecond = false;
                lastSecondTick = now;
            }

            // Auto-terminate after 120 seconds (based on seconds logged)
            if (secondsLogged >= 120) {
                cout << "2 minutes (120 seconds) complete. Auto-terminating.\n";
                break;
            }
        }
    }

    // Explicit Cleanup, essentially due diligence as writer does close as well
    if (csv.is_open()) csv.close();
    if (writer.isOpened()) writer.release();
    cap.release();
    destroyAllWindows();

    return 0;
}