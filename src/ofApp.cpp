#include "ofApp.h"

using namespace std;
using namespace cv;
using namespace ofxCv;
//https://yuki-sato.com/wordpress/2016/04/15/opencv-%E3%82%AB%E3%83%A1%E3%83%A9%E3%81%AE%E6%AD%AA%E3%81%BF%E3%82%92%E3%81%AA%E3%81%8A%E3%81%99%E3%82%AD%E3%83%A3%E3%83%AA%E3%83%96%E3%83%AC%E3%83%BC%E3%82%B7%E3%83%A7%E3%83%B3-c/
//公式？
//http://opencv.jp/opencv-2svn/cpp/camera_calibration_and_3d_reconstruction.html
//--------------------------------------------------------------
void ofApp::setup(){
	loadPicture();
	chesboarFind();
	calibration();
	calibrationPicture();
	grabber.setDeviceID(1);
	grabber.initGrabber(4096, 2160);
}

//--------------------------------------------------------------
void ofApp::update(){
	grabber.update();
	if(grabber.isFrameNew() ){
		Mat frame = toCv(grabber.getPixelsRef());
		Mat outputMat;
		undistort(frame,outputMat, _camera_mtx, _camera_dist);
		toOf(outputMat, outputImg);
		outputImg.update();
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	outputImg.draw(0,0,1024,576);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if (key == 'e') {
		exit();
	}
}

//フォルダ内の画像全部読み込み
void ofApp::loadPicture() {
	//https://qiita.com/FollowTheDarkside/items/278dca14561c347f637c
	dir.listDir();
	dir.allowExt(".png");
	dir.sort();
	for (int i = 0; i < dir.size(); i++) {
		ofImage img;
		img.load(dir.getPath(i));
		imageList.push_back(img);
		numberOfImage++;
	}
	for (int j = 0; j < hCount * vCount; j++) {
		obj.push_back(Point3f(j / hCount, j % vCount, 0.0f));
	}
	pictureSize = Size(imageList[0].getWidth(),imageList[0].getHeight());
	cout << "picture num" << numberOfImage <<endl;
}

//チェスボードのコーナー検出
void ofApp::chesboarFind() {
	cout << "corner find" << endl;
	Mat gray, input;
	for (unsigned int i = 0; i < numberOfImage; i++) {
		input = ofxCv::toCv(imageList[i]);
		cvtColor(input, gray, CV_BGR2GRAY);
		Size chessboardPatterns(hCount,vCount);
		vector<Point2f> centers;
		bool find = findChessboardCorners(gray, chessboardPatterns, centers, CALIB_CB_ADAPTIVE_THRESH| CALIB_CB_NORMALIZE_IMAGE);
		if (find) {
			cornerSubPix(gray, centers, Size(11,11),Size(-1,-1),TermCriteria(TermCriteria::EPS+TermCriteria::COUNT, 30, 0.1));
			objectPoints.push_back(obj);
			imagePoints.push_back(centers);
			cout << "read "  << string(to_string(i)) << endl;
			cout << "find corner" <<endl;
		}
		else {
			cout << "not found" << endl;
		}
	}
	cout << "--------------" << endl;
}

//カメラキャリブレーション
void ofApp::calibration() {
	vector<Mat> rvecs, tvecs;
	mtx = Mat(3, 3, CV_64F);
	dist = Mat(8, 1, CV_64F);
	//内部・外部パラメータの推定
	//ofxCvのカメラキャリブレーションもある
	calibrateCamera(objectPoints, imagePoints, pictureSize, mtx, dist, rvecs, tvecs);
	saveCarParams();
	cout << "cal end" << endl;
	cout << "--------------" << endl;

}

//カメラキャリブレーション用パラメータ保存
void ofApp::saveCarParams() {
	String path = "params/";
	checkExistenceOfFolder(path);

	String filepath = path + "calibration.yml";
	FileStorage fs (filepath, FileStorage::WRITE);
	fs << "mtx" << mtx;
	fs << "dist" << dist;
	fs.release();
	cout << "save cal params" << endl;
	cout << "--------------" << endl;

}

void ofApp::calibrationPicture() {
	FileStorage fs("params/calibration.yml", FileStorage::READ);
	fs["mtx"] >> _camera_mtx;
	fs["dist"] >> _camera_dist;
	fs.release();
	cout << "create calibration picture " << endl;
	std::time(&rawtime);
	localtime_s(&timeinfo, &rawtime);
	long tmpHHmmss = timeinfo.tm_hour * 10000 + timeinfo.tm_min * 100 + timeinfo.tm_sec;
	String filepath = "calibrationImg/" + string(to_string(tmpHHmmss)) +string("/");
	checkExistenceOfFolder(filepath);

	for (int i = 0; i < imageList.size(); i++) {
		Mat outputMat;
		ofImage output;
		undistort(toCv(imageList[i]), outputMat, mtx, dist);
		toOf(outputMat, output);
		output.update();
		output.save(string(filepath) + string(to_string(i)) + string("_cal.png"));
		cout << string(filepath) + string(to_string(i)) + string("_cal.png") << endl;
	}
	imageList.clear();
	cout << "--------------" << endl;
}