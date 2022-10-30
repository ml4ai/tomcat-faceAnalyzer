#pragma once

#include <string>
#include <opencv2/highgui/highgui.hpp>

namespace Utilities {
    
    //=======================================================================
    /**
    A class for capturing images using inotify from a NFS directory
        This program waits for an image to be written to the NFS directory and
        uses this image file to generate and return a cv::Mat object
    */
    class InotifySequenceCapture {
        
        private:
            std::string monitorPath;
            int fd, watch_desc;
        
        public:

            // Default constructor
            InotifySequenceCapture() = default;
               
            // Initialize the inotify watch by passing it the path to the NFS directory
            void initPath(const std::string &path);
            
            // Wait for the next image file to get written, and use this image file 
            // to return a Mat object
            cv::Mat GetNextFrame();
            
            // Close the inotify watch on the given directory
            void closeInotify();
    };
}
