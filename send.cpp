#include <opencv2/opencv.hpp>
#include <iostream>
#include <sys/time.h>

using namespace std;


string addr = "rtsp://ID:PASSWORD@127.0.0.1/Straming/channels/1";

void show_ipcam(){
    cv::Mat img;
    cv::VideoCapture cap(addr);
    if(!cap.isOpened()){
        cout << "cannot open usb camera\n";
        return;
    }
    double prev = static_cast<double>(cv::getTickCount());
    double now;
    long long init = 0;
    int fps_tick = 0;
    int cnt = 0;
    struct timeval te;
    while(1){
        cap.read(img);
        if(img.empty()){
            cout << "empty video is captured\n";
            break;
        }

        gettimeofday(&te, NULL);
        if(!init) {
            init = te.tv_sec;
        }
        else {
            if(init == te.tv_sec) {
                cnt++;
            }
            else {
                fps_tick = cnt;
                cnt = 0;
                init = te.tv_sec;
            }
        }

        now = static_cast<double>(cv::getTickCount());
        string text = "Frame size: " + to_string((int)cap.get(cv::CAP_PROP_FRAME_WIDTH)) 
            + "x" + to_string((int)cap.get(cv::CAP_PROP_FRAME_HEIGHT)) +
            "  FPS: " + to_string(fps_tick);
        cv::putText(img, text, cv::Point(10,40), 1, 2, cv::Scalar(255,0,255), 2.5);
        cv::imshow("image", img);

        prev = now;
        int keycode = cv::waitKey(1) & 0xff ;
        if (keycode == 'q') break ;
    }

    cap.release();
    cv::destroyAllWindows();
    return;
}

void show_usbcam(){
    cv::Mat img;
    cv::VideoCapture cap(
        "nvarguscamerasrc ! video/x-raw(memory:NVMM), \
        width=1280, height=720, format=NV12, framerate=30/1 ! \
        nvvidconv flip-method=0 ! video/x-raw, width=1280, \
        height=720, format=BGRx ! videoconvert ! video/x-raw, \
        format=BGR ! appsink", cv::CAP_GSTREAMER);
    if(!cap.isOpened()){
        cout << "cannot open IP camera\n";
        return;
    }
    double prev = static_cast<double>(cv::getTickCount());
    double now;
    long long init = 0;
    int fps_tick = 0;
    int cnt = 0;
    struct timeval te;
    while(1){
        cap.read(img);
        if(img.empty()){
            cout << "empty video is captured\n";
            break;
        }

        gettimeofday(&te, NULL);
        if(!init) {
            init = te.tv_sec;
        }
        else {
            if(init == te.tv_sec) {
                cnt++;
            }
            else {
                fps_tick = cnt;
                cnt = 0;
                init = te.tv_sec;
            }
        }

        now = static_cast<double>(cv::getTickCount());
        string text = "Frame size: " + to_string((int)cap.get(cv::CAP_PROP_FRAME_WIDTH)) 
            + "x" + to_string((int)cap.get(cv::CAP_PROP_FRAME_HEIGHT)) +
            "  FPS: " + to_string(fps_tick);
        cv::putText(img, text, cv::Point(10,40), 1, 2, cv::Scalar(255,0,255), 2.5);
        cv::imshow("image", img);

        prev = now;
        int keycode = cv::waitKey(1) & 0xff ;
        if (keycode == 'q') break ;
    }

    cap.release();
    cv::destroyAllWindows();
    return;
}

void send_ipcam_slow(){
    cv::Mat img;
    cv::VideoCapture cap(addr);
    cv::Size size = cv::Size((int)cap.get(cv::CAP_PROP_FRAME_WIDTH), (int)cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    int fps = cap.get(cv::CAP_PROP_FPS);
    cv::VideoWriter out;
    out.open(
        "appsrc ! videoconvert ! x264enc tune=zerolatency \
        bitrate=5000 speed-preset=superfast ! rtph264pay ! \
        udpsink host=192.168.100.255 port=5000", 
        cv::CAP_GSTREAMER, 0, fps, size, true);

    if(!cap.isOpened() || !out.isOpened()){
        cout << "cannot open IP camera\n";
        return;
    }
    double prev = static_cast<double>(cv::getTickCount());
    double now;
    long long init = 0;
    int fps_tick = 0;
    int cnt = 0;
    struct timeval te;
    while(1){
        cap.read(img);
        if(img.empty()){
            cout << "empty video is captured\n";
            break;
        }

        gettimeofday(&te, NULL);
        if(!init) {
            init = te.tv_sec;
        }
        else {
            if(init == te.tv_sec) {
                cnt++;
            }
            else {
                fps_tick = cnt;
                cnt = 0;
                init = te.tv_sec;
            }
        }

        now = static_cast<double>(cv::getTickCount());
        string text = "Frame size: " + to_string((int)cap.get(cv::CAP_PROP_FRAME_WIDTH)) 
            + "x" + to_string((int)cap.get(cv::CAP_PROP_FRAME_HEIGHT)) +
            "  FPS: " + to_string(fps_tick);
        cv::putText(img, text, cv::Point(10,40), 1, 2, cv::Scalar(255,0,255), 2.5);
        out.write(img);
        cv::imshow("image", img);

        prev = now;
        int keycode = cv::waitKey(1) & 0xff ;
        if (keycode == 'q') break ;
    }

    cv::destroyAllWindows();
    cap.release();
    out.release();

    cout << "\nVideo sending is done\n";
    return;
}

void send_ipcam_fast(){
    cv::Mat img;
    cv::VideoCapture cap(
        "rtspsrc location=addr \
        latency=200 ! application/x-rtp, media=video, encoding-name=H264 ! \
        rtph264depay ! h264parse ! omxh264dec ! video/x-raw(memory:NVMM) ! \
        nvvidconv ! video/x-raw, format=BGRx ! videoconvert ! \
        video/x-raw, format=BGR ! appsink", cv::CAP_GSTREAMER);
    cv::Size size = cv::Size((int)cap.get(cv::CAP_PROP_FRAME_WIDTH), (int)cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    int fps = cap.get(cv::CAP_PROP_FPS);
    cv::VideoWriter out;
    out.open(
        "appsrc ! videoconvert ! video/x-raw, format=BGRx ! \
        nvvidconv ! omxh264enc insert-sps-pps=true ! \
        h264parse ! rtph264pay pt=96 ! queue ! application/x-rtp, media=video, \
        encoding-name=H264 ! udpsink host=192.168.100.255 port=5000 sync=false", 
        cv::CAP_GSTREAMER, 0, fps, size, true);
    if(!cap.isOpened() || !out.isOpened()){
        cout << "cannot open IP camera\n";
        return;
    }
    double prev = static_cast<double>(cv::getTickCount());
    double now;
    long long init = 0;
    int fps_tick = 0;
    int cnt = 0;
    struct timeval te;
    while(1){
        cap.read(img);
        if(img.empty()){
            cout << "empty video is captured\n";
            break;
        }

        gettimeofday(&te, NULL);
        if(!init) {
            init = te.tv_sec;
        }
        else {
            if(init == te.tv_sec) {
                cnt++;
            }
            else {
                fps_tick = cnt;
                cnt = 0;
                init = te.tv_sec;
            }
        }

        now = static_cast<double>(cv::getTickCount());
        string text = "Frame size: " + to_string((int)cap.get(cv::CAP_PROP_FRAME_WIDTH)) 
            + "x" + to_string((int)cap.get(cv::CAP_PROP_FRAME_HEIGHT)) +
            "  FPS: " + to_string(fps_tick);
        cv::putText(img, text, cv::Point(10,40), 1, 2, cv::Scalar(255,0,255), 2.5);
        out.write(img);
        cv::imshow("image", img);

        prev = now;
        int keycode = cv::waitKey(1) & 0xff ;
        if (keycode == 'q') break ;
    }

    cv::destroyAllWindows();
    cap.release();
    out.release();

    cout << "\nVideo sending is done\n";
    return;
}

void send_usbcam(){
    cv::Mat img;
    cv::VideoCapture cap(
        "nvarguscamerasrc ! video/x-raw(memory:NVMM), \
        width=1280, height=720, format=NV12, framerate=30/1 ! \
        nvvidconv ! video/x-raw, format=BGRx ! videoconvert ! video/x-raw, \
        format=BGR ! appsink", cv::CAP_GSTREAMER);
    cv::VideoWriter out;
    cv::Size size = cv::Size((int)cap.get(cv::CAP_PROP_FRAME_WIDTH), (int)cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    int fps = cap.get(cv::CAP_PROP_FPS);
    out.open(
        "appsrc ! videoconvert ! x264enc tune=zerolatency \
        bitrate=5000 speed-preset=superfast ! rtph264pay ! \
        udpsink host=192.168.100.255 port=5000", 
        cv::CAP_GSTREAMER, 0, fps, size, true);
        
    if(!cap.isOpened() || !out.isOpened()){
        cout << "cannot open IP camera\n";
        return;
    }
    double prev = static_cast<double>(cv::getTickCount());
    double now;
    long long init = 0;
    int fps_tick = 0;
    int cnt = 0;
    struct timeval te;
    while(1){
        cap.read(img);
        if(img.empty()){
            cout << "empty video is captured\n";
            break;
        }

        gettimeofday(&te, NULL);
        if(!init) {
            init = te.tv_sec;
        }
        else {
            if(init == te.tv_sec) {
                cnt++;
            }
            else {
                fps_tick = cnt;
                cnt = 0;
                init = te.tv_sec;
            }
        }

        now = static_cast<double>(cv::getTickCount());
        string text = "Frame size: " + to_string((int)cap.get(cv::CAP_PROP_FRAME_WIDTH)) 
            + "x" + to_string((int)cap.get(cv::CAP_PROP_FRAME_HEIGHT)) +
            "  FPS: " + to_string(fps_tick);
        cv::putText(img, text, cv::Point(10,40), 1, 2, cv::Scalar(255,0,255), 2.5);
        out.write(img);
        cv::imshow("image", img);

        prev = now;
        int keycode = cv::waitKey(1) & 0xff ;
        if (keycode == 'q') break ;
    }

    cv::destroyAllWindows();
    cap.release();
    out.release();
    
    cout << "\nVideo sending is done\n";
    return;
}

int main(){
    show_ipcam();
    show_usbcam();
    send_ipcam_slow();
    send_ipcam_fast();
    send_usbcam();

    return 0;
}
