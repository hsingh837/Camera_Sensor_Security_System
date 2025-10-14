//Same idea as Recording.py, but we want to write it in C++

#include <opencv2/opencv.hpp>
#include <filesystem>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

int main() {
    //Output directory must exist in current working directory, ensuring that is true
    const std::string outDir = "Videos";
    try {
        fs::create_directories(outDir);
    
    } catch (const std::exception& e) {
        std::cerr << "Failed to create/verify output directory: " <<e.what() << std::endl;
        return 1;
    }

    //Opening Default Camera
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Cannot Open Camera" << std::endl;
        return 1;

    }

    //Initialize Codec/State
    int fourcc = cv::VideoWriter::fourcc('m','p','4','v');
    cv::VideoWriter out;
    bool recording = false;
    std::string filename;

    //Actual main part
    while (true) {
        cv::Mat frame;
        if (!cap.read(frame)) {
            std::cerr << "No frame, exiting" << std:: endl;
            break;
        }

        //showing the live preview itself
        cv::imshow("frame", frame);
        int key = cv::waitKey(1) & 0xFF;

        //Start recording on keypress r
        if (key == 'r' && !recording) {
            //Next index is based on existing output # of .mp4 files
            size_t count = 0;
            try {
                for (const auto& entry : fs::directory_iterator(outDir)) {
                    if (!entry.is_regular_file()) continue;
                    const std::string name = entry.path().filename().string();
                    if (name.rfind("Output", 0) == 0 && name.size() >= 9 && name.substr(name.size()-4) == ".mp4") {
                        ++count;
                    }
                }
            } catch (const std::exception& e) {
                std::cerr << "Directory iteration error: " << e.what() << std::endl;
            }
            const size_t nextIndex = count + 1;
            filename = (fs::path(outDir) / ("Output" + std::to_string(nextIndex) + ".mp4")).string();

            out.open(filename, fourcc, 20.0, frame.size(), true);
            if(!out.isOpened()) {
                std::cerr << "Failed to open VideoWriter for: " << filename << std::endl;
            }
        }
        //Stop recording & exit camera on key 27 (ESC key)
        else if (key == 27) {
            if (recording) {
                recording = false;
                out.release();
                std::cout << "Recording Stopped: " << filename << std::endl;
            }
            break;
        }
        //Write frames while recording
        if (recording) {
            out.write(frame);
        }
    }
    //Once finished, we can 'cleanup' by closing windows and stopping recording completely
    cap.release();
    if (out.isOpened()) out.release();
    cv::destroyAllWindows();

    return 0;
}