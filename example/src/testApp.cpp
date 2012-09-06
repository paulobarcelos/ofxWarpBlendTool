#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){	
    ofBackground(0, 0, 0);
    
    testcard.loadImage("testcard.png");
    
	/*controller.setup(// pointer to the texture
                     &(testcard.getTextureReference()),
                     // (optional) name of the panel/save folder ("Warp/Blend")
                     "Warp/Blend",
                     //(optional) drawing offset (default = ofPoint(0,0))
                     ofPoint(ofGetScreenWidth()/2 - testcard.getWidth()/2,ofGetScreenHeight()/2 - testcard.getHeight()/2),
                     // (optional) width of GUI (default = 300)
                     200
                     );*/
    
    //controller.setup(&(testcard.getTextureReference()));
    ofTexture * texture = &(testcard.getTextureReference());
    controller.setup(texture,
                     ofRectangle(texture->getWidth()/2, 0,texture->getWidth()/2, texture->getHeight()));
    
    drawGui = true;
    
}

//--------------------------------------------------------------
void testApp::draw(){
    
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
void testApp::keyPressed(int key){
	if(key == ' ') ofToggleFullscreen();
    if(key == 'g' || key == 'G') drawGui = !drawGui;
}
