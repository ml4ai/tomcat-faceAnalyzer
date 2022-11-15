#include "DirectorySequenceCapture.h"
#include <iostream>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <iomanip>

using namespace Utilities;

DirectorySequenceCapture::DirectorySequenceCapture(const std::string &path):
    p(path), directory_itr(path) {}

cv::Mat DirectorySequenceCapture::GetNextFrame() {
    while (this->directory_itr != this->end_itr) {
        this->current_filepath = this->directory_itr->path().string();
        this->current_filename = this->directory_itr->path().stem().string();
        this->directory_itr++;

        if (this->current_filepath.find("png") != std::string::npos &&
            this->current_filepath.find("out") == std::string::npos) {
            auto readImage = cv::imread(this->current_filepath);
            if (!readImage.empty())
                return readImage;
        }

        std::cout << "Skipped " << this->current_filepath << std::endl;
    }

    this->current_filename = "";
    this->current_filepath = "";
    return cv::Mat();
}

std::string DirectorySequenceCapture::getCurrentFileName() {
    return this->current_filename;
}

std::string DirectorySequenceCapture::getCurrentFilePath() {
    return this->current_filepath;
}

std::string DirectorySequenceCapture::getFileTimestampFromName() {
    std::string timestamp_string = this->current_filename;
    std::replace( timestamp_string.begin(), timestamp_string.end(), '_', ':');
    return timestamp_string;
}

std::string DirectorySequenceCapture::getFileModifiedTimestamp() {
    struct stat t_stat;
    stat(this->getCurrentFilePath().c_str(), &t_stat);
    struct tm *time_now_tm = gmtime(&t_stat.st_mtime);

    char timestampBuffer[300];
    strftime(timestampBuffer, sizeof(timestampBuffer), "%FT%T", time_now_tm);

    std::stringstream timestampStream;
    timestampStream << timestampBuffer << '.' << std::setfill('0') << std::setw(9) << t_stat.st_mtim.tv_nsec << 'Z';

    return timestampStream.str();
}

path DirectorySequenceCapture::getMonitoringPath() {
    return this->p;
}
