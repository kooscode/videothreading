#include <cstdlib>
#include <stdio.h>
#include <vector>
#include <iostream>

//OPENCV 
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

//Requires libterraclear
//add "../" to include path or wherever libterraclear was cloned to..
#include "libterraclear/src/camera_usb.hpp"
#include "libterraclear/src/stopwatch.hpp"

namespace tc = terraclear;

#include <thread>
#include <mutex>

std::mutex _mutex;
cv::Mat _cam_raw;
cv::Mat _frame;
cv::VideoCapture _videocapture;
bool _running = true;

void reader_thread()
{
    while (_running)
    {
        //read frame...
        _videocapture.read(_cam_raw); 

        _mutex.lock();
            _cam_raw.copyTo(_frame);     
        _mutex.unlock();
    }
}

int main(int argc, char** argv) 
{
    //testing regular usb cam implementation..
//    tc::camera_usb usbcam(0, 1280, 720);
//    tc::camera_base* cam = &usbcam;
//    cam->update_frames();
//    cv::Mat cam_raw = cam->getRGBFrame();
    
    //Open CV Window stuff
    std::string window_name = "rgb";
    cv::namedWindow(window_name, cv::WINDOW_NORMAL | cv::WINDOW_FREERATIO);// | WINDOW_AUTOSIZE);
    

    //timing..
    tc::stopwatch sw;
    sw.start();
    
    _videocapture.open(0);
    
    _videocapture.set(CV_CAP_PROP_FRAME_WIDTH,1920); // resolution X
    _videocapture.set(CV_CAP_PROP_FRAME_HEIGHT,1080); //resolution Y

//    _videocapture.set(CV_CAP_PROP_FRAME_WIDTH,1280); // resolution X
//    _videocapture.set(CV_CAP_PROP_FRAME_HEIGHT,720); //resolution Y

//    _videocapture.set(CV_CAP_PROP_FRAME_WIDTH,1280); // resolution X
//    _videocapture.set(CV_CAP_PROP_FRAME_HEIGHT,960); //resolution Y


    //make sure video stream is open/active
    if (!_videocapture.isOpened())
    { //check if video device has been initialised
        std::cout << "Error loading video" << std::endl;
        return -1;
    }  

    //read first frame..
    _videocapture.read(_cam_raw); 
    
    //copy it..
    _cam_raw.copyTo(_frame);     

    //start capture thread..
    std::thread cvthread(&reader_thread);
    
    do
    {
         //adjust framerate
        if (sw.get_elapsed_ms() > 30)
        {
            _mutex.lock();
                std::stringstream fpsstr;
                double fps = 1000 /  sw.get_elapsed_ms();
                fpsstr << std::fixed << std::setprecision(0) << fps << "fps";
                cv::putText(_frame, fpsstr.str(), cv::Point(10,50), cv::FONT_HERSHEY_PLAIN, 4,  cv::Scalar(0x00, 0x00, 0xff), 4);   
                sw.reset();
                //show window
                cv::imshow(window_name, _frame);
            _mutex.unlock();
        }
        
        
        int x = cv::waitKey(1);
        if(x == 27) // ESC Key = exit
        {
            break;       
        }
        else if (x == 115)
        {
            //s for SAVE

            //Save frame..
            std::string filename = "frame-grab.jpg";
            cv::imwrite(filename, _frame);
        }        
        else if (x > 0)
        {
            std::cout << x << std::endl ;
        }
     
        //update image frame
       // cam->update_frames();
        
   } while (true);    
    
   //stop thread..
   _running = false; 
   cvthread.join();

    return 0;
}

