#include "ofApp.h"

using namespace std;
using namespace cv;
using namespace ofxCv;
//https://yuki-sato.com/wordpress/2016/04/15/opencv-%E3%82%AB%E3%83%A1%E3%83%A9%E3%81%AE%E6%AD%AA%E3%81%BF%E3%82%92%E3%81%AA%E3%81%8A%E3%81%99%E3%82%AD%E3%83%A3%E3%83%AA%E3%83%96%E3%83%AC%E3%83%BC%E3%82%B7%E3%83%A7%E3%83%B3-c/
//公式？
//http://opencv.jp/opencv-2svn/cpp/camera_calibration_and_3d_reconstruction.html
//--------------------------------------------------------------
void ofApp::setup() {
	dir = checkDirectory();
}

//--------------------------------------------------------------
void ofApp::update() {
	grabber.update();
	if (grabber.isFrameNew()) {
		Mat outputMat;
		undistort(toCv(grabber.getPixelsRef()), outputMat, _camera_mtx, _camera_dist);
		toOf(outputMat, outputImg);
		outputImg.update();
	}
}

//--------------------------------------------------------------
void ofApp::draw() {
	outputImg.draw(0, 0, 1024, 576);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	switch (key) {
	case 'q':
		chesboarFind();
		calibration();
		saveCalParams();
		break;
	case 'r':
		chesboarFind();
		readyml(read_path);
		cout << "read param" << endl;
		cout << "--------------" << endl;
		break;
	case 'e':
		computeReprojectionErrors();
		cout << "--------------" << endl;
		break;
	case 's':
		savePicture();
		cout << "--------------" << endl;
		break;
	case 'v':
		grabber.setDeviceID(1);
		grabber.initGrabber(4096, 2160);
		break;
	}
}

//---------------------------------------------------------------
//キャリブレーション用のメイン機能
//---------------------------------------------------------------

//チェスボードのコーナー検出
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
		input = toCv(loadPicture(i));
		cvtColor(input, gray, CV_BGR2GRAY);
		bool find = findChessboardCorners(gray, chessboardPatterns, centers, CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_NORMALIZE_IMAGE);
		if (find) {
			cornerSubPix(gray, centers, Size(5, 5), Size(-1, -1), TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 30, 0.1));
			objectPoints.push_back(obj);
			imagePoints.push_back(centers);
			cout << "find corner" << i + 1 << "/" << dir.size() << endl;
		}
		else {
			cout << "not found" << i + 1 << "/" << dir.size() << endl;
		}
	}
	cout << "--------------" << endl;
}

//カメラキャリブレーション
void ofApp::calibration() {
	mtx = Mat(3, 3, CV_64F);
	dist = Mat(8, 1, CV_64F);
	//内部・外部パラメータの推定
	calibrateCamera(objectPoints, imagePoints, pictureSize, mtx, dist, rvecs, tvecs);
	cout << "cal end" << endl;
	cout << "--------------" << endl;
}

//カメラキャリブレーション用パラメータ保存
void ofApp::saveCalParams() {
	String path = "params/";
	checkExistenceOfFolder(path);
	String filepath = path + "calibration.yml";
	writeyml(path, mtx, dist);
	cout << "save cal params" << endl;
	cout << "--------------" << endl;
}

//取り込んだ画像を補正して保存する
void ofApp::savePicture() {
	cout << "create calibration picture " << endl;
	String filepath = "calibrationImg/" + string(to_string(getNowtime())) + string("/");
	checkExistenceOfFolder(filepath);
	int doCalNum = dir.size();
	if (doCalNum > 10) {
		//枚数が多いと書き出しがつらいので
		doCalNum = 10;
	}
	if (mtx.empty() && !_camera_mtx.empty()) {
		mtx = _camera_mtx;
		dist = _camera_dist;
	}
	else if (mtx.empty() && _camera_mtx.empty()) {

		return;
	}
	for (int i = 0; i < doCalNum; i++) {
		Mat outputMat;
		ofImage output;
		undistort(toCv(loadPicture(i)), outputMat, mtx, dist);
		toOf(outputMat, output);
		output.update();
		output.save(string(filepath) + string(to_string(i)) + string("_cal.png"));
		cout << string(filepath) + string(to_string(i)) + string("_cal.png") << endl;
	}
}

//---------------------------------------------------------------
// サブ機能
//---------------------------------------------------------------

//指定のフォルダ内画像を取得
ofImage ofApp::loadPicture(int num) {
	//https://qiita.com/FollowTheDarkside/items/278dca14561c347f637c
	ofImage img;
	img.load(dir.getPath(num));
	return img;
}

//現在時刻を取得
long ofApp::getNowtime() {
	time(&rawtime);
	localtime_s(&timeinfo, &rawtime);
	long tmpHHmmss = timeinfo.tm_hour * 10000 + timeinfo.tm_min * 100 + timeinfo.tm_sec;
	return tmpHHmmss;
}

//キャリブレーション用画像が保存されているディレクトリを取得
ofDirectory ofApp::checkDirectory() {
	ofDirectory d = "images/";
	d.listDir();
	d.allowExt(".png");
	d.sort();
	return d;
}

//再投影誤差
void ofApp::computeReprojectionErrors() {
	double totalerror = 0;
	double totalPoints = 0;
	float projectionError = 0;
	if (objectPoints.empty() || rvecs.empty() || tvecs.empty()) {
		cout << "error ReprojectionErrors" << endl;
		return;
	}
	if (mtx.empty() && !_camera_mtx.empty()) {
		mtx = _camera_mtx;
		dist = _camera_dist;
	}
	for (int i = 0; i < objectPoints.size(); i++) {
		double error = 0;
		vector<Point2f> imagePoints2;
		projectPoints(objectPoints[i], rvecs[i], tvecs[i], mtx, dist, imagePoints2);
		error = norm(imagePoints[i], imagePoints2, NORM_L2);
		size_t n = objectPoints[i].size();
		totalerror += error * error;
		totalPoints += n;
	}
	projectionError = sqrt(totalerror / totalPoints);
	appendyml(param_path, to_string(projectionError));
	cout << "error" << projectionError << endl;
}

//ymlファイル読み取り
void ofApp::readyml(String path) {
	FileStorage fs(path, FileStorage::READ);
	fs["mtx"] >> _camera_mtx;
	fs["dist"] >> _camera_dist;
	fs.release();
}

//ymlファイル書き出し
void ofApp::writeyml(String path, Mat _mtx, Mat _dist) {
	FileStorage fs(path, FileStorage::WRITE);
	fs << "mtx" << _mtx;
	fs << "dist" << _dist;
	fs.release();
}

//ymlファイル追加書き出し
void ofApp::appendyml(String path, String message) {
	FileStorage fs(path, FileStorage::APPEND);
	fs << "error " << message;
	fs.release();
}