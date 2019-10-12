#include "ofApp.h"

using namespace std;
using namespace cv;
using namespace ofxCv;
//https://yuki-sato.com/wordpress/2016/04/15/opencv-%E3%82%AB%E3%83%A1%E3%83%A9%E3%81%AE%E6%AD%AA%E3%81%BF%E3%82%92%E3%81%AA%E3%81%8A%E3%81%99%E3%82%AD%E3%83%A3%E3%83%AA%E3%83%96%E3%83%AC%E3%83%BC%E3%82%B7%E3%83%A7%E3%83%B3-c/
//�����H
//http://opencv.jp/opencv-2svn/cpp/camera_calibration_and_3d_reconstruction.html
//--------------------------------------------------------------
void ofApp::setup(){
	dir = checkDirectory();
	chesboarFind();
	calibration();
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
	switch (key) {
	case 'e':
		computeReprojectionErrors();
		break;
	case 's':
		savePicture();
		break;
	case 'v':
		grabber.setDeviceID(1);
		grabber.initGrabber(4096, 2160);
		break;
	}
}

//�`�F�X�{�[�h�̃R�[�i�[���o
void ofApp::chesboarFind() {
	cout << "chess check" << endl;
	Mat gray, input;
	Size chessboardPatterns(hCount, vCount);
	vector<Point2f> centers;
	ofImage inputImg;
	pictureSize = Size(loadPicture(0).getWidth(), loadPicture(0).getHeight());
	for (int j = 0; j < hCount * vCount; j++) {
		obj.push_back(Point3f(j / hCount, j % vCount, 0.0f));
	}
	cout << "object check " << endl;
	for (unsigned int i = 0; i < dir.size(); i++) {
		input = ofxCv::toCv(loadPicture(i));
		cvtColor(input, gray, CV_BGR2GRAY);
		bool find = findChessboardCorners(gray, chessboardPatterns, centers, CALIB_CB_ADAPTIVE_THRESH| CALIB_CB_NORMALIZE_IMAGE);
		if (find) {
			cornerSubPix(gray, centers, Size(5,5),Size(-1,-1),TermCriteria(TermCriteria::EPS+TermCriteria::COUNT, 30, 0.1));
			objectPoints.push_back(obj);
			imagePoints.push_back(centers);
			cout << "find corner" << i << "/" << dir.size() <<endl;
		}
		else {
			cout << "not found" << string(to_string(i)) << endl;
		}
	}
	cout << "--------------" << endl;
}

//�J�����L�����u���[�V����
void ofApp::calibration() {
	mtx = Mat(3, 3, CV_64F);
	dist = Mat(8, 1, CV_64F);
	//�����E�O���p�����[�^�̐���
	//ofxCv�̃J�����L�����u���[�V����������
	calibrateCamera(objectPoints, imagePoints, pictureSize, mtx, dist, rvecs, tvecs);
	saveCarParams();
	cout << "cal end" << endl;
	cout << "--------------" << endl;
}

//�J�����L�����u���[�V�����p�p�����[�^�ۑ�
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

//��荞�񂾉摜��␳���ĕۑ�����
void ofApp::savePicture() {
	FileStorage fs("params/calibration.yml", FileStorage::READ);
	fs["mtx"] >> _camera_mtx;
	fs["dist"] >> _camera_dist;
	fs.release();
	cout << "create calibration picture " << endl;
	String filepath = "calibrationImg/" + string(to_string(getNowtime())) +string("/");
	checkExistenceOfFolder(filepath);
	for (int i = 0; i < dir.size(); i++) {
		Mat outputMat;
		ofImage output;
		undistort(toCv(loadPicture(i)), outputMat, mtx, dist);
		toOf(outputMat, output);
		output.update();
		output.save(string(filepath) + string(to_string(i)) + string("_cal.png"));
		cout << string(filepath) + string(to_string(i)) + string("_cal.png") << endl;
	}
	cout << "--------------" << endl;
}

//�w��̃t�H���_���摜���擾
ofImage ofApp::loadPicture(int num) {
	//https://qiita.com/FollowTheDarkside/items/278dca14561c347f637c
	ofImage img;
	img.load(dir.getPath(num));
	return img;
}

//���ݎ������擾
long ofApp::getNowtime() {
	std::time(&rawtime);
	localtime_s(&timeinfo, &rawtime);
	long tmpHHmmss = timeinfo.tm_hour * 10000 + timeinfo.tm_min * 100 + timeinfo.tm_sec;
	return tmpHHmmss;
}

//�L�����u���[�V�����p�摜���ۑ�����Ă���f�B���N�g�����擾
ofDirectory ofApp::checkDirectory() {
	string paths = "images/";
	checkExistenceOfFolder(paths);
	ofDirectory d = paths;
	d.listDir();
	d.allowExt(".png");
	d.sort();
	return d;
}

//�ē��e�덷
void ofApp::computeReprojectionErrors() {
	vector<float> pn;
	double totalerror = 0;
	double totalPoints = 0;
	pn.resize(objectPoints.size());
	for (int i = 0; i < obj.size(); i++) {
		double error = 0;
		vector<Point2f> imagePoints2;
		projectPoints(objectPoints[i], rvecs[i], tvecs[i], mtx, dist, imagePoints2);
		error = norm(imagePoints[i], imagePoints2, NORM_L2);
		size_t n  = objectPoints[i].size();
		pn[i] = (float)sqrt(error * error / n);
		totalerror += error * error;
		totalPoints += n;
	}
	projectionError = sqrt(totalerror/totalPoints);
	FileStorage fs("params/calibration.yml", FileStorage::APPEND);
	fs << "error " << projectionError;
	fs.release();
	cout << "error" << projectionError << endl;
}