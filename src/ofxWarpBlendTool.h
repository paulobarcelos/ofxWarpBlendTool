#pragma once
#include "ofMain.h"
#include "ofxGui.h"
#include "ofxGlWarper.h"

namespace ofxWarpBlendTool {
	class SelectablePoint: public ofPoint {
	public:
		SelectablePoint(){
			selected = false;
		}
		bool selected;
	};
	
	class BaseQuad{
	public:
		int index;
		SelectablePoint TL;
		SelectablePoint TR;
		SelectablePoint BL;
		SelectablePoint BR;
	};
	class InternalQuad : public BaseQuad{
	public:
		int x;
		int y;
	};
	
	
	class ControlQuad : public BaseQuad{
	public:
		vector<InternalQuad*> internalQuads;
	};

	
	class Controller{
		
	public:
		void setup(ofTexture * texture, string name = "Warp/Blend", float guiWidth = 400, ofPoint initialOffset = ofPoint(0,0));
		void draw();
		void drawGui();
		
		void keyPressed(ofKeyEventArgs & args);
		void keyReleased(ofKeyEventArgs & args);
		void mouseMoved(ofMouseEventArgs & args);
		void mouseDragged(ofMouseEventArgs & args);
		void mousePressed(ofMouseEventArgs & args);
		void mouseReleased(ofMouseEventArgs & args);
		
	protected:
		void onSave(bool & value);
		void onLoad(bool & value);
		void onBlendChange(float & value);
		void onGridChange(int & value);
		void onCoordinateschange(float & value);
		void onEnablePerpective(bool & value);
		
		void selectVertex(float mouseX, float mouseY);
		ofPoint getInteractionOffset(float mouseX, float mouseY);
		void updateVertices();
		void saveVertices();
		void loadVertices();
		
		void drawEvent(ofEventArgs& args);
		bool drawing, drawn;
		
		
		string safe_string(string str) {
			string safe = str;
			for (int i = 0; i < strlen(str.c_str()); i++) {
				if (isalpha(str[i]))
					safe[i] = str[i];
				else
					safe[i] = '_';
			}
			return safe;
		}


		ofTexture * texture;
		ofxPanel gui;		
		ofxGLWarper perspective;
				
		ofVboMesh controlMesh;
		ofVboMesh internalMesh;
		
		vector<ControlQuad*> controlQuads;
		
		ofVec2f gridSize;
		ofVec2f resolution;
		
		ofVec2f coordinatesStart;
		ofVec2f coordinatesEnd;		
		
	
		string name;
		string guiFile, perspectiveFile, meshFile;
		int lastClickTime;
		bool isMovingVertex;
		ofPoint interactionOffset;
		ofVec2f mouse;
		
		float blendL, blendR, blendT, blendB;
		
		ofFbo guiHelperFbo;
		ofPoint initialOffset;
	};
}