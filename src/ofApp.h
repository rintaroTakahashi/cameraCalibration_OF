#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxOpenCv.h"
#include <direct.h>
#include <ctime>

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
		void calibrationPicture();
		
		static bool checkExistenceOfFolder(const string folder_name) {
			if (_mkdir(folder_name.c_str()) == 0) {
				return true;
			}
			else {
				return false;
			}
		}
		Mat _camera_mtx, _camera_dist;

		ofImage inputImg, outputImg;
		int vCount = 7;
		int hCount = 10;
		vector<vector<Point3f>> objectPoints;
		vector<vector<Point2f>> imagePoints;
		vector<Point3f> obj;
		unsigned int numberOfImage = 0;
		Mat mtx, dist;

		Size pictureSize;

		vector<ofImage> imageList;
		ofDirectory dir = "images/";

		ofVideoGrabber grabber;
		std::time_t rawtime;
		std::tm timeinfo;
};