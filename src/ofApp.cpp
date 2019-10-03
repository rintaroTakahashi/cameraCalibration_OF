#include "ofApp.h"

using namespace std;
using namespace cv;
using namespace ofxCv;
//https://yuki-sato.com/wordpress/2016/04/15/opencv-%E3%82%AB%E3%83%A1%E3%83%A9%E3%81%AE%E6%AD%AA%E3%81%BF%E3%82%92%E3%81%AA%E3%81%8A%E3%81%99%E3%82%AD%E3%83%A3%E3%83%AA%E3%83%96%E3%83%AC%E3%83%BC%E3%82%B7%E3%83%A7%E3%83%B3-c/
//--------------------------------------------------------------
void ofApp::setup(){
	loadPicture();
	chesboarFind();
	calibration();
	grabber.initGrabber(1280, 720);
	grabber.setDeviceID(0);
}

//--------------------------------------------------------------
void ofApp::update(){
	grabber.update();
	if(grabber.isFrameNew() ){
		Mat frame = toCv(grabber.getPixelsRef());
		Mat outputMat;
		undistort(frame,outputMat,mtx,dist);
		toOf(frame, outputImg);
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

//�t�H���_���̉摜�S���ǂݍ���
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

//�`�F�X�{�[�h�̃R�[�i�[���o
void ofApp::chesboarFind() {
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
			cout << "find : " << obj <<endl;
		}
		else {
			cout << "not found" << endl;
		}
	}
}

//�J�����L�����u���[�V����
void ofApp::calibration() {
	vector<Mat> rvecs, tvecs;
	mtx = Mat(3, 3, CV_64F);
	dist = Mat(8, 1, CV_64F);
	//�����E�O���p�����[�^�̐���
	calibrateCamera(objectPoints, imagePoints, pictureSize, mtx, dist, rvecs, tvecs);
	saveCarParams();
	cout << "cal end" << endl;
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
}