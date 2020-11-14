#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <bits/stdc++.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

using namespace cv;
using namespace std;

#define NUM_OPS 6
#define VID_OUT "/dev/video20"

static int out;

static void setup(int set[])
{
	string search[NUM_OPS]={"topLeftX=","topLeftY=","width=","height=","showRect=","blurFactor="};
	ifstream in("options.txt");
	stringstream ss;
	ss << in.rdbuf();
	in.close();
	string s = ss.str();
	for(int i = 0;i<NUM_OPS;i++)
	{
		int pos = s.find(search[i]);
//		cout << s.substr(pos+search[i].length(),pos+search[i].length()-s.find('\n',pos)) << endl;
		set[i] = stoi(s.substr(pos+search[i].length(),pos+search[i].length()-s.find('\n',pos)));
	}
	out = open(VID_OUT,O_RDWR);
	if(out<0)
	{
		cout << "Could not open virtual camera device" << endl;
		exit(1);
	}
	struct v4l2_format format;
	memset(&format,0,sizeof(format));
	format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	if(ioctl(out,VIDIOC_G_FMT,&format) < 0){
		cout << "Could not configure virtual camera device" << endl;
		close(out);
		exit(1);
	}
	format.fmt.pix.width = 1280;
	format.fmt.pix.height = 720;
	format.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
	format.fmt.pix.sizeimage = 3*1280*720;
	format.fmt.pix.field = V4L2_FIELD_NONE;
	if(ioctl(out,VIDIOC_S_FMT,&format) < 0){
		cout << "Could not configure virtual camera device" << endl;
		close(out);
		exit(1);
	}
}


void writeFrame(Mat frame)
{
	cvtColor(frame,frame,COLOR_BGR2RGB);
	int written = write(out,frame.data,3*1280*720);
	if(written < 0)
	{
		cout << "Could not write to virtual camera device" << endl;
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
	int set[NUM_OPS] = {0};
	setup(set);
	Mat frame;
	VideoCapture cap(0);
	if(!cap.isOpened())
	{
		cout << "Could not start webcam" << endl;
	}
	CascadeClassifier cas;
	cas.load("lbpcascade_frontalface_improved.xml");
	for(;;){
		cap >> frame;
		if(frame.empty())
		{
			continue;
		}
		vector<Rect> faces;
		cas.detectMultiScale(frame,faces,1.2,2,0|CASCADE_SCALE_IMAGE,Size(30,30));
		faces[0].x-=set[0];
		faces[0].y-=set[1];
		faces[0].width+=set[2];
		faces[0].height+=set[3];
		if(faces[0].x < 0 || faces[0].y < 0 || faces[0].width < 0 || faces[0].height < 0)
		{
			continue;
		}
//		cout << faces[0].x << endl << faces[0].y << endl << faces[0].width << endl << faces[0].height << endl;
		Mat sec = Mat(frame,faces[0]).clone();
		blur(frame,frame,Size(set[5],set[5]));
		sec.copyTo(frame(faces[0]));
		if(set[4]){
			rectangle(frame,faces[0],Scalar(0,250,0));
		}
//		imshow("win",frame);
		writeFrame(frame);
/*		if(waitKey(1)=='q')
		{
			break;
		}*/
	}
	close(out);
}
