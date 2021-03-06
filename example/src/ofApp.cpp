#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	
    ofBackground(0, 0, 0);
    ofSetVerticalSync(true);
	
	ofSetLogLevel(OF_LOG_VERBOSE);
	
	ofEnableNormalizedTexCoords();
	ofDisableArbTex();
    
    // We will use this flag to decide if we should draw the GUI or not
    drawGui = true;
    
    // Pointer to the texture
    testcard.load("testcard.png");
    ofTexture * texture = &(testcard.getTexture());
    
    // Size of the quad you will work with
    ofVec2f size(testcard.getWidth(), testcard.getHeight());
    
    // Subsection of the texture coordinates
    //ofRectangle subsection(ofPoint(0.5,0.0), ofPoint(1.0,1.0));
    
    // Inital perspective corners position (order is top-left, top-right, bottom-left, bottom-right).
    // In this example, let's position it agains the right corner of the screen.
    ofPoint corners[4];
    corners[0].x = ofGetWidth() - testcard.getWidth();
    corners[0].y = 0;
    corners[1].x = ofGetWidth();
    corners[1].y = 0;
    corners[2].x = ofGetWidth();
    corners[2].y = testcard.getHeight();
    corners[3].x = ofGetWidth() - testcard.getWidth();
    corners[3].y = testcard.getHeight();
    
    // Name for the controller panel (this will also define the folder in which the data will be saved)
    string name = "Warp/Blend";
    
    // Size of the GUI elements (a wider GUI gives your more precision to control the texture coordinates)
    float guiWidth = 250;
    float guiHeight = 15;
    
    // Setup!
    controller.setup(texture, size, ofRectangle(0, 0, 0, 0), corners, name, guiWidth, guiHeight);
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofSetColor(255);
    controller.draw();
	if(drawGui)controller.drawGui();
    
	ofSetColor(100);
    ofDrawBitmapString(ofToString(ofGetFrameRate()), 20, 20);
    
    ofSetColor(255);
    ofDrawBitmapString("Press SPACE to toggle fullscreen.", 20, 40);
    ofDrawBitmapString("Press G to toggle GUI.", 20, 55);
    if(drawGui){
#ifdef TARGET_OSX
        ofDrawBitmapString("Use CMD+Z / CMD+SHIFT+Z to undo/redo changes.", 20, 100);
        ofDrawBitmapString("Use CMD+S to save changes.", 20, 115);
        ofDrawBitmapString("Use CMD+R to reload changes.", 20, 130);
#else
        ofDrawBitmapString("Use CTRL+Z / CTRL+SHIFT+Z to undo/redo changes.", 20, 100);
        ofDrawBitmapString("Use CTRL+S to save changes.", 20, 115);
        ofDrawBitmapString("Use CTRL+R to reload changes.", 20, 1130);
#endif
    }
	
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if(key == ' ') ofToggleFullscreen();
    if(key == 'g' || key == 'G') drawGui = !drawGui;
}
