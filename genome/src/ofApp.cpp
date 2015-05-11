#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

	// KINECT -----
	
	kinect.setRegistration(true);
	kinect.init();
	kinect.open();
	
	if(kinect.isConnected()) {
		ofLogNotice() << "sensor-emitter dist: " << kinect.getSensorEmitterDistance() << "cm";
		ofLogNotice() << "sensor-camera dist:  " << kinect.getSensorCameraDistance() << "cm";
		ofLogNotice() << "zero plane pixel size: " << kinect.getZeroPlanePixelSize() << "mm";
		ofLogNotice() << "zero plane dist: " << kinect.getZeroPlaneDistance() << "mm";
	}
	
	colorImage.allocate(kinect.width, kinect.height);
	grayDepthImage.allocate(kinect.width, kinect.height);
	grayThreshNear.allocate(kinect.width, kinect.height);
	grayThreshFar.allocate(kinect.width, kinect.height);
	
	// zero the tilt on startup
	angle = 0;
	kinect.setCameraTiltAngle((int)angle);
	
	nearThreshold = 230;
	farThreshold = 70;
	bShowColor = true;
	
	// GAMEBOARD -----
	gameBoardWidth = 12;
	gameBoardHeight= 6;
	makeGameBoard();
	
	// OSC -----
	sender.setup("localhost", 6010);
	receiver.setup(6001);
	currentBeat = 0;

	// GUI -----
	gui = new ofxUICanvas();
	
	gui->addSlider("WIDTH", 1, 20, &gameBoardWidth);
	gui->addSlider("HEIGHT", 1, 20, &gameBoardHeight);
	gui->addRangeSlider("THRESHOLD", 255, 0, &nearThreshold, &farThreshold);
	gui->addSlider("ANGLE", 0, 30, &angle);
	gui->addToggle("SHOW COLOR", &bShowColor);
	gui->addRangeSlider("BLOB SIZE", 20, (kinect.width * kinect.height) / 3, &minBlobSize, &maxBlobSize);
	gui->addSlider("NUM BLOBS", 1, 20, &numBlobs);
	gui->autoSizeToFitWidgets();
	ofAddListener(gui->newGUIEvent, this, &ofApp::guiEvent); 
	gui->loadSettings("settings.xml");

}

//--------------------------------------------------------------
void ofApp::guiEvent(ofxUIEventArgs &e){

	if (e.getName() == "WIDTH" || e.getName() == "HEIGHT") {
		gameBoardCornerPoints.clear();
		gameBoardInnerPoints.clear();
		makeGameBoard();
	}

	if (e.getName() == "ANGLE") {
		kinect.setCameraTiltAngle((int)angle);
	}

}

//--------------------------------------------------------------
void ofApp::makeGameBoard(){

	int gameBoardInitialSize = 300;
	int offset = 100;

	gameBoardCornerPoints.push_back(ofPoint(offset, offset));
	gameBoardCornerPoints.push_back(ofPoint(offset + gameBoardInitialSize, offset));
	gameBoardCornerPoints.push_back(ofPoint(offset + gameBoardInitialSize, offset + gameBoardInitialSize));
	gameBoardCornerPoints.push_back(ofPoint(offset, offset + gameBoardInitialSize));
	
	for (int y = 0; y <= (int)gameBoardHeight; y++) {
    for (int x = 0; x <= (int)gameBoardWidth; x++) {
			gameBoardInnerPoints.push_back(ofPoint(x, y));
		}
	}
	
}

//--------------------------------------------------------------
void ofApp::updateGameBoard(){
	int i = 0;
	for (float y = 0; y <= 1; y += 1.0 / (int)gameBoardHeight) {
    for (float x = 0; x <= 1; x += 1.0 / (int)gameBoardWidth) {
			
			ofPoint top = gameBoardCornerPoints[0].getInterpolated(gameBoardCornerPoints[1], x);
			ofPoint bottom = gameBoardCornerPoints[3].getInterpolated(gameBoardCornerPoints[2], x);
			
			ofPoint left = gameBoardCornerPoints[0].getInterpolated(gameBoardCornerPoints[3], y);
			ofPoint right = gameBoardCornerPoints[1].getInterpolated(gameBoardCornerPoints[2], y);
			
			float Px = ((top.x * bottom.y - top.y * bottom.x) * (left.x - right.x) - (top.x - bottom.x) * (left.x * right.y - left.y * right.x)) / ((top.x - bottom.x) * (left.y - right.y) - (top.y - bottom.y) * (left.x - right.x));

			float Py = ((top.x * bottom.y - top.y * bottom.x) * (left.y - right.y) - (top.y - bottom.y) * (left.x * right.y - left.y * right.x)) / ((top.x - bottom.x) * (left.y - right.y) - (top.y - bottom.y) * (left.x - right.x));

			gameBoardInnerPoints[i].set(Px, Py);

			i++;
		}
	}
	
	gameBoardSquares.clear();
	gameBoardSquaresFilled.clear();
	for (int i = 0; i < gameBoardInnerPoints.size() - (int)gameBoardWidth - 1; i++) {
		if (i % ((int)gameBoardWidth + 1) != (int)gameBoardWidth) {
			ofPolyline line;
			line.addVertex(gameBoardInnerPoints[i]);
			line.addVertex(gameBoardInnerPoints[i + 1]);
			line.addVertex(gameBoardInnerPoints[i + 2 + (int)gameBoardWidth]);
			line.addVertex(gameBoardInnerPoints[i + 1 + (int)gameBoardWidth]);
			line.addVertex(gameBoardInnerPoints[i]);
			
			gameBoardSquares.push_back(line);
		}
	}
	
}

//--------------------------------------------------------------
void ofApp::update(){
	updateGameBoard();
	
	while (receiver.hasWaitingMessages()) {
		ofxOscMessage m;
		receiver.getNextMessage(&m);
		
		if (m.getAddress() == "/tempo") {
			currentBeat = m.getArgAsInt32(0);
		}
	}
	
	kinect.update();
	if(kinect.isFrameNew()) {
		grayDepthImage.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height);
		
		// threshold image
		grayThreshNear = grayDepthImage;
		grayThreshFar = grayDepthImage;
		grayThreshNear.threshold((int)nearThreshold, true);
		grayThreshFar.threshold((int)farThreshold);
		cvAnd(grayThreshNear.getCvImage(), grayThreshFar.getCvImage(), grayDepthImage.getCvImage(), NULL);
		
		grayDepthImage.flagImageChanged();
		
		contourFinder.findContours(grayDepthImage, (int)minBlobSize, (int)maxBlobSize, (int)numBlobs, false);
		
		for (int i = 0; i < gameBoardSquares.size(); i++) {
			bool filled = false;
			if (i % (int)gameBoardWidth == currentBeat) {
				for (int j = 0; j < contourFinder.nBlobs; j++) {
					if (gameBoardSquares[i].inside(contourFinder.blobs[j].centroid)) {
						filled = true;
						ofLog() << i % (int)gameBoardWidth << " " << floor((float)i / (float)gameBoardWidth);
					}
				}
				if (filled) {
					ofxOscMessage m;
					m.setAddress("/note");
					m.addIntArg(floor((float)i / (float)gameBoardWidth) + 1);
					sender.sendMessage(m);
				}
			}
		}
		
		
		
//		int j = 1;
//		for (int i = 0; i < gameBoardSquaresFilled.size(); i++) {
//			if (i % (int)gameBoardWidth == currentBeat) {
//				if (gameBoardSquaresFilled[i]) {
//					ofxOscMessage m;
//					m.setAddress("/note");
//					m.addIntArg(j);
//					sender.sendMessage(m);
//				}
//				j++;
//			}
//		}
		
	}
	
}



//--------------------------------------------------------------
void ofApp::draw(){

	ofBackground(0);
	
	// kinect
	ofSetColor(255);
	if (bShowColor) {
		kinect.draw(0, 0, ofGetWidth(), ofGetHeight());
	} else {
		grayDepthImage.draw(0, 0, ofGetWidth(), ofGetHeight());
		contourFinder.draw(0, 0, ofGetWidth(), ofGetHeight());
	}
	
	// grid lines
	ofSetColor(255, 0, 255);
	for (int i = 0; i < gameBoardInnerPoints.size() - (int)gameBoardWidth - 1; i++) {
		if (i % ((int)gameBoardWidth + 1) != (int)gameBoardWidth) {
			ofPolyline line;
			line.addVertex(gameBoardInnerPoints[i]);
			line.addVertex(gameBoardInnerPoints[i + 1]);
			line.addVertex(gameBoardInnerPoints[i + 2 + (int)gameBoardWidth]);
			line.addVertex(gameBoardInnerPoints[i + 1 + (int)gameBoardWidth]);
			line.addVertex(gameBoardInnerPoints[i]);
			line.draw();
		}
	}
	
	// filled rectangles (tempo line)
	ofSetColor(128);
	for (int i = 0; i < gameBoardSquares.size(); i++) {
		if (i % (int)gameBoardWidth == currentBeat) {
			ofBeginShape();
				for(int j = 0; j < gameBoardSquares[i].getVertices().size(); j++) {
					ofVertex(gameBoardSquares[i].getVertices().at(j).x, gameBoardSquares[i].getVertices().at(j).y);
				}
			ofEndShape();
		}
	}
	
	// dots
	float radius;
	radius = 10;
	for (int i = 0; i < gameBoardCornerPoints.size(); i++){
		ofCircle(gameBoardCornerPoints[i], radius);
	}

	radius = 5;
	for (int i = 0; i < gameBoardInnerPoints.size(); i++){
		ofCircle(gameBoardInnerPoints[i], radius);
	}

}

//--------------------------------------------------------------
void ofApp::exit(){

	gui->saveSettings("settings.xml");     
	delete gui;

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
	
	float radius = 50;
	for (int i = 0; i < gameBoardCornerPoints.size(); i++){
		ofPoint mouseLoc(x, y);
		if (gameBoardCornerPoints[i].distance(mouseLoc) < radius) {
			gameBoardCornerPoints[i].set(mouseLoc);
		}
	}

}