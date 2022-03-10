#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <cstring>

#define MAX_EVENT_MONITOR 2048
#define NAME_LEN 32
#define MONITOR_EVENT_SIZE (sizeof(struct inotify_event))
#define BUFFER_LEN MAX_EVENT_MONITOR*(MONITOR_EVENT_SIZE+NAME_LEN)

using namespace cv;
using namespace std;

int main(){
	int fd, watch_desc;
	char buffer[BUFFER_LEN];
	fd = inotify_init();
	
	string path = "/data/cat/lion/";

	if (fd < 0)
		printf("Notify did not initialize");

	watch_desc = inotify_add_watch(fd, path.c_str(), IN_CREATE);

	if (watch_desc == -1)
		printf("Couldn't add watch to the path");
	else
		printf("Monitoring path...\n");

	while(1){
		int total_read = read(fd, buffer, BUFFER_LEN);
		if(total_read < 0)
			printf("Read error");

		int i = 0;
		char *name = "";
		path = "/data/cat/lion/";
		while(i < total_read){
			struct inotify_event *event = (struct inotify_event*) &buffer[i];
			if(event->len){
				if(event->mask & IN_CREATE){
					if(event->mask & IN_ISDIR)
						printf("Directory \"%s\" was created\n", event->name);
					else {
						printf("File \"%s\" was created\n", event->name);
						name = event->name;
					}
				}
				i += MONITOR_EVENT_SIZE + event->len;
			}
		}
		if (name[0] == '.') {
			continue;
		}
		path += name;
		cout << "Reached here, this is the name of the file: " << path << endl;
		//usleep(1000);
		Mat img = imread(path, 0);
		imshow("testing", img);
		waitKey(1);
	}

	inotify_rm_watch(fd, watch_desc);
	close(fd);
}
