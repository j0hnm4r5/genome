#pragma once

#include "ofMain.h"
#include "ofxKinect.h"
#include "ofxOpenCv.h"
#include "ofxOsc.h"
#include "ofxSecondWindow.h"
#include "ofxUI.h"

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
	
		// KINECT -----
		ofxKinect kinect;
		ofxCvColorImage colorImage;
		ofxCvGrayscaleImage grayDepthImage;
		ofxCvGrayscaleImage grayThreshNear;
		ofxCvGrayscaleImage grayThreshFar;
	
		ofxCvContourFinder contourFinder;
		float minBlobSize;
		float maxBlobSize;
		float numBlobs;
		bool bHoles;
	
		float nearThreshold;
		float farThreshold;
	
		float angle;
	
		bool bShowColor;
	
		// GAMEBOARD -----
		void makeGameBoard();
		void updateGameBoard();
	
		vector<ofPoint> gameBoardCornerPoints;
		vector<ofPoint> gameBoardInnerPoints;
	
		vector<ofPolyline> gameBoardSquares;
		vector<bool> gameBoardSquaresFilled;
	
		float gameBoardWidth;
		float gameBoardHeight;
	
		// OSC -----
		ofxOscSender sender;
		ofxOscReceiver receiver;
	
		int currentBeat;
		
};
