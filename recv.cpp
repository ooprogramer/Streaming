#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;

void recv(){
    cv::Mat img;
    
    cv::VideoCapture cap(
        "udpsrc port=5000 ! application/x-rtp, media=video, \
        clock-rate=90000, encoding-name=H264, payload=96 ! \
        rtph264depay ! decodebin ! videoconvert ! appsink", cv::CAP_GSTREAMER);
    if(!cap.isOpened()){
        cout << "cannot open IP camera\n";
        return;
    }
    while(1){
        cap.read(img);
        if(img.empty()){
            cout << "empty video is captured\n";
            break;
        }
        cv::imshow("image", img);

        int keycode = cv::waitKey(1) & 0xff ;
        if (keycode == 'q') break;
    }

    cap.release();
    cv::destroyAllWindows();
    return;
}

int main(){
    recv();

    return 0;
}
        