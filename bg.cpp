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
#include <vector>
#include <unordered_map>
#include <csignal>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <charconv>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/videoio.hpp>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/videodev2.h>

#define VID_OUT "/dev/video20"

sig_atomic_t sigint_ex = 0;

static int setup(std::unordered_map<std::string, int>& set, int& width, int& height) {
	const std::vector<std::string> setStr{
		"topLeftX=", "topLeftY=", "width=",
		"height=",	 "showRect=", "blurFactor=","frameWidth=","frameHeight="};
	std::ifstream in("options.txt");
	std::stringstream ss;
	ss << in.rdbuf();
	in.close();
	std::string s = ss.str();
	for(const auto& str : setStr) {
		int pos = s.find(str);
		int n;
		std::string tmp = s.substr(pos+str.length(),pos+str.length()-s.find('\n',pos));
		auto res = std::from_chars(tmp.data(),tmp.data()+tmp.size(),n);
		if(res.ec == std::errc()){
			set[str.substr(0,str.length()-1)] = n;
		}
	}

	if(set.count("frameWidth") && set["frameWidth"] != width){
		width = set["frameWidth"];
	}

	if(set.count("frameHeight") && set["frameHeight"] != height){
		height = set["frameHeight"];
	}

	int fd = open(VID_OUT, O_RDWR);
	if(fd < 0) {
		std::cout << "Could not open virtual camera device\n";
		exit(1);
	}
	struct v4l2_format format {};
	format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	if(ioctl(fd, VIDIOC_G_FMT, &format) < 0) {
		std::cout << "Could not configure virtual camera device\n";
		close(fd);
		exit(1);
	}
	format.fmt.pix.width = width;
	format.fmt.pix.height = height;
	format.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
	format.fmt.pix.sizeimage = 3 * width * height;
	format.fmt.pix.field = V4L2_FIELD_NONE;
	if(ioctl(fd, VIDIOC_S_FMT, &format) < 0) {
		std::cout << "Could not configure virtual camera device\n";
		close(fd);
		exit(1);
	}
	return fd;
}

void writeFrame(int fd, cv::Mat &frame, int width, int height) {
	if(write(fd, frame.data, 3 * width * height) < 0) {
		std::cout << "Could not write to virtual camera device\n";
	}
}

void sigHandler(int sig) {
	sigint_ex = 1;
}

int main() {
	signal(SIGINT, sigHandler);
	std::unordered_map<std::string,int> set;
	cv::VideoCapture cap(0);
	if(!cap.isOpened()) {
		std::cout << "Could not start webcam\n";
	}
	int width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
	int height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
	int fd = setup(set, width, height);
	cv::Mat frame;
	cv::CascadeClassifier cas;
	cas.load("lbpcascade_frontalface_improved.xml");
	cv::Mat lastFrame = cv::Mat::zeros(cv::Size(width,height),CV_8UC1);
	for(;;) {
		try {
			if(sigint_ex){
				close(fd);
				exit(1);
			}
			cap >> frame;
			if(frame.empty()) {
				continue;
			}
			cv::Mat frame_gray;
			cv::cvtColor(frame, frame_gray, cv::COLOR_BGR2GRAY);
			std::vector<cv::Rect> faces;
			cas.detectMultiScale(frame_gray, faces, 1.2, 2,
								 0 | cv::CASCADE_SCALE_IMAGE, cv::Size(30, 30));
			if(faces.empty()) {
				writeFrame(fd,lastFrame, width, height);
				continue;
			}
			std::vector<cv::Mat> sectors(faces.size());
			for(size_t i{};i<faces.size();i++){
				faces[i].x -= set["topLeftX"];
				faces[i].y -= set["topLeftY"];
				faces[i].width += set["width"];
				faces[i].height += set["height"];
				if(faces[i].x < 0 || faces[i].y < 0 || faces[i].width < 0 ||
				   faces[i].height < 0) {
					continue;
				}
				sectors[i] = cv::Mat(frame, faces[i]).clone();
			}
			cv::blur(frame, frame, cv::Size(set["blurFactor"], set["blurFactor"]));
			for(size_t i{};i<faces.size();i++){
				sectors[i].copyTo(frame(faces[i]));
				if(set["showRect"]) {
					cv::rectangle(frame, faces[i], cv::Scalar(0, 250, 0));
				}
			}
			cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
			writeFrame(fd,frame, width, height);
			lastFrame = frame.clone();
		} catch(cv::Exception &e) {
		}
	}
	close(fd);
}
