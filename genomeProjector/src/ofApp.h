#pragma once

#include "ofMain.h"
#include "ofxUI.h"
#include "ofxOsc.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
		void exit();

		void keyPressed(int key);
		void mousePressed(int x, int y, int button);
		void mouseDragged(int x, int y, int button);
	
		// GUI -----
		ofxUICanvas *gui;
		void guiEvent(ofxUIEventArgs &e);
	
		// GAMEBOARD -----
		void makeGameBoard();
		void updateGameBoard();
	
		vector<ofPoint> gameBoardCornerPoints;
		vector<ofPoint> gameBoardInnerPoints;
	
		vector<ofPolyline> gameBoardSquares;
	
		float gameBoardWidth;
		float gameBoardHeight;
	
		// OSC -----
		ofxOscReceiver receiver;
	
		int currentBeat;
};
