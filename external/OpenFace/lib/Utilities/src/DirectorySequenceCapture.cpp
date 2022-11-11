#include "DirectorySequenceCapture.h"
#include <iostream>

using namespace Utilities;

DirectorySequenceCapture::DirectorySequenceCapture(const std::string &path):
    p(path), directory_itr(path) {}

cv::Mat DirectorySequenceCapture::GetNextFrame() {
    while (this->directory_itr != this->end_itr) {
        this->current_filename = this->directory_itr->path().string();
        this->directory_itr++;

        if (this->current_filename.substr(this->current_filename.find_last_of(".") + 1) == "png") {
            auto readImage = cv::imread(this->current_filename);

            if (!readImage.empty())
                return readImage;
        }

        std::cout << "Skipped " << this->current_filename << std::endl;
    }

    this->current_filename = "";
    return cv::Mat();
}

std::string DirectorySequenceCapture::getCurrentFileName() {
    return this->current_filename;
}

path DirectorySequenceCapture::getMonitoringPath() {
    return this->p;
}
