#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	ofSetVerticalSync(true);
	
    ofBackground(0, 0, 0   );
    fbo.allocate(1280, 720);
    fbo.begin();
    int mod = 0;
    float size = 1280. / 16.;
    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 9; y ++) {
            if(mod%2 == 0)ofSetColor(0);
            else ofSetColor(255,0,255);
            ofRect((float)x * size, (float)y * size, size, size);
            mod++;
        }
    }
    
    fbo.end();
	
	controller.setup(&(fbo.getTextureReference()), "Blend", 400, ofPoint(ofGetWidth()/2, 100));
}

//--------------------------------------------------------------
void testApp::update(){

}

//--------------------------------------------------------------
void testApp::draw(){
	ofSetColor(255);
	
	controller.draw();
	controller.drawGui();
}


//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}