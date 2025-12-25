#include <opencv2/opencv.hpp>
#include <iostream>


int main() {
    std::string imagePath = "9.jpg";
    //std::string imagePath = '8.jpg';
    cv::Mat image = cv::imread(imagePath, cv::IMREAD_COLOR);
    cv::imshow("Display Window", image);
    cv::waitKey(0);
    return 0;
}