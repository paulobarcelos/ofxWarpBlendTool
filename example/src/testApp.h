#pragma once

#include "ofMain.h"
#include "ofxWarpBlendTool.h"


class testApp : public ofBaseApp{
public:
    void setup();
    void draw();
    
    void keyPressed(int key);
    
    ofImage testcard;
	ofxWarpBlendTool::Controller controller;
    bool drawGui;
};
