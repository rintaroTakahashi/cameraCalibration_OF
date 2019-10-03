#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxOpenCv.h"
#include <direct.h>

using namespace cv;
using namespace std;

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);

		void loadPicture();
		void calibration();
		void chesboarFind();
		void saveCarParams();
		
		static bool checkExistenceOfFolder(const string folder_name) {
			if (_mkdir(folder_name.c_str()) == 0) {
				return true;
			}
			else {
				return false;
			}
		}

		ofImage inputImg, outputImg;
		int vCount = 10;
		int hCount = 7;
		vector<vector<Point3f>> objectPoints;
		vector<vector<Point2f>> imagePoints;
		vector<Point3f> obj;
		unsigned int numberOfImage = 0;
		Mat mtx, dist;

		Size pictureSize;

		vector<ofImage> imageList;
		ofDirectory dir = "images/";

		ofVideoGrabber grabber;
};