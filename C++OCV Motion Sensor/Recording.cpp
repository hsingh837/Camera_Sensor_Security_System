#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <stdio.h>

/* int main() {
    std::string imagePath = "photoname.jpg";

    cv::Mat image = cv::imread(imagePath, cv::IMREAD_COLOR);

    if (image.empty()) {
        std::cerr << "Error: Could not load image at path: " << imagePath << std::endl;
        return 1;
    }

    cv::imshow("Display Window", image);
    cv::waitKey(0);
    return 0;
}
*/

//Building the baseline code for recording here with openCV documentation

using namespace cv;
using namespace std;

int main(int, char**) {
    
    Mat src; //using default camera as srouce
    VideoCapture cap(0);
    if (!cap.isOpened()) { //error management, in case fetching default camera doesn't work
        cerr << "ERROR! Unable to open camera\n";
        return -1;
    }
    //pull in a frame to get size and type of frame
    cap >> src;
    if (src.empty()) { //error management, checking for success
        cerr << "ERROR! blank frame grabbed\n";
        return -1;
    }
    bool isColor = (src.type() == CV_8UC3);

    //Initializing the Videowriter to begin recording
    VideoWriter writer;
    int codec = VideoWriter::fourcc('m', 'p', '4', 'v'); //my desired codec is .mp4, however documentation utilizes MJPG
    double fps = 60.0; //framerate of created videostream
    string filename = "./Output Videos/live.mp4"; //name of output file
    writer.open(filename, codec, fps, src.size(), isColor);
    if (!writer.isOpened()) {
        cerr << "Could not open the output video file for write\n";
        return -1;
    }

    //Writing loop for pulling in frames into videostream for recording
    cout << "Writing videofile: " << filename << endl 
         << "Press any key to terminate" << endl;
    
    for (;;)
    {
        //checking for success (error management)
        if (!cap.read(src)) {
            cerr << "ERROR! blank frame grabbed\n";
            break;
        }

        //encoding frame into video file stream
        writer.write(src);
        //show live and wait for a keypress with 5 ms wait
        imshow("Live", src);
        
        if (waitKey(5) >= 0) {
            break;
        }
    }
    //The videofile will be closed and automatically released to working directory as executable
    return 0;
}   


