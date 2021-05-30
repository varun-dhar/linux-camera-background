/*
MIT License
Copyright (c) 2020 Varun Dhar
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <array>
#include <csignal>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#define NUM_OPS 6
#define VID_OUT "/dev/video20"

static int out;

static void setup(std::array<int,NUM_OPS>& set, int width, int height)
{
	std::array<std::string,NUM_OPS> search {"topLeftX=","topLeftY=","width=","height=","showRect=","blurFactor="};
	std::ifstream in("options.txt");
	std::stringstream ss;
	ss << in.rdbuf();
	in.close();
	std::string s = ss.str();
	for(int i = 0;i<NUM_OPS;i++)
	{
		int pos = s.find(search[i]);
//		std::cout << s.substr(pos+search[i].length(),pos+search[i].length()-s.find('\n',pos)) << '\n';
		set[i] = std::stoi(s.substr(pos+search[i].length(),pos+search[i].length()-s.find('\n',pos)));
	}
	out = open(VID_OUT,O_RDWR);
	if(out<0)
	{
		std::cout << "Could not open virtual camera device\n";
		exit(1);
	}
	struct v4l2_format format{};
	format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	if(ioctl(out,VIDIOC_G_FMT,&format) < 0){
		std::cout << "Could not configure virtual camera device\n";
		close(out);
		exit(1);
	}
	format.fmt.pix.width = width;
	format.fmt.pix.height = height;
	format.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
	format.fmt.pix.sizeimage = 3*width*height;
	format.fmt.pix.field = V4L2_FIELD_NONE;
	if(ioctl(out,VIDIOC_S_FMT,&format) < 0){
		std::cout << "Could not configure virtual camera device\n";
		close(out);
		exit(1);
	}
}


void writeFrame(cv::Mat& frame,int width,int height)
{
	if(write(out,frame.data,3*width*height) < 0)
	{
		std::cout << "Could not write to virtual camera device\n";
	}
}

void sigHandler(int sig)
{
	close(out);
	exit(1);
}

int main()
{
	signal(SIGINT,sigHandler);
	std::array<int,NUM_OPS> set{};
	cv::VideoCapture cap(0);
	if(!cap.isOpened())
	{
		std::cout << "Could not start webcam\n";
	}
	int width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
	int height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
	setup(set,width,height);
	cv::Mat frame;
	cv::CascadeClassifier cas;
	cas.load("lbpcascade_frontalface_improved.xml");
	cv::Mat lastFrame;
	for(;;){
		cap >> frame;
		if(frame.empty())
		{
			continue;
		}
		cv::Mat sec,frame_gray;
		cv::cvtColor(frame,frame_gray,cv::COLOR_BGR2GRAY);
		std::vector<cv::Rect> faces;
		cas.detectMultiScale(frame_gray,faces,1.2,2,0|cv::CASCADE_SCALE_IMAGE,cv::Size(30,30));
		if(faces.empty())
		{
			writeFrame(lastFrame,width,height);
			continue;
		}
		faces[0].x-=set[0];
		faces[0].y-=set[1];
		faces[0].width+=set[2];
		faces[0].height+=set[3];
		if(faces[0].x < 0 || faces[0].y < 0 || faces[0].width < 0 || faces[0].height < 0)
		{
			continue;
		}
//		std::cout << faces[0].x << '\n' << faces[0].y << '\n' << faces[0].width << '\n' << faces[0].height << '\n';
		sec = cv::Mat(frame,faces[0]).clone();
		cv::blur(frame,frame,cv::Size(set[5],set[5]));
		sec.copyTo(frame(faces[0]));
		if(set[4]){
			cv::rectangle(frame,faces[0],cv::Scalar(0,250,0));
		}
		cv::cvtColor(frame,frame,cv::COLOR_BGR2RGB);
		writeFrame(frame,width,height);
		lastFrame = frame.clone();
	}
	close(out);
}
