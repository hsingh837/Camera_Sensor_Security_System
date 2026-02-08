//Redeveloping motion sensor to use 2 webcams and accordingly adjusting output
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

/*
    Program 2 (Dual-Camera, Non-Threaded) — C++ Motion Sensor

    Goals (mirrors the Python evolution):
    - Camera 0 is REQUIRED
    - Camera 1 is OPTIONAL (program must still run if it’s missing)
    - Record video from any available camera(s)
    - Run motion sensor only when recording (press 'm' after 'r')
    - Log 1 row per second to CSV:
        - If 1 cam:  Second,Cam1
        - If 2 cams: Second,Cam1,Cam2
      where each camera column is Motion / No motion
    - Auto-terminate after 120 seconds once the motion sensor starts

    NOTE: This version is intentionally NON-threaded.
          We will introduce threading in Program 3 after correctness is proven.
*/

// Find the next available sequential index for files like BaseName1.ext, BaseName2.ext, ...
static int getNextIndex(const fs::path& dir, const string& baseName, const string& extWithDot)
{
    int maxIdx = 0;
    if (!fs::exists(dir)) return 1;

    // Example: baseName="Cam1_OutputVideo", extWithDot=".mp4"
    // pattern becomes: ^Cam1_OutputVideo(\d+)\.mp4$
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
    // ---------------------------------------------------------------------
    // Output folders (Program 2 keeps your current behavior: relative to CWD)
    // Program 4+ can anchor these to the exe path like we discussed later.
    // ---------------------------------------------------------------------
    fs::path videoDir = fs::path("./Output Videos");
    fs::path dataDir  = fs::path("./Output Data");
    fs::create_directories(videoDir);
    fs::create_directories(dataDir);

    // ---------------------------------------------------------------------
    // Camera setup
    // ---------------------------------------------------------------------
    VideoCapture cap1(0);             // REQUIRED
    VideoCapture cap2(1);             // OPTIONAL
    bool cam2Available = cap2.isOpened();

    if (!cap1.isOpened())
    {
        cerr << "ERROR! Unable to open camera 0 (required)\n";
        return -1;
    }

    if (!cam2Available)
    {
        cout << "Camera 1 not detected. Running in single-camera mode.\n";
    }

    Mat src1, src2;

    // Grab one frame from Cam1 to establish size/type
    cap1 >> src1;
    if (src1.empty())
    {
        cerr << "ERROR! blank frame grabbed from camera 0\n";
        return -1;
    }

    // If Cam2 exists, grab one frame to confirm it's actually producing frames
    if (cam2Available)
    {
        cap2 >> src2;
        if (src2.empty())
        {
            cout << "Camera 1 opened but returned an empty frame. Disabling Cam2.\n";
            cam2Available = false;
        }
    }

    bool isColor1 = (src1.type() == CV_8UC3);
    bool isColor2 = cam2Available ? (src2.type() == CV_8UC3) : false;

    // ---------------------------------------------------------------------
    // Recording / motion sensor state
    // ---------------------------------------------------------------------
    bool recordingOn = false;
    bool motionOn = false;

    VideoWriter writer1;
    VideoWriter writer2; // only used if cam2Available at recording start
    ofstream csv;

    // ---------------------------------------------------------------------
    // Timing (single authoritative clock for per-second logging)
    // ---------------------------------------------------------------------
    using clock_t = std::chrono::steady_clock;
    clock_t::time_point motionStartTime{};
    clock_t::time_point lastSecondTick{};

    int secondsLogged = 0;                 // 1..120
    bool motionDetectedCam1ThisSecond = false;
    bool motionDetectedCam2ThisSecond = false;

    // ---------------------------------------------------------------------
    // Motion detection baseline (per camera)
    // ---------------------------------------------------------------------
    Mat prevGray1, prevGray2;
    Mat gray1, gray2, diff1, diff2, thresh1, thresh2;

    // --- Tunables for "SIGNIFICANT movement"
    // These are intentionally explicit and easy to tweak.
    const int    DIFF_THRESH  = 25;    // pixel intensity change threshold (0..255)
    const double MOTION_RATIO = 0.02;  // fraction of pixels changed (2%) counts as motion
    // ---

    cout << "Controls:\n"
         << "  r = start recording (records Cam0 always, Cam1 if present)\n"
         << "  m = start motion sensor (only while recording; runs up to 120s then exits)\n"
         << "  ESC = exit early\n";

    // ---------------------------------------------------------------------
    // Main loop (NON-threaded): we read frames directly in this loop
    // ---------------------------------------------------------------------
    for (;;)
    {
        // ---- Read camera 0 (required)
        if (!cap1.read(src1) || src1.empty())
        {
            cerr << "ERROR! blank frame grabbed from camera 0\n";
            break;
        }

        // ---- Read camera 1 (optional)
        if (cam2Available)
        {
            if (!cap2.read(src2) || src2.empty())
            {
                // If Cam2 stops producing frames, we gracefully disable it
                cout << "Camera 1 stopped producing frames. Disabling Cam2.\n";
                cam2Available = false;

                // If we were recording Cam2, we release its writer cleanly
                if (writer2.isOpened())
                    writer2.release();
            }
        }

        // ---- Always show live feed(s)
        imshow("Cam1 Live (Camera 0)", src1);
        if (cam2Available)
            imshow("Cam2 Live (Camera 1)", src2);

        // ---- Key input
        int key = waitKey(1);

        if (key == 27) // ESC
        {
            cout << "ESC pressed. Exiting early.\n";
            break;
        }

        // -----------------------------------------------------------------
        // Start recording ('r')
        // We open writer(s) here, and then write every frame while recordingOn.
        // -----------------------------------------------------------------
        if (!recordingOn && (key == 'r' || key == 'R'))
        {
            // Independent sequential indexes for each camera’s video
            int nextVid1 = getNextIndex(videoDir, "Cam1_OutputVideo", ".mp4");
            fs::path videoPath1 = videoDir / ("Cam1_OutputVideo" + to_string(nextVid1) + ".mp4");

            // For Cam2, we only create a path if Cam2 is currently available
            fs::path videoPath2;
            if (cam2Available)
            {
                int nextVid2 = getNextIndex(videoDir, "Cam2_OutputVideo", ".mp4");
                videoPath2 = videoDir / ("Cam2_OutputVideo" + to_string(nextVid2) + ".mp4");
            }

            int codec = VideoWriter::fourcc('m', 'p', '4', 'v');

            // In Program 2, we keep fps fixed like your baseline.
            // In Program 3 (threaded), we can measure/derive fps more accurately.
            double fps = 60.0;

            // Open writer for Cam1
            writer1.open(videoPath1.string(), codec, fps, src1.size(), isColor1);
            if (!writer1.isOpened())
            {
                cerr << "Could not open Cam1 output video for write\n";
                return -1;
            }

            // Open writer for Cam2 if available
            if (cam2Available)
            {
                writer2.open(videoPath2.string(), codec, fps, src2.size(), isColor2);
                if (!writer2.isOpened())
                {
                    cout << "Warning: Could not open Cam2 output video. Continuing with Cam1 only.\n";
                    cam2Available = false; // treat as disabled for recording/sensing
                }
            }

            recordingOn = true;
            cout << "Recording started:\n"
                 << "  Cam1 -> " << videoPath1.string() << "\n";
            if (cam2Available)
                cout << "  Cam2 -> " << videoPath2.string() << "\n";
        }

        // -----------------------------------------------------------------
        // Start motion sensor ('m') — only while recording
        // CSV format adapts based on whether Cam2 is available at start.
        // -----------------------------------------------------------------
        if (!motionOn && recordingOn && (key == 'm' || key == 'M'))
        {
            int nextData = getNextIndex(dataDir, "MotionLog", ".csv");
            fs::path dataPath = dataDir / ("MotionLog" + to_string(nextData) + ".csv");

            csv.open(dataPath.string(), ios::out);
            if (!csv.is_open())
            {
                cerr << "Could not open CSV for write\n";
                return -1;
            }

            // Header adapts to camera availability
            if (cam2Available)
                csv << "Second,Cam1,Cam2\n";
            else
                csv << "Second,Cam1\n";

            motionOn = true;
            motionStartTime = clock_t::now();
            lastSecondTick = motionStartTime;

            secondsLogged = 0;
            motionDetectedCam1ThisSecond = false;
            motionDetectedCam2ThisSecond = false;

            // Initialize baselines from the current frames
            cvtColor(src1, prevGray1, COLOR_BGR2GRAY);
            if (cam2Available)
                cvtColor(src2, prevGray2, COLOR_BGR2GRAY);

            cout << "Motion sensor started. Logging to: " << dataPath.string() << "\n";
            cout << "Will auto-terminate after 2 minutes (120 seconds).\n";
        }

        // -----------------------------------------------------------------
        // If recording, write every frame for any available writers
        // -----------------------------------------------------------------
        if (recordingOn)
        {
            writer1.write(src1);
            if (cam2Available && writer2.isOpened())
                writer2.write(src2);
        }

        // -----------------------------------------------------------------
        // Motion detection + CSV logging only while motion sensor is active
        // -----------------------------------------------------------------
        if (motionOn)
        {
            // ---- Cam1 motion detection
            cvtColor(src1, gray1, COLOR_BGR2GRAY);
            absdiff(gray1, prevGray1, diff1);
            threshold(diff1, thresh1, DIFF_THRESH, 255, THRESH_BINARY);

            int changed1 = countNonZero(thresh1);
            int total1 = thresh1.rows * thresh1.cols;
            double ratio1 = (total1 > 0) ? (double)changed1 / (double)total1 : 0.0;

            if (ratio1 >= MOTION_RATIO)
                motionDetectedCam1ThisSecond = true;

            prevGray1 = gray1.clone();

            // ---- Cam2 motion detection (only if available)
            if (cam2Available)
            {
                cvtColor(src2, gray2, COLOR_BGR2GRAY);
                absdiff(gray2, prevGray2, diff2);
                threshold(diff2, thresh2, DIFF_THRESH, 255, THRESH_BINARY);

                int changed2 = countNonZero(thresh2);
                int total2 = thresh2.rows * thresh2.cols;
                double ratio2 = (total2 > 0) ? (double)changed2 / (double)total2 : 0.0;

                if (ratio2 >= MOTION_RATIO)
                    motionDetectedCam2ThisSecond = true;

                prevGray2 = gray2.clone();
            }

            // ---- Every ~1 second, write one CSV row
            auto now = clock_t::now();
            auto elapsedSinceTickMs =
                std::chrono::duration_cast<std::chrono::milliseconds>(now - lastSecondTick).count();

            if (elapsedSinceTickMs >= 1000)
            {
                secondsLogged += 1;

                const string cam1Status = (motionDetectedCam1ThisSecond ? "Motion" : "No motion");
                if (cam2Available)
                {
                    const string cam2Status = (motionDetectedCam2ThisSecond ? "Motion" : "No motion");

                    // CSV row matches what we'd like to see in terminal output
                    csv << secondsLogged << "," << cam1Status << "," << cam2Status << "\n";
                    cout << secondsLogged << "," << cam1Status << "," << cam2Status << "\n";
                }
                else
                {
                    csv << secondsLogged << "," << cam1Status << "\n";
                    cout << secondsLogged << "," << cam1Status << "\n";
                }

                // Reset 1-second window accumulation flags
                motionDetectedCam1ThisSecond = false;
                motionDetectedCam2ThisSecond = false;

                lastSecondTick = now;
            }

            // Auto-terminate after 120 seconds (based on seconds logged)
            if (secondsLogged >= 120)
            {
                cout << "2 minutes (120 seconds) complete. Auto-terminating.\n";
                break;
            }
        }
    }

    // ---------------------------------------------------------------------
    // Cleanup (explicit, consistent with your current style)
    // ---------------------------------------------------------------------
    if (csv.is_open()) csv.close();
    if (writer1.isOpened()) writer1.release();
    if (writer2.isOpened()) writer2.release();
    cap1.release();
    if (cam2Available) cap2.release();
    destroyAllWindows();

    return 0;
}
