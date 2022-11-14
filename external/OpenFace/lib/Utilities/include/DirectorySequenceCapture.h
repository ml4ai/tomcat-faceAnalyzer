#pragma once

#include "boost/filesystem.hpp"
#include <opencv2/highgui/highgui.hpp>

using boost::filesystem::path;
using boost::filesystem::directory_iterator;

namespace Utilities {
    
    //=======================================================================
    /**
    A class for capturing images using inotify from a NFS directory
        This program waits for an image to be written to the NFS directory and
        uses this image file to generate and return a cv::Mat object
    */
    class DirectorySequenceCapture {

        private:
            path p;
            directory_iterator directory_itr;
            directory_iterator end_itr;
            std::string current_filename;
            std::string current_filepath;

        public:
            // Default constructor
            DirectorySequenceCapture(const std::string &path);

            // Wait for the next image file to get written, and use this image file 
            // to return a Mat object
            cv::Mat GetNextFrame();

            std::string getCurrentFileName();
            std::string getCurrentFilePath();

            std::string getFileModifiedTimestamp();
            std::string getFileTimestampFromName();

            path getMonitoringPath();
    };
}
