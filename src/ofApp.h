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

		void calibration();
		void chesboarFind();
		void saveCarParams();
		void savePicture();
		void computeReprojectionErrors();

		ofDirectory checkDirectory();
		ofImage loadPicture(int num);
		long getNowtime();
		
		static bool checkExistenceOfFolder(const string folder_name) {
			if (_mkdir(folder_name.c_str()) == 0) {
				return true;
			}
			else {
				return false;
			}
		}
		vector<vector<Point3f>> objectPoints;
		vector<vector<Point2f>> imagePoints;
		vector<Point3f> obj;
		vector<Point3f> outputPoint;
		vector<Mat> rvecs, tvecs;

		Mat mtx, dist;
		Mat _camera_mtx, _camera_dist;
		ofImage outputImg;
		ofDirectory dir;
		Size pictureSize;
		ofVideoGrabber grabber;
		time_t rawtime;
		tm timeinfo;

		float projectionError;
		int vCount = 7;
		int hCount = 10;
};