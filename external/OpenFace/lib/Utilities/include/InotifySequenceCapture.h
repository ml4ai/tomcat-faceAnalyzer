#pragma once

#include <string>
#include <opencv2/highgui/highgui.hpp>

namespace Utilities {
    class InotifySequenceCapture {
        
        private:
            std::string monitorPath;
            int fd, watch_desc;
        
        public:
            InotifySequenceCapture() = default;
                
            void initPath(const std::string &path);
            
            cv::Mat GetNextFrame();
            
            void closeInotify();
    };
}
