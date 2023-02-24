#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>

void info(void) {
    std::cout << "EML 4840 Robot Design" << std::endl
              << "Florida International University" << std::endl
              << "Department of Mechanical and Materials Engineering" << std::endl
              << "Anthony Abrahao <aabrahao@fiu.edu>" << std::endl
              << "Miami, Spring 2022" << std::endl
              << std::endl;
}

#define KEY_ESC 27

using Pixel = cv::Vec<uchar, 3>; // G,B,R

uchar gray(Pixel p) {
    return 0.3*p[2] + 0.59*p[1] + 0.11*p[0];
}

void record_p(std::vector<uchar>data,std::vector<int>&positions)
{
    uchar threshold = 220;
    int position =-1;
    int count=0;
    int min_count= 3;
    bool crossed_threshold=false;
    
    for(int i=0;i<data.size();i++)
    {
        if(data[i]>=threshold)
        {
        count++;
            if(count>=min_count && position==-1){
                position=i-min_count+1;
                positions.push_back(position);
                //std::cout << "threshold crossed at position "<<position<<std::endl;
                crossed_threshold=true;
            }
        }else {
                if(crossed_threshold)
                {
                    if(data[i]<150){
                    position = i;
                    positions.push_back(position);
                    //std::cout <<"threshold dropped at position "<<position<<std::endl;
                    crossed_threshold=false;
                    }
                }
            
        
            count=0;
        

        }
    } 
     if(position==-1)
        positions.push_back(0);
        positions.push_back(0);
}

void plotRow(cv::Mat &image, std::vector<uchar> row_colors, int y, float scale, cv::Scalar color) {
    std::vector<cv::Point> points;
    for (int x = 0; x < row_colors.size(); ++x)
        points.push_back( cv::Point(x, y - scale*row_colors[x]) );
    cv::polylines(image, points, false, color, 2);
}

int main(int argc, char **argv) {
    info();
    // Create a capture object from device number (or video filename)
    cv::VideoCapture cap("/home/pedro/projects/LineFollower/line.mp4");
    // Check if any error happened
    if(!cap.isOpened()) {
        std::cout << "Ops, capture cannot be created!" << std::endl;
        return -1;
    }
    std::cout << "Press:" << std::endl
              << "s            : to save image" << std::endl
              << "r, g, b, or k: to show colors" << std::endl
              << "q or ESC     : to quit" << std::endl;

        // Create an image object 
        cv::Mat frame;

        while(cap.read(frame)){
        
        cv::namedWindow("EML4840");
        cv::Mat hsv ;
        cv::cvtColor(frame,hsv,cv::COLOR_BGR2HSV);

        int track_scale=20;
        int track_resize = 50;
        

        //Menu
        bool show_red=true;
        bool show_blue=true;
        bool show_green=true;
        bool show_gray=true;
        bool show_hue=true;
        bool show_sat=true;
        bool show_val=true;


        std::vector<uchar> r,g,b,k;
        std::vector<uchar> h,s,v;
        std::vector<int>positions;

        bool running =true;

    
        r.clear();
        g.clear();
        b.clear();
        k.clear();

        h.clear();
        s.clear();
        v.clear();
        
        int y= 0.7*(frame.rows-1);
        float scale =0.01*track_scale;

        for (int x = 0; x < frame.cols; ++x ) {
            
            Pixel pixel = frame.at<Pixel>( cv::Point(x, y) );
            r.push_back( pixel[2] );
            g.push_back( pixel[1] );
            b.push_back( pixel[0] );
            k.push_back( gray(pixel) );
            
            Pixel hsvpixel=hsv.at<Pixel>(cv::Point(x,y));
            h.push_back( hsvpixel[0] );
            s.push_back( hsvpixel[1] );
            v.push_back( hsvpixel[2] );

            //std::cout << x <<":"<<(int)hsvpixel[1]<<std::endl;
        
        }
        // clone image and keep the orginal for processing!
        cv::Mat canvas = frame.clone();
        if (show_red)   plotRow(canvas, r, y, scale, cv::Scalar(0,0,255));
        if (show_green) plotRow(canvas, g, y, scale, cv::Scalar(0,255,0));
        if (show_blue)  plotRow(canvas, b, y, scale, cv::Scalar(255,0,0));
        if (show_gray)  plotRow(canvas, k, y, scale, cv::Scalar(0,0,0));
        if (show_hue) plotRow(canvas, h, y, scale, cv::Scalar(255,0,255));
        if (show_sat)  plotRow(canvas, s, y, scale, cv::Scalar(255,255,0));
        if (show_val)  plotRow(canvas, v, y, scale, cv::Scalar(0,255,255));
        cv::line(canvas, cv::Point(0, y), cv::Point(frame.cols, y), cv::Scalar(0,0,0), 2);
        
        
        record_p(v,positions);

        if(positions[0]==0)
        {
            positions[0]=1;
        }
         if(positions[1]==0)
         {
            positions[1]=1;
         }

         int mp= ((positions[0]+positions[1])/2);
        //Place marker
        cv::drawMarker(canvas,cv::Point(positions[0],y),cv::Scalar(0,165,255),cv::MARKER_DIAMOND,10,2);
        cv::drawMarker(canvas,cv::Point(positions[1],y),cv::Scalar(0,165,255),cv::MARKER_DIAMOND,10,2);
        cv::drawMarker(canvas,cv::Point(mp,y),cv::Scalar(0,165,255),cv::MARKER_CROSS,10,2);

        int center=(frame.cols)/2;

        cv::drawMarker(canvas,cv::Point(center,y),cv::Scalar(255,255,255),cv::MARKER_CROSS,20,3);

        //Error
        double u = abs(center-mp);
        double Er= (u/mp)*100;

        std::stringstream d;
       
        d<< std::fixed << std::setprecision(2) << Er;

        std::string n= d.str();
        

        int tx = 0.3*frame.cols;
        int ty = 0.9*frame.rows;
        cv::putText(canvas,cv::String("Error(%):"+n),cv::Point(tx,ty),cv::FONT_HERSHEY_DUPLEX,1,cv::Scalar(255,255,255),2,false);

        // Menu
        int key = cv::waitKey(10);
        switch(key) {
        
        case 'r':
            show_red = !show_red;
            break;
        case 'g':
            show_green = !show_green;
            break;
        case 'b':
            show_blue = !show_blue;
            break;
        case 'k':
            show_gray = !show_gray;
            break;
        case 'h':
            show_hue=!show_hue;
            break;
        case 's':
            show_sat=!show_sat;
            break;
        case 'v':
            show_val=!show_val;
            break;

        }
        cv::resize(canvas,canvas,cv::Size(),0.025*track_resize,0.02*track_resize);
        cv:: imshow("EML4840",canvas);
        if(cv::waitKey(30)=='q')
        {
            break;
        }
    }
cap.release();
cv::destroyAllWindows();
return 0;

}

