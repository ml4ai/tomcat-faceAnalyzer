#include "InotifySequenceCapture.h"

#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/inotify.h>

using namespace Utilities;

#define MAX_EVENT_MONITOR 2048
#define NAME_LEN 32
#define MONITOR_EVENT_SIZE (sizeof(struct inotify_event))
#define BUFFER_LEN MAX_EVENT_MONITOR*(MONITOR_EVENT_SIZE+NAME_LEN)  
#define SLEEP_TIME_MICROSECONDS 50  

void InotifySequenceCapture::initPath(const std::string &path) {
    this->monitorPath = path;
    this->fd = inotify_init();
    
    if (fd < 0)
	    std::cerr << "Notify did not initialize";

    this->watch_desc = inotify_add_watch(this->fd, path.c_str(), IN_CREATE);

    if (watch_desc == -1)
	    std::cerr << "Couldn't add watch to the path" << std::endl;
}

cv::Mat InotifySequenceCapture::GetNextFrame() {
	char buffer[BUFFER_LEN];
	std::string filename = "";
	do {
		int total_read = read (this->fd, buffer, BUFFER_LEN);
		if (total_read < 0)
			printf("Read error");

		int i = 0;
		filename = "";
		
		while(i < total_read) {
			struct inotify_event *event = (struct inotify_event*) &buffer[i];
			if (event->len) {
				if (event->mask & IN_CREATE) {
					if (event->mask & IN_ISDIR)
						std::cout << "This is a directory";
					else
						filename = event->name;
				}
				i += MONITOR_EVENT_SIZE + event->len;
			}
		}
	} while(!(filename.substr(filename.find_last_of(".") + 1) == "png"));
    
    // Use the filename variable here to get the name of the file
    //if (filename[0] == '.')
	//    continue;

    filename = this->monitorPath + "/" + filename;

    cv::Mat retImg;
    
    do {
    	retImg = cv::imread(filename);
	    usleep(SLEEP_TIME_MICROSECONDS);
    } while(retImg.empty());
    
    return retImg;
}

void InotifySequenceCapture::closeInotify() {
    // Close the inotify watch
	inotify_rm_watch(this->fd, this->watch_desc);
	close(this->fd);
}
	    
	    
