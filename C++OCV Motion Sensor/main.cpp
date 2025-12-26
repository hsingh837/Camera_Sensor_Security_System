#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
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
