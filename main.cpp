#include <cstdlib>
#include <stdio.h>
#include <vector>
#include <iostream>

//OPENCV 
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

//Requires libterraclear
//add "../" to include path or wherever libterraclear was cloned to..
#ifndef TC_USE_REALSENSE
    #define TC_USE_REALSENSE
#endif

#include "libterraclear/src/camera_usb.hpp"
#include "libterraclear/src/camera_file.hpp"
#include "libterraclear/src/camera_depth_realsense.hpp"
#include "libterraclear/src/stopwatch.hpp"

namespace tc = terraclear;

#include <thread>
#include <mutex>

tc::stopwatch _frame_sw;
std::mutex _mutex;
cv::Mat _frame;
uint32_t _frame_ms = 0;
tc::camera_base* _vidstream = nullptr;

bool _running = true;

void reader_thread()
{
    if (_vidstream == nullptr)
    {
        std::cout << "ERROR - CAMERA IS NULL" << std::endl;
        _running = false;
    }
    
    while (_running)
    {
        //read frame...
       _vidstream->update_frames(); 

        _mutex.lock();
            _vidstream->getRGBFrame().copyTo(_frame);
            _frame_ms = _frame_sw.get_elapsed_ms();
            _frame_sw.reset();
        _mutex.unlock();
    }
}

int main(int argc, char** argv) 
{
    
    //std::cout << cv::getBuildInformation() << std::endl;
    
    //testing regular usb cam implementation..
//    tc::camera_usb usbcam(0, 1280, 720);
//    tc::camera_base* cam = &usbcam;
//    cam->update_frames();
//    cv::Mat cam_raw = cam->getRGBFrame();
    
    //Open CV Window stuff
    std::string window_name = "rgb";
    cv::namedWindow(window_name, cv::WINDOW_NORMAL | cv::WINDOW_FREERATIO);// | WINDOW_AUTOSIZE);

   // _vidstream = new tc::camera_usb(0, 1920, 1080);
    if (argc > 1)
        _vidstream = new tc::camera_depth_realsense(argv[1]);
    else
        _vidstream = new tc::camera_depth_realsense();
        
  
//    _vidstream = new tc::camera_file("v4l2src device=/dev/video0 ! nvvidconv ! video/x-raw, width=1920, height=1080, format=BGR ! appsink");

    //timing..
    _frame_sw.start();
    
//    _videocapture.set(CV_CAP_PROP_FRAME_WIDTH,1920); // resolution X
//    _videocapture.set(CV_CAP_PROP_FRAME_HEIGHT,1080); //resolution Y

//    _videocapture.set(CV_CAP_PROP_FRAME_WIDTH,1280); // resolution X
//    _videocapture.set(CV_CAP_PROP_FRAME_HEIGHT,720); //resolution Y

//    _videocapture.set(CV_CAP_PROP_FRAME_WIDTH,1280); // resolution X
//    _videocapture.set(CV_CAP_PROP_FRAME_HEIGHT,960); //resolution Y

    //read first frame..
    _vidstream->update_frames(); 
    
    //copy it..
    _vidstream->getRGBFrame().copyTo(_frame);     

    //start capture thread..
    std::thread cvthread(&reader_thread);
    
    tc::stopwatch sw;
    sw.start();
    
    do
    {
         //adjust framerate
        if ((sw.get_elapsed_ms() > 30) && (_frame_ms > 0))
        {
            _mutex.lock();
                std::stringstream fpsstr;
                double cam_fps = 1000 /  _frame_ms;
                double refresh_fps = 1000 /  sw.get_elapsed_ms();
                fpsstr << "C:" << std::fixed << std::setprecision(0) << cam_fps << "";
                fpsstr << "R:" << std::fixed << std::setprecision(0) << refresh_fps;
                cv::putText(_frame, fpsstr.str(), cv::Point(10,50), cv::FONT_HERSHEY_PLAIN, 4,  cv::Scalar(0x00, 0x00, 0xff), 4);   
                
                std::cout << fpsstr.str() << std::endl;
                
                //show window
                cv::imshow(window_name, _frame);
                sw.reset();
            _mutex.unlock();
        }
        
        
        int x = cv::waitKey(1);
        if(x == 27) // ESC Key = exit
        {
            _running = false; 
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
        
   } while (_running);    
    
   //stop thread..
   cvthread.join();

    return 0;
}

