#include <stdio.h>
#include <unistd.h>
#include <string>

//camera-functions
#include "camera.h"

//graphics/egl library
//#include "graphics.h"

//opencv
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

//image size
//#define IMAGE_HEIGHT 512
//#define IMAGE_WIDTH 768

#define IMAGE_HEIGHT 512
#define IMAGE_WIDTH 512


inline uchar clip(int n) {
    n &= -(n >= 0);
    return n | ((255 - n) >> 31);
}

//YUV420 --> BGR888
void yuv2bgr(uchar* &yuv, uchar* &bgr, int w, int h) {
    //NOTE: not error checking/limits done! Make sure the given arrays are of appropiate size.
    // > yuv: 1.50 * w * h byte
    // > rgb: 3.00 * w * h byte
    
    int C, D, E;
    
    //start idx for U & V
    int ys = 0;
    int us = w*h;
    int vs = w*h + ((w*h)>>2);
    
    // row/column derived from i in U/V domain
    int r, c;
    
    for (int i=0;i<w*h;i++) {
        //get current position
        r = (((int)floor(i/w)) >> 1) * (w>>1);
        c = (i%w) >> 1;
        
        //read values
        C = yuv[ys + i] - 16;
        D = yuv[us + r + c] - 128;
        E = yuv[vs + r + c] - 128;
        
        //compute 
        bgr[i*3 + 0] = clip(( 298 * C + 516 * D           + 128) >> 8); //B
        bgr[i*3 + 1] = clip(( 298 * C - 100 * D - 208 * E + 128) >> 8); //G
        bgr[i*3 + 2] = clip(( 298 * C           + 409 * E + 128) >> 8); //R
    }
}



void argb2bgr(uchar* &argb, uchar* &bgr, int w, int h) {
    //NOTE: not error checking/limits done! Make sure the given arrays are of appropiate size.
    // > argb: 4.00 * w * h byte
    // >  rgb: 3.00 * w * h byte
        
    for (int i=0;i<w*h;i++) {
        bgr[i*3 + 0] = argb[i*4 + 2]; //B
        bgr[i*3 + 1] = argb[i*4 + 1]; //G
        bgr[i*3 + 2] = argb[i*4 + 0]; //R
    }
}



//entry point
int main(int argc, const char **argv)
{
    //frame buffer
    const void* buf; int buf_size;

	//should the camera convert frame data from yuv to argb automatically?
	bool do_argb_conversion = true;
    
	//how many detail levels (1 = just the capture res, > 1 goes down by half each level, 4 max)
    // NOTE: OpenCV rendering below cannot handle this!
	int num_levels = 1;
    //frames per second
    int frame_rate = 30;
    int frames     = 100;
    
	//init camera
	CCamera* cam = StartCamera(IMAGE_WIDTH, IMAGE_HEIGHT, frame_rate, num_levels, do_argb_conversion);
   
    //allocate images
    cv::Mat img_yuv( IMAGE_HEIGHT*1.5, IMAGE_WIDTH, CV_8UC1);
    cv::Mat img_bgr( IMAGE_HEIGHT    , IMAGE_WIDTH, CV_8UC3);
    cv::Mat img_argb(IMAGE_HEIGHT    , IMAGE_WIDTH, CV_8UC4);
    
    printf("Creating OpenCV windows\n");
    
    //OpenCV: window to show images
    // --> create for each level
    std::string ocvwindow = std::string("ocvwindow");
    for (int j=0; j<num_levels; j++)
        cv::namedWindow( ocvwindow + std::to_string(j), cv::WINDOW_AUTOSIZE );

    printf("Running frame loop\n");
    for (int i=0; i<frames; i++) {
 
        //read frames of all levels
        for (int j=0; j<num_levels; j++) {

            //load frame into tmp
            if(cam->BeginReadFrame(j, buf, buf_size)) {
        
                //copy data to Mat-object
                printf("%d - read: %d bytes\n", i, buf_size);
                
                if (do_argb_conversion) {
                    // argb --> bgr                    
                    img_argb.data = (uchar*) buf;
                    argb2bgr( img_argb.data, img_bgr.data, IMAGE_WIDTH, IMAGE_HEIGHT);
                    
                } else {
                    // yuv420 --> bgr                    
                    img_yuv.data = (uchar*) buf;
                    yuv2bgr( img_yuv.data, img_bgr.data, IMAGE_WIDTH, IMAGE_HEIGHT);
                }
                
                //show image
                cv::imshow( ocvwindow + std::to_string(j), img_bgr);

            } else {
                printf("%d - Cannot read %d\n", i, j);
            }
        
            cam->EndReadFrame(j);
        }
        
        //wait for key
        cv::waitKey(10 + (1000/frame_rate));
        //cv::waitKey(0);
    }

    //gracefully stop camera & mmal
    StopCamera();
}
