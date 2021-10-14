#include "opencv2/opencv.hpp"
#include <iostream>
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h> 
#include <string.h>

using namespace cv;

void *display(void *);


int main(int argc, char** argv)
{   

    //--------------------------------------------------------
    //networking stuff: socket, bind, listen
    //--------------------------------------------------------
    int                 localSocket,
                        remoteSocket,
                        port = 4097;                               

    struct  sockaddr_in localAddr,
                        remoteAddr;
    pthread_t thread_id;
    
           
    int addrLen = sizeof(struct sockaddr_in);

       
    if ( (argc > 1) && (strcmp(argv[1],"-h") == 0) ) {
          std::cerr << "usage: ./cv_video_srv [port] [capture device]\n" <<
                       "port           : socket port (4097 default)\n" <<
                       "capture device : (0 default)\n" << std::endl;

          exit(1);
    }

    if (argc == 2) port = atoi(argv[1]);

    localSocket = socket(AF_INET , SOCK_STREAM , 0);
    if (localSocket == -1){
         perror("socket() call failed!!");
    }    

    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = INADDR_ANY;
    localAddr.sin_port = htons( port );

    if( bind(localSocket,(struct sockaddr *)&localAddr , sizeof(localAddr)) < 0) {
         perror("Can't bind() socket");
         exit(1);
    }
    
    // Listening
    listen(localSocket , 3);
    
    std::cout <<  "Waiting for connections...\n"
              <<  "Server Port:" << port << std::endl;

    // accept connection from an incoming client
    while(1){
    //if (remoteSocket < 0) {
    //    perror("accept failed!");
    //    exit(1);
    //}
       
     remoteSocket = accept(localSocket, (struct sockaddr *)&remoteAddr, (socklen_t*)&addrLen);  
      //std::cout << remoteSocket<< "32"<< std::endl;
    if (remoteSocket < 0) {
        perror("accept failed!");
        exit(1);
    } 
    std::cout << "Connection accepted" << std::endl;
     pthread_create(&thread_id,NULL,display,&remoteSocket);

     // pthread_join(thread_id,NULL);

    }
    // pthread_join(thread_id,NULL);
    // close(remoteSocket);

    return 0;
}

void *display(void *ptr){
    int socket = *(int *)ptr;
    //OpenCV Code
    //----------------------------------------------------------

    Mat img, imgGray, flipped;
    img = Mat::zeros(480 , 640, CV_8UC3);   
     // make it continuous
    if (!img.isContinuous()) {
        img = img.clone();
    }
    uchar *iptr = img.data;

    int imgSize = img.total() * img.elemSize();
    int bytes = 0;
    int key;
    

    // make img continuos
    //if ( ! img.isContinuous() ) { 
    //      img = img.clone();
    //      imgGray = img.clone();
    //}
        
    std::cout << "Image Size:" << imgSize << std::endl;

    namedWindow("CV Video Client", 1);

    while(1) {
                
                // send processed image
                if ((bytes = recv(socket, iptr, imgSize, MSG_WAITALL)) == -1){
                     std::cerr << "recv failed, received bytes = " << bytes << std::endl;
                     break;
                }

		cv::imshow("CV Video Client", img);
		if ((key = cv::waitKey(10)) >= 0) break;
    }

    close(socket);

    return 0;
}
