#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

	// GAMEBOARD -----
	gameBoardWidth = 12;
	gameBoardHeight= 6;
	makeGameBoard();
	
	ofSetRectMode(OF_RECTMODE_CENTER);
	
	// OSC -----
	receiver.setup(6000);
	currentBeat = 0;

	// GUI -----
	gui = new ofxUICanvas();
	
	gui->addSlider("WIDTH", 1, 20, &gameBoardWidth);
	gui->addSlider("HEIGHT", 1, 20, &gameBoardHeight);
	gui->autoSizeToFitWidgets();
	ofAddListener(gui->newGUIEvent, this, &ofApp::guiEvent); 
	gui->loadSettings("settings.xml");

	

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
void ofApp::guiEvent(ofxUIEventArgs &e){

	if (e.getName() == "WIDTH" || e.getName() == "HEIGHT") {
		gameBoardCornerPoints.clear();
		gameBoardInnerPoints.clear();
		makeGameBoard();
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
void ofApp::draw(){

	ofBackground(0);
	
	
	// grid lines
	ofSetColor(255);
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

	ofSetColor(255);
	
	// corner dots
	float radius = 10;
	for (int i = 0; i < gameBoardCornerPoints.size(); i++){
		ofCircle(gameBoardCornerPoints[i], radius);
	}

	// inner dots
	for (int i = 0; i < gameBoardInnerPoints.size(); i++){
		ofRect(gameBoardInnerPoints[i], 3, 15);
		ofRect(gameBoardInnerPoints[i], 15, 3);
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