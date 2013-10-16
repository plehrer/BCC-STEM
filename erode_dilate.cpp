//objectTrackingTutorial.cpp

//Written by  Kyle Hounslow 2013

//Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software")
//, to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
//and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

//The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
//IN THE SOFTWARE.

#include <sstream>
#include <string>
#include <iostream>
#include <opencv/highgui.h>
#include <opencv/cv.h>



using namespace cv;
using namespace std;


// Assign initial values to the min and max HSV threshold values to 
// track red
int H_MIN = 159;
int H_MAX = 256;
int S_MIN = 127;
int S_MAX = 256;
int V_MIN = 176;
int V_MAX = 256;


//default capture width and height
const int FRAME_WIDTH = 1280;
const int FRAME_HEIGHT = 720;
//max number of objects to be detected in frame
const int MAX_NUM_OBJECTS=10;
//minimum and maximum object area
const int MIN_OBJECT_AREA = 20*20;
const int MAX_OBJECT_AREA = FRAME_HEIGHT*FRAME_WIDTH/1.5;

const string trackbarWindowName = "HSV Threshold Values";


char key = 0;


void on_trackbar( int, void* )
{//This function gets called whenever a
	// trackbar position is changed





}
string intToString(int number){


	std::stringstream ss;
	ss << number;
	return ss.str();
}

void drawObject(int x, int y,Mat &frame){

	//use some of the openCV drawing functions to draw crosshairs
	//on your tracked image!

    //UPDATE:JUNE 18TH, 2013
    //added 'if' and 'else' statements to prevent
    //memory errors from writing off the screen (ie. (-25,-25) is not within the window!)

	circle(frame,Point(x,y),20,Scalar(0,255,0),2);
    if(y-25>0)
    line(frame,Point(x,y),Point(x,y-25),Scalar(0,255,0),2);
    else line(frame,Point(x,y),Point(x,0),Scalar(0,255,0),2);
    if(y+25<FRAME_HEIGHT)
    line(frame,Point(x,y),Point(x,y+25),Scalar(0,255,0),2);
    else line(frame,Point(x,y),Point(x,FRAME_HEIGHT),Scalar(0,255,0),2);
    if(x-25>0)
    line(frame,Point(x,y),Point(x-25,y),Scalar(0,255,0),2);
    else line(frame,Point(x,y),Point(0,y),Scalar(0,255,0),2);
    if(x+25<FRAME_WIDTH)
    line(frame,Point(x,y),Point(x+25,y),Scalar(0,255,0),2);
    else line(frame,Point(x,y),Point(FRAME_WIDTH,y),Scalar(0,255,0),2);

	putText(frame,intToString(x)+","+intToString(y),Point(x,y+30),1,1,Scalar(0,255,0),2);
	putText(frame,"x = "+intToString(x)+","+"y = "+intToString(y),Point(5,715),1,2,Scalar(0,255,0),2);
}
void morphOps(Mat &thresh){

	//create structuring element that will be used to "dilate" and "erode" image.
	//the element chosen here is a 3px by 3px rectangle

	Mat erodeElement = getStructuringElement( MORPH_RECT,Size(3,3));
    //dilate with larger element so make sure object is nicely visible
	Mat dilateElement = getStructuringElement( MORPH_RECT,Size(8,8));

	erode(thresh,thresh,erodeElement);
	erode(thresh,thresh,erodeElement);


	dilate(thresh,thresh,dilateElement);
	dilate(thresh,thresh,dilateElement);

}
void trackFilteredObject(int &x, int &y, Mat threshold, Mat &cameraFeed){

	Mat temp;
	threshold.copyTo(temp);
	//these two vectors needed for output of findContours
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;
	//find contours of filtered image using openCV findContours function
	findContours(temp,contours,hierarchy,CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE );
	//use moments method to find our filtered object
	double refArea = 0;
	bool objectFound = false;
	if (hierarchy.size() > 0) {
		int numObjects = hierarchy.size();
        //if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
        if(numObjects<MAX_NUM_OBJECTS){
			for (int index = 0; index >= 0; index = hierarchy[index][0]) {

				Moments moment = moments((cv::Mat)contours[index]);
				double area = moment.m00;

				//if the area is less than 20 px by 20px then it is probably just noise
				//if the area is the same as the 3/2 of the image size, probably just a bad filter
				//we only want the object with the largest area so we safe a reference area each
				//iteration and compare it to the area in the next iteration.
                if(area>MIN_OBJECT_AREA && area<MAX_OBJECT_AREA && area>refArea){
					x = moment.m10/area;
					y = moment.m01/area;
					objectFound = true;

				}else objectFound = false;


			}
			//let user know you found an object
			if(objectFound ==true){
				putText(cameraFeed,"Tracking Object",Point(0,50),2,2,Scalar(0,255,0),4);
				//draw object location on screen
				drawObject(x,y,cameraFeed);}

		}else putText(cameraFeed,"TOO MUCH NOISE!",Point(0,50),2,2,Scalar(0,0,255),4);
	}
}





int main(int argc, char* argv[])
{

	//some boolean variables for different functionality within this
	//program
    bool trackObjects = true;
    bool useMorphOps = true;
	//Matrix to store each frame of the webcam feed
	Mat cameraFeed;
	Mat cameraFeed2;
	//matrix storage for HSV image
	Mat HSV;
	Mat HSV2;
	//matrix storage for binary threshold image
	Mat threshold;
	Mat threshold2;
	//x and y values for the location of the object
	int x=0, y=0;
	int x2=0, y2=0;
	

// open the default cameras
VideoCapture capture(0);
VideoCapture capture2(1);

// check for failure of first camera
if (!capture.isOpened()) {
printf("Failed to open video device 1 or video file!\n");
return 1;
}

// check for failure of second camera
if (!capture2.isOpened()) {
printf("Failed to open video device 2 or video file!\n");
return 1;
}

// Set Capture device properties.
capture.set(CV_CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
capture.set(CV_CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);

capture2.set(CV_CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
capture2.set(CV_CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);


//Create background window

	Mat org;
    org = imread("./BackgroundComputerVision_3.jpg");
    if (org.empty()) 
    {
        cout << "Cannot load image!" << endl;
        return -1;
    }
	namedWindow("Background", CV_WINDOW_AUTOSIZE);
    imshow("Background", org);
	resizeWindow("Background",1244, 768);
	moveWindow("Background",10, 0);

	
namedWindow("First Camera Tracking",0);
namedWindow("Second Camera Tracking",0);

namedWindow("First Camera HSV Smoothed and Thresholded Video",0);
namedWindow("Second Camera HSV Smoothed and Thresholded Video",0);
/*
moveWindow("First Camera Tracking", 20, 170);
moveWindow("Second Camera Tracking", 800, 170);

moveWindow("First Camera HSV Smoothed and Thresholded Video", 20, 450);
moveWindow("Second Camera HSV Smoothed and Thresholded Video", 800, 450);
*/

//create window for trackbars
    namedWindow(trackbarWindowName,0);
	//create memory to store trackbar name on window
	char TrackbarName[50];
	sprintf( TrackbarName, "H_MIN", H_MIN);
	sprintf( TrackbarName, "H_MAX", H_MAX);
	sprintf( TrackbarName, "S_MIN", S_MIN);
	sprintf( TrackbarName, "S_MAX", S_MAX);
	sprintf( TrackbarName, "V_MIN", V_MIN);
	sprintf( TrackbarName, "V_MAX", V_MAX);
	//create trackbars and insert them into window
	//3 parameters are: the address of the variable that is changing when the trackbar is moved(eg.H_LOW),
	//the max value the trackbar can move (eg. H_HIGH), 
	//and the function that is called whenever the trackbar is moved(eg. on_trackbar)
	//                                  ---->    ---->     ---->      
    createTrackbar( "H_MIN", trackbarWindowName, &H_MIN, 256, on_trackbar );
    createTrackbar( "H_MAX", trackbarWindowName, &H_MAX, 256, on_trackbar );
    createTrackbar( "S_MIN", trackbarWindowName, &S_MIN, 256, on_trackbar );
    createTrackbar( "S_MAX", trackbarWindowName, &S_MAX, 256, on_trackbar );
    createTrackbar( "V_MIN", trackbarWindowName, &V_MIN, 256, on_trackbar );
    createTrackbar( "V_MAX", trackbarWindowName, &V_MAX, 256, on_trackbar );

//moveWindow(trackbarWindowName, 465, 170);
//resizeWindow(trackbarWindowName, 320, 1);



	//start an infinite loop where webcam feed is copied to cameraFeed matrix
	//all of our operations will be performed within this loop
	while( key != 'q'){

// get a new frame from camera
capture >> cameraFeed;    //retrieve image from first camera
capture2 >> cameraFeed2;   //retrieve image from second camera

		//store image to matrix
		capture.read(cameraFeed);
		//convert frame from BGR to HSV colorspace

// Convert color space to HSV as it is much easier to filter colors in the HSV color-space
cvtColor(cameraFeed, HSV, CV_BGR2HSV);
cvtColor(cameraFeed2, HSV2, CV_BGR2HSV);

		//filter HSV image between values and store filtered image to
		//threshold matrix
		inRange(HSV,Scalar(H_MIN,S_MIN,V_MIN),Scalar(H_MAX,S_MAX,V_MAX),threshold);
		inRange(HSV2,Scalar(H_MIN,S_MIN,V_MIN),Scalar(H_MAX,S_MAX,V_MAX),threshold2);

		//perform morphological operations on thresholded image to eliminate noise
		//and emphasize the filtered object(s)
		if(useMorphOps)
		morphOps(threshold);
		morphOps(threshold2);
		//pass in thresholded frame to our object tracking function
		//this function will return the x and y coordinates of the
		//filtered object
		if(trackObjects)
			trackFilteredObject(x,y,threshold,cameraFeed);
		    trackFilteredObject(x2,y2,threshold2,cameraFeed2);
		//show frames 
//		imshow(windowName2,threshold);
//		imshow(windowName,cameraFeed);
//		imshow(windowName1,HSV);
		

		
imshow("First Camera Tracking", cameraFeed);
imshow("Second Camera Tracking", cameraFeed2);

imshow("First Camera HSV Smoothed and Thresholded Video", threshold);
imshow("Second Camera HSV Smoothed and Thresholded Video", threshold2);


resizeWindow("First Camera Tracking", 427, 240);
resizeWindow("First Camera HSV Smoothed and Thresholded Video",427, 240);

resizeWindow("Second Camera Tracking",427, 240);
resizeWindow("Second Camera HSV Smoothed and Thresholded Video",427, 240);

		//delay 30ms so that screen can refresh.
		//image will not appear without this waitKey() command
		key = waitKey(30);
	}


	return 0;
}
