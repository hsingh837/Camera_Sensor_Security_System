// Program 3 (Dual-Camera, THREADED) — C++ Motion Sensor
// Same behavior as Program 2, but camera I/O is moved into background threads.
// This removes blocking reads from the main loop and improves timing stability.

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

#include <thread>
#include <mutex>
#include <atomic>

using namespace cv;
using namespace std;
namespace fs = std::filesystem;

// ------------------------------------------------------------
// Utility: find next available sequential index for BaseName#.ext
// ------------------------------------------------------------
static int getNextIndex(const fs::path& dir, const string& baseName, const string& extWithDot)
{
    int maxIdx = 0;
    if (!fs::exists(dir)) return 1;

    const regex pat("^" + baseName + R"((\d+))" + "\\" + extWithDot + "$");

    for (const auto& entry : fs::directory_iterator(dir))
    {
        if (!entry.is_regular_file()) continue;

        const string fname = entry.path().filename().string();
        smatch m;
        if (regex_match(fname, m, pat) && m.size() == 2)
        {
            int idx = 0;
            try { idx = stoi(m[1].str()); } catch (...) { idx = 0; }
            maxIdx = max(maxIdx, idx);
        }
    }
    return maxIdx + 1;
}

// ============================================================
// Threaded camera capture class
// ============================================================
//
// Why this exists:
// - VideoCapture::read() can block unpredictably (USB hiccups, driver latency)
// - In non-threaded designs, a slow camera can stall the entire loop
// - Here, each camera captures frames in its own thread
// - The main loop always reads "the latest frame" without waiting
//
class CameraStream
{
public:
    explicit CameraStream(int index)
        : camIndex(index), running(false), ok(false), newFrame(false)
    {
        cap.open(index);
        if (!cap.isOpened())
        {
            ok = false;
            return;
        }

        // Warm start: grab one frame so consumers have something immediately.
        Mat tmp;
        if (cap.read(tmp) && !tmp.empty())
        {
            lock_guard<mutex> lk(mtx);
            frame = tmp;
            ok = true;
            newFrame = true;
        }
        else
        {
            ok = false;
            cap.release();
            return;
        }

        running = true;
        th = thread(&CameraStream::loop, this);
    }

    // Non-copyable (threads + mutex)
    CameraStream(const CameraStream&) = delete;
    CameraStream& operator=(const CameraStream&) = delete;

    ~CameraStream()
    {
        stop();
    }

    bool isOk() const { return ok; }

    // Grab a snapshot of the latest frame.
    // Returns false if stream is not OK.
    bool read(Mat& out, bool* outIsNew = nullptr)
    {
        if (!ok) return false;

        lock_guard<mutex> lk(mtx);
        if (frame.empty()) return false;

        frame.copyTo(out);

        if (outIsNew)
        {
            *outIsNew = newFrame;
        }
        newFrame = false;

        return true;
    }

    void stop()
    {
        if (!running) return;

        running = false;
        if (th.joinable()) th.join();

        if (cap.isOpened()) cap.release();
    }

    // Optional: access to capture props if needed later
    double get(int propId) const
    {
        if (!cap.isOpened()) return 0.0;
        return cap.get(propId);
    }

private:
    void loop()
    {
        // Capture loop: keep reading frames in background.
        // If read fails repeatedly, we mark the stream as not OK.
        int consecutiveFails = 0;

        while (running)
        {
            Mat tmp;
            bool ret = cap.read(tmp);

            if (!ret || tmp.empty())
            {
                consecutiveFails++;
                // If the camera disappears, stop treating it as available.
                if (consecutiveFails >= 30)
                {
                    ok = false;
                    break;
                }
                // Tiny sleep prevents spinning at 100% CPU on failure
                this_thread::sleep_for(chrono::milliseconds(5));
                continue;
            }

            consecutiveFails = 0;

            {
                lock_guard<mutex> lk(mtx);
                frame = tmp;      // latest frame wins
                newFrame = true;  // mark that consumer hasn't seen this one yet
            }
        }
    }

    int camIndex;
    mutable mutex mtx;
    VideoCapture cap;
    Mat frame;

    thread th;
    atomic<bool> running;
    atomic<bool> ok;

    bool newFrame; // protected by mtx
};

// ============================================================
// Program 3 main
// ============================================================

int main(int argc, char** argv)
{
    // ---------------------------------------------------------
    // Output folders (still relative to CWD in Program 3)
    // Next upgrade will anchor these relative to the executable.
    // ---------------------------------------------------------
    fs::path videoDir = fs::path("./Output Videos");
    fs::path dataDir  = fs::path("./Output Data");
    fs::create_directories(videoDir);
    fs::create_directories(dataDir);

    // ---------------------------------------------------------
    // Start threaded camera streams
    // ---------------------------------------------------------
    CameraStream cam1(0); // REQUIRED
    if (!cam1.isOk())
    {
        cerr << "ERROR! Unable to open camera 0 (required)\n";
        return -1;
    }

    unique_ptr<CameraStream> cam2;
    bool cam2Available = false;

    {
        auto tmp = make_unique<CameraStream>(1);
        if (tmp->isOk())
        {
            cam2 = std::move(tmp);
            cam2Available = true;
        }
        else
        {
            cout << "Camera 1 not detected. Running in single-camera mode.\n";
        }
    }

    // ---------------------------------------------------------
    // Grab an initial frame from Cam1 to establish size/type
    // ---------------------------------------------------------
    Mat src1, src2;
    if (!cam1.read(src1) || src1.empty())
    {
        cerr << "ERROR! Could not read initial frame from Cam1\n";
        return -1;
    }

    bool isColor1 = (src1.type() == CV_8UC3);

    bool isColor2 = false;
    if (cam2Available)
    {
        if (!cam2->read(src2) || src2.empty())
        {
            cout << "Cam2 opened but no initial frame. Disabling Cam2.\n";
            cam2Available = false;
            cam2.reset();
        }
        else
        {
            isColor2 = (src2.type() == CV_8UC3);
        }
    }

    // ---------------------------------------------------------
    // Recording / motion sensor state
    // ---------------------------------------------------------
    bool recordingOn = false;
    bool motionOn = false;

    VideoWriter writer1;
    VideoWriter writer2; // only if Cam2 remains available
    ofstream csv;

    // ---------------------------------------------------------
    // Timing (single authoritative clock)
    // ---------------------------------------------------------
    using clock_t = std::chrono::steady_clock;
    clock_t::time_point lastSecondTick{};

    int secondsLogged = 0; // 1..120
    bool motionDetectedCam1ThisSecond = false;
    bool motionDetectedCam2ThisSecond = false;

    // ---------------------------------------------------------
    // Motion detection baseline (per camera)
    // ---------------------------------------------------------
    Mat prevGray1, prevGray2;
    Mat gray1, gray2, diff1, diff2, thresh1, thresh2;

    const int    DIFF_THRESH  = 25;
    const double MOTION_RATIO = 0.02;

    cout << "Controls:\n"
         << "  r = start recording (records Cam0 always, Cam1 if present)\n"
         << "  m = start motion sensor (only while recording; runs up to 120s then exits)\n"
         << "  ESC = exit early\n";

    // ---------------------------------------------------------
    // Main loop
    // ---------------------------------------------------------
    for (;;)
    {
        // ---- Pull latest Cam1 frame (non-blocking snapshot)
        if (!cam1.read(src1) || src1.empty())
        {
            cerr << "ERROR! Cam1 stream stopped.\n";
            break;
        }

        // ---- Pull latest Cam2 frame if available
        if (cam2Available)
        {
            Mat tmp2;
            if (!cam2->read(tmp2) || tmp2.empty())
            {
                // Cam2 died mid-run: disable it gracefully (and keep going with Cam1)
                cout << "Camera 1 stopped producing frames. Disabling Cam2.\n";
                cam2Available = false;
                cam2.reset();

                if (writer2.isOpened())
                    writer2.release();

                // Also close Cam2 window if it exists
                try { destroyWindow("Cam2 Live (Camera 1)"); } catch (...) {}
            }
            else
            {
                src2 = tmp2;
            }
        }

        // ---- Show live feed(s)
        imshow("Cam1 Live (Camera 0)", src1);
        if (cam2Available)
            imshow("Cam2 Live (Camera 1)", src2);

        int key = waitKey(1);
        if (key == 27)
        {
            cout << "ESC pressed. Exiting early.\n";
            break;
        }

        // -----------------------------------------------------
        // Start recording
        // -----------------------------------------------------
        if (!recordingOn && (key == 'r' || key == 'R'))
        {
            int nextVid1 = getNextIndex(videoDir, "Cam1_OutputVideo", ".mp4");
            fs::path videoPath1 = videoDir / ("Cam1_OutputVideo" + to_string(nextVid1) + ".mp4");

            fs::path videoPath2;
            if (cam2Available)
            {
                int nextVid2 = getNextIndex(videoDir, "Cam2_OutputVideo", ".mp4");
                videoPath2 = videoDir / ("Cam2_OutputVideo" + to_string(nextVid2) + ".mp4");
            }

            int codec = VideoWriter::fourcc('m', 'p', '4', 'v');

            // Threaded capture doesn’t automatically guarantee 60 fps, but it reduces stalls.
            // For now we keep your simple fixed FPS. Later we can compute/write actual FPS.
            double fps = 60.0;

            writer1.open(videoPath1.string(), codec, fps, src1.size(), isColor1);
            if (!writer1.isOpened())
            {
                cerr << "Could not open Cam1 output video for write\n";
                return -1;
            }

            if (cam2Available)
            {
                writer2.open(videoPath2.string(), codec, fps, src2.size(), isColor2);
                if (!writer2.isOpened())
                {
                    cout << "Warning: Could not open Cam2 output video. Continuing with Cam1 only.\n";
                    cam2Available = false;
                    cam2.reset();
                }
            }

            recordingOn = true;
            cout << "Recording started:\n"
                 << "  Cam1 -> " << videoPath1.string() << "\n";
            if (cam2Available)
                cout << "  Cam2 -> " << videoPath2.string() << "\n";
        }

        // -----------------------------------------------------
        // Start motion sensor
        // -----------------------------------------------------
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

            if (cam2Available) csv << "Second,Cam1,Cam2\n";
            else              csv << "Second,Cam1\n";

            motionOn = true;
            lastSecondTick = clock_t::now();
            secondsLogged = 0;

            motionDetectedCam1ThisSecond = false;
            motionDetectedCam2ThisSecond = false;

            // Initialize baselines from current frames
            cvtColor(src1, prevGray1, COLOR_BGR2GRAY);
            if (cam2Available)
                cvtColor(src2, prevGray2, COLOR_BGR2GRAY);

            cout << "Motion sensor started. Logging to: " << dataPath.string() << "\n";
            cout << "Will auto-terminate after 2 minutes (120 seconds).\n";
        }

        // -----------------------------------------------------
        // Write frames to video(s)
        // -----------------------------------------------------
        if (recordingOn)
        {
            writer1.write(src1);
            if (cam2Available && writer2.isOpened())
                writer2.write(src2);
        }

        // -----------------------------------------------------
        // Motion detection + CSV logging
        // -----------------------------------------------------
        if (motionOn)
        {
            // Cam1 motion detection
            cvtColor(src1, gray1, COLOR_BGR2GRAY);
            absdiff(gray1, prevGray1, diff1);
            threshold(diff1, thresh1, DIFF_THRESH, 255, THRESH_BINARY);

            int changed1 = countNonZero(thresh1);
            int total1 = thresh1.rows * thresh1.cols;
            double ratio1 = (total1 > 0) ? (double)changed1 / (double)total1 : 0.0;

            if (ratio1 >= MOTION_RATIO)
                motionDetectedCam1ThisSecond = true;

            // NOTE: clone() is safe and simple. Later optimization:
            // reuse buffers or swap Mats to reduce allocations.
            prevGray1 = gray1.clone();

            // Cam2 motion detection (optional)
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

            // Per-second logging (same model as your Python program)
            auto now = clock_t::now();
            auto elapsedSinceTickMs =
                chrono::duration_cast<chrono::milliseconds>(now - lastSecondTick).count();

            if (elapsedSinceTickMs >= 1000)
            {
                secondsLogged += 1;

                const string cam1Status = motionDetectedCam1ThisSecond ? "Motion Detected" : "No motion";

                if (cam2Available)
                {
                    const string cam2Status = motionDetectedCam2ThisSecond ? "Motion Detected" : "No motion";
                    csv  << secondsLogged << "," << cam1Status << "," << cam2Status << "\n";
                    cout << secondsLogged << "," << cam1Status << "," << cam2Status << "\n";
                }
                else
                {
                    csv  << secondsLogged << "," << cam1Status << "\n";
                    cout << secondsLogged << "," << cam1Status << "\n";
                }

                motionDetectedCam1ThisSecond = false;
                motionDetectedCam2ThisSecond = false;
                lastSecondTick = now;
            }

            if (secondsLogged >= 120)
            {
                cout << "2 minutes (120 seconds) complete. Auto-terminating.\n";
                break;
            }
        }
    }

    // ---------------------------------------------------------
    // Cleanup
    // ---------------------------------------------------------
    if (csv.is_open()) csv.close();
    if (writer1.isOpened()) writer1.release();
    if (writer2.isOpened()) writer2.release();

    // Stop streams explicitly (also done in destructors, but explicit feels cleaner)
    cam1.stop();
    if (cam2Available && cam2) cam2->stop();

    destroyAllWindows();
    return 0;
}
