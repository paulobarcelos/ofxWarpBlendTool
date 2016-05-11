#include "ofxWarpBlendTool.h"
using namespace ofxWarpBlendTool;

const string Controller::defaultName = "Warp/Blend";
const ofVec2f Controller::defaultOriginalSize = ofVec2f(0,0); // so we can set to texture measurements on runtime
const ofRectangle Controller::defaultOriginalCoordinates = ofRectangle(0,0,0,0); // so we can set to texture measurements on runtime
const ofPoint Controller::defaultOriginalPerspective[4] = {ofPoint(),ofPoint(),ofPoint(),ofPoint()}; // so we can set to texture measurements on runtime
const float Controller::defaultGuiWidth = 250;
const float Controller::defaultGuiHeight = 15;

Controller::Controller(){
    texture = NULL;
    name = defaultName;
    originalCoordinates = defaultOriginalCoordinates;
    memcpy (originalPerspective,defaultOriginalPerspective,sizeof(ofPoint)*4);
    guiWidth = defaultGuiWidth;
    guiHeight = defaultGuiHeight;
    scissor = ofRectangle(0,0, ofGetWidth(), ofGetHeight());
	postBrt = postSat = postBMult = postRMult = postGMult = postBMult = 1.0;
    scissorEnabled = false;
}
void Controller::setTexture(ofTexture * texture){
    if(!this->texture){
        setup(texture, originalSize, originalCoordinates, originalPerspective, name, guiWidth, guiHeight);
    }
    else this->texture = texture;
}
void Controller::setup(ofTexture * texture, ofVec2f originalSize, ofRectangle originalCoordinates, ofPoint originalPerspective[4], string name, float guiWidth, float guiHeight ){
    // Arguments
    this->texture = texture;
	this->name = name;
    this->guiWidth = guiWidth;
    this->guiHeight = guiHeight;
    if(originalSize.x==0.0
       && originalSize.y==0.0){
        originalSize.set(texture->getWidth(), texture->getHeight());
    }
    this->originalSize = originalSize;
    if(originalCoordinates.x==0.0
       && originalCoordinates.y==0.0
       && originalCoordinates.width==0.0
       && originalCoordinates.height==0.0){
        if(ofGetUsingNormalizedTexCoords())originalCoordinates.set(0.0, 0.0, 1.0, 1.0);
        else originalCoordinates.set(0.0, 0.0, texture->getWidth(), texture->getHeight());
    }
    this->originalCoordinates = originalCoordinates;
    if(originalPerspective[0].x == 0 && originalPerspective[0].y == 0
       && originalPerspective[1].x == 0 && originalPerspective[1].y == 0
       && originalPerspective[2].x == 0 && originalPerspective[2].y == 0
       && originalPerspective[3].x == 0 && originalPerspective[3].y == 0){
        originalPerspective[0].x = ofGetWidth()/2 - texture->getWidth()/2;
        originalPerspective[0].y = ofGetHeight()/2 - texture->getHeight()/2;
        originalPerspective[1].x = ofGetWidth()/2 + texture->getWidth()/2;
        originalPerspective[1].y = ofGetHeight()/2 - texture->getHeight()/2;
        originalPerspective[2].x = ofGetWidth()/2 + texture->getWidth()/2;
        originalPerspective[2].y = ofGetHeight()/2 + texture->getHeight()/2;
        originalPerspective[3].x = ofGetWidth()/2 - texture->getWidth()/2;
        originalPerspective[3].y = ofGetHeight()/2 + texture->getHeight()/2;
    }
    memcpy(this->originalPerspective,originalPerspective,sizeof(ofPoint)*4);
    
	// Register draw event for the enable/disable magic
	ofAddListener(ofEvents().draw, this, &Controller::drawEvent);
	drawing = false;
    drawn = true;
	
	// Some intial setup
	guiHelperFbo.allocate(150, 60);
	blendB = blendL = blendR = blendT = 0;
	lastClickTime = ofGetElapsedTimeMillis();
	historyIndex = -1;
    guiHasChanged = false;
    perspectiveHasChanged = false;

	// Generate filenames
	safename = safe_string(name);
	stringstream ss_guiFile;
	ss_guiFile << safename << "/gui.xml";	
	guiFile = ss_guiFile.str();
	
	stringstream ss_perspectiveFile;
	ss_perspectiveFile << safename << "/perspective.xml";
	perspectiveFile = ss_perspectiveFile.str();
	
	stringstream ss_meshFile;
	ss_meshFile << safename << "/mesh";
	meshFile = ss_meshFile.str();
	
	// Perspective
    perspective.setup(0, 0, getWindowWidth(), getWindowHeight());
    perspective.setCornerSensibility(0.03);
    perspective.activate();
    ofAddListener(perspective.changeEvent, this, &Controller::onPerspectiveChange);
    
	// Meshes
    controlMesh.setMode(OF_PRIMITIVE_POINTS);
	internalMesh.setMode(OF_PRIMITIVE_TRIANGLES);
	
	// GUI
    gui.setup(name, guiFile,guiWidth, guiHeight);
	gui.setPosition(originalPerspective[0].x, originalPerspective[0].y);
	
	ofxButton * load = new ofxButton();
    load->setup("Load", guiWidth, guiHeight);
    load->addListener(this, &Controller::onLoad);
    gui.add(load);
	
	ofxButton * save = new ofxButton();
    save->setup("Save", guiWidth, guiHeight);
    save->addListener(this, &Controller::onSave);
    gui.add(save);
    
    ofxToggle * enablePerspective = new ofxToggle();
    enablePerspective->setup("Perspective Warp", true, guiWidth, guiHeight);
    enablePerspective->addListener(this, &Controller::onEnablePerspective);
    gui.add(enablePerspective);
    
    ofxButton * resetPerspective = new ofxButton();
    resetPerspective->setup("Reset Perspective", guiWidth, guiHeight);
    resetPerspective->addListener(this, &Controller::onResetPerspective);
    gui.add(resetPerspective);
    
    ofxButton * resetMesh = new ofxButton();
    resetMesh->setup("Reset Mesh", guiWidth, guiHeight);
    resetMesh->addListener(this, &Controller::onResetMesh);
    gui.add(resetMesh);
    
    ofxIntSlider * resolutionX = new ofxIntSlider();
    resolutionX->setup("Horizontal Resolution", 4, 1, 20, guiWidth, guiHeight);
    resolutionX->addListener(this, &Controller::onGridChange);
    gui.add(resolutionX);
	
	ofxIntSlider * resolutionY = new ofxIntSlider();
    resolutionY->setup("Vertical Resolution", 4, 1, 20, guiWidth, guiHeight);
    resolutionY->addListener(this, &Controller::onGridChange);
    gui.add(resolutionY);
    
    ofxIntSlider * columns = new ofxIntSlider();
    columns->setup("Grid Columns", 4, 1, 20, guiWidth, guiHeight);
    columns->addListener(this, &Controller::onGridChange);
    gui.add(columns);
    
    ofxIntSlider * rows = new ofxIntSlider();
    rows->setup("Grid Rows", 4, 1, 20, guiWidth, guiHeight);
    rows->addListener(this, &Controller::onGridChange);
    gui.add(rows);
    
    ofxFloatSlider * startX = new ofxFloatSlider();
	float startXmin = originalCoordinates.x;
	float startXmax = originalCoordinates.x+originalCoordinates.width;
	float startXmargin = (startXmax - startXmin) * 0.3333;
    startX->setup("UV Start X", startXmin, startXmin - startXmargin, startXmax + startXmargin, guiWidth, guiHeight);
    startX->addListener(this, &Controller::onCoordinatesChange);
    gui.add(startX);
    
    ofxFloatSlider * startY = new ofxFloatSlider();
	float startYmin = originalCoordinates.y;
	float startYmax = originalCoordinates.y+originalCoordinates.height;
	float startYmargin = (startYmax - startYmin) * 0.3333;
    startY->setup("UV Start Y", startYmin, startYmin - startYmargin, startYmax + startYmargin, guiWidth, guiHeight);
    startY->addListener(this, &Controller::onCoordinatesChange);
    gui.add(startY);
    
    ofxFloatSlider * endX = new ofxFloatSlider();
	float endXmin = originalCoordinates.x;
	float endXmax = originalCoordinates.x+originalCoordinates.width;
	float endXmargin = (endXmax - endXmin) * 0.3333;
    endX->setup("UV End X", endXmax, endXmin - endXmargin, endXmax + endXmargin, guiWidth, guiHeight);
    endX->addListener(this, &Controller::onCoordinatesChange);
    gui.add(endX);
    
    ofxFloatSlider * endY = new ofxFloatSlider();
	float endYmin = originalCoordinates.y;
	float endYmax = originalCoordinates.y+originalCoordinates.height;
	float endYmargin = (endYmax - endYmin) * 0.3333;
    endY->setup("UV End Y", endYmax, endYmin - endYmargin, endYmax + endYmargin, guiWidth, guiHeight);
    endY->addListener(this, &Controller::onCoordinatesChange);
    gui.add(endY);
	
	ofxFloatSlider * blendTSlider = new ofxFloatSlider();
    blendTSlider->setup("Blend Top", 0, 0, 1, guiWidth, guiHeight);
    blendTSlider->addListener(this, &Controller::onBlendChange);
    gui.add(blendTSlider);
	
	ofxFloatSlider * blendDSlider = new ofxFloatSlider();
    blendDSlider->setup("Blend Down", 0, 0, 1, guiWidth, guiHeight);
    blendDSlider->addListener(this, &Controller::onBlendChange);
    gui.add(blendDSlider);
	
	ofxFloatSlider * blendLSlider = new ofxFloatSlider();
    blendLSlider->setup("Blend Left", 0, 0, 1, guiWidth, guiHeight);
    blendLSlider->addListener(this, &Controller::onBlendChange);
    gui.add(blendLSlider);
	
	ofxFloatSlider * blendRSlider = new ofxFloatSlider();
    blendRSlider->setup("Blend Right", 0, 0, 1, guiWidth, guiHeight);
    blendRSlider->addListener(this, &Controller::onBlendChange);
    gui.add(blendRSlider);
    
    ofxToggle * scissorEnable = new ofxToggle();
    scissorEnable->setup("Scissor Active", false, guiWidth, guiHeight);
    scissorEnable->addListener(this, &Controller::onScissorEnabled);
    gui.add(scissorEnable);
    
    ofxIntSlider * scissorX = new ofxIntSlider();
    scissorX->setup("Scissor Start X", 0, 0, ofGetScreenWidth(), guiWidth, guiHeight);
    scissorX->addListener(this, &Controller::onScissorChange);
    gui.add(scissorX);
    
    ofxIntSlider * scissorY = new ofxIntSlider();
    scissorY->setup("Scissor Start Y", 0, 0, ofGetScreenHeight(), guiWidth, guiHeight);
    scissorY->addListener(this, &Controller::onScissorChange);
    gui.add(scissorY);
    
    ofxIntSlider * scissorWidth = new ofxIntSlider();
    scissorWidth->setup("Scissor End X", ofGetScreenWidth(), 0, ofGetScreenWidth(), guiWidth, guiHeight);
    scissorWidth->addListener(this, &Controller::onScissorChange);
    gui.add(scissorWidth);
    
    ofxIntSlider * scissorHeight = new ofxIntSlider();
    scissorHeight->setup("Scissor End Y", ofGetScreenHeight(), 0, ofGetScreenHeight(), guiWidth, guiHeight);
    scissorHeight->addListener(this, &Controller::onScissorChange);
    gui.add(scissorHeight);
	
	
	ofxFloatSlider * postBrtSlider = new ofxFloatSlider();
    postBrtSlider->setup("Brighteness", 1, 0, 5, guiWidth, guiHeight);
    postBrtSlider->addListener(this, &Controller::onPostProcessingValueChanged);
    gui.add(postBrtSlider);
	
	ofxFloatSlider * postConSlider = new ofxFloatSlider();
    postConSlider->setup("Contrast", 1, 0, 5, guiWidth, guiHeight);
    postConSlider->addListener(this, &Controller::onPostProcessingValueChanged);
    gui.add(postConSlider);
	
	ofxFloatSlider * postSatSlider = new ofxFloatSlider();
    postSatSlider->setup("Saturation", 1, 0, 5, guiWidth, guiHeight);
    postSatSlider->addListener(this, &Controller::onPostProcessingValueChanged);
    gui.add(postSatSlider);
	
	ofxFloatSlider * postRMultSlider = new ofxFloatSlider();
    postRMultSlider->setup("Red Multiplier", 1, 0, 5, guiWidth, guiHeight);
    postRMultSlider->addListener(this, &Controller::onPostProcessingValueChanged);
    gui.add(postRMultSlider);
	
	ofxFloatSlider * postGmultSlider = new ofxFloatSlider();
    postGmultSlider->setup("Green Multiplier", 1, 0, 5, guiWidth, guiHeight);
    postGmultSlider->addListener(this, &Controller::onPostProcessingValueChanged);
    gui.add(postGmultSlider);
	
	ofxFloatSlider * postBMultSlider = new ofxFloatSlider();
    postBMultSlider->setup("Blue Multipler", 1, 0, 5, guiWidth, guiHeight);
    postBMultSlider->addListener(this, &Controller::onPostProcessingValueChanged);
    gui.add(postBMultSlider);
	
	ofxIntSlider * lineThickness = new ofxIntSlider();
    lineThickness->setup("GUI Lines Thickness", 1, 1, 10, guiWidth, guiHeight);
    lineThickness->addListener(this, &Controller::onGuiLinesThicknessChange);
    gui.add(lineThickness);
    
	
	// Post processing
    if(ofIsGLProgrammableRenderer()){
	shader.setupShaderFromSource(GL_VERTEX_SHADER, VertShaderProgrammable);
	if(ofGetUsingNormalizedTexCoords()) shader.setupShaderFromSource(GL_FRAGMENT_SHADER, NormalizedFragShaderProgrammable);
        else shader.setupShaderFromSource(GL_FRAGMENT_SHADER, UnnormalizedFragShaderProgrammable);
        shader.bindDefaults();
	shader.linkProgram();
    }
    else {
        shader.setupShaderFromSource(GL_VERTEX_SHADER, VertShader);
	if(ofGetUsingNormalizedTexCoords()) shader.setupShaderFromSource(GL_FRAGMENT_SHADER, NormalizedFragShader);
	else shader.setupShaderFromSource(GL_FRAGMENT_SHADER, UnnormalizedFragShader);
	shader.linkProgram();
    }
	
	// load settings
	onLoad();
	
	
}
void Controller::draw(){
    if(scissorEnabled){
        glEnable(GL_SCISSOR_TEST);
        glScissor(scissor.x, ofGetHeight()-scissor.y-scissor.height, scissor.width, scissor.height);
    }
    
	perspective.begin();


         
	shader.begin();
        
	shader.setUniformTexture("tex0", *texture, 0 );
	shader.setUniform1f("brt", postBrt );
	shader.setUniform1f("sat", postSat );
	shader.setUniform1f("con", postCon );
	shader.setUniform1f("rMult", postRMult );
	shader.setUniform1f("gMult", postGMult );
	shader.setUniform1f("bMult", postBMult );

	internalMesh.draw();

        shader.end();


        ofMesh mesh;
        mesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
        if(blendT>0){
            mesh.addColor(ofFloatColor(0,0,0,1));
            mesh.addVertex(ofPoint(0,0,0));
            mesh.addColor(ofFloatColor(0,0,0,1));
            mesh.addVertex(ofPoint(getWindowWidth(), 0, 0));
            mesh.addColor(ofFloatColor(0,0,0,0));
            mesh.addVertex(ofPoint( 0.0f, blendT*getWindowHeight(), 0.0f ));
            mesh.addColor(ofFloatColor(0,0,0,0));
            mesh.addVertex(ofPoint( getWindowWidth(), blendT*getWindowHeight(), 0.0f ));
            mesh.draw();
            mesh.clear();
        }
        if(blendB>0){
            mesh.addColor(ofFloatColor(0, 0, 0, 1));
            mesh.addVertex(ofPoint( 0.0f, getWindowHeight(), 0.0f ));
            mesh.addColor(ofFloatColor(0, 0, 0, 1));
            mesh.addVertex(ofPoint( getWindowWidth(), getWindowHeight(), 0.0f ));
            mesh.addColor(ofFloatColor(0, 0, 0, 0));
            mesh.addVertex(ofPoint( 0.0f, getWindowHeight() - blendB*getWindowHeight(), 0.0f ));
            mesh.addColor(ofFloatColor(0, 0, 0, 0));
            mesh.addVertex(ofPoint( getWindowWidth(), getWindowHeight() - blendB*getWindowHeight(), 0.0f ));
            mesh.draw();
            mesh.clear();
        }
        if(blendL>0){
            mesh.addColor(ofFloatColor(0, 0, 0, 1));
            mesh.addVertex(ofPoint( 0.0f, 0.0f, 0.0f ));
            mesh.addColor(ofFloatColor(0, 0, 0, 1));
            mesh.addVertex(ofPoint( 0.0f,  getWindowHeight(), 0.0f ));
            mesh.addColor(ofFloatColor(0, 0, 0, 0));
            mesh.addVertex(ofPoint( blendL*getWindowHeight(), 0.0f, 0.0f ));
            mesh.addColor(ofFloatColor(0, 0, 0, 0));
            mesh.addVertex(ofPoint( blendL*getWindowHeight(),  getWindowHeight(), 0.0f ));
            mesh.draw();
            mesh.clear();
        }
        if(blendR>0){
            mesh.addColor(ofFloatColor(0, 0, 0, 1));
            mesh.addVertex(ofPoint( getWindowWidth(), 0.0f, 0.0f ));
            mesh.addColor(ofFloatColor(0, 0, 0, 1));
            mesh.addVertex(ofPoint( getWindowWidth(), getWindowHeight(), 0.0f ));
            mesh.addColor(ofFloatColor(0, 0, 0, 0));
            mesh.addVertex(ofPoint( getWindowWidth() - blendR*getWindowHeight(), 0.0f, 0.0f ));
            mesh.addColor(ofFloatColor(0, 0, 0, 0));
            mesh.addVertex(ofPoint( getWindowWidth() - blendR*getWindowHeight(), getWindowHeight(), 0.0f ));
            mesh.draw();
            mesh.clear();
        }
	
	perspective.end();
    
    if(scissorEnabled){
        glDisable(GL_SCISSOR_TEST);
    }
}
void Controller::drawGui(){
	drawn = true; // for the enable/disable magic
	
	ofPushStyle();
	ofSetLineWidth(guiLineThickness);
    
	perspective.begin();
	ofPushStyle();
	ofNoFill();
	ofSetColor(255);
	if(blendT>0) ofRect(0,0,getWindowWidth(), blendT*getWindowHeight());
	if(blendB>0) ofRect(0,getWindowHeight() - blendB*getWindowHeight(),getWindowWidth(), blendB*getWindowHeight());
	if(blendL>0) ofRect(0,0,blendL*getWindowHeight(), getWindowHeight());
	if(blendR>0) ofRect(getWindowWidth() - blendR*getWindowHeight() ,0,blendR*getWindowHeight(), getWindowHeight());
	ofPopStyle();
    perspective.end();
	
	if(perspective.isActive()){
        perspective.begin();
		ofPushStyle();
		ofNoFill();
		ofSetColor(255);
		ofRect(0, 0, getWindowWidth(), getWindowHeight());
		ofPopStyle();
        perspective.end();
        
        ofPushStyle();
        ofSetColor(255,255,0);
    
		ofCircle(perspective.fromWarpToScreenCoord(0, 0, 0), 9);
		ofCircle(perspective.fromWarpToScreenCoord(0, getWindowHeight(), 0), 9);
		ofCircle(perspective.fromWarpToScreenCoord(getWindowWidth(), 0, 0), 9);
		ofCircle(perspective.fromWarpToScreenCoord(getWindowWidth(), getWindowHeight(), 0), 9);

		ofPopStyle();
	}
	else{
        perspective.begin();
		internalMesh.drawWireframe();
        perspective.end();
        
        for (int i = 0; i < controlMesh.getVertices().size(); i++) {
            ofPushStyle();
            ofSetColor(controlMesh.getColor(i));
            ofPoint vertex = controlMesh.getVertex(i);
            ofCircle(perspective.fromWarpToScreenCoord(vertex.x,vertex.y,vertex.z), 9);
            ofPopStyle();
        }
        
		
	}
	
	
	
	guiHelperFbo.begin();
	ofClear(0, 0, 0, 200);
	
	ofPushStyle();
	ofSetColor(255, 255, 255);
	ofVec4f transformed = perspective.fromScreenToWarpCoord(mouse.x, mouse.y, 0);
    ofVec2f texSize(coordinatesEnd.x - coordinatesStart.x, coordinatesEnd.y - coordinatesStart.y);
    ofVec4f texCoords;
    texCoords.x = ofMap(transformed.x, 0, getWindowWidth(), 0, texSize.x, false) + coordinatesStart.x;
    texCoords.y = ofMap(transformed.y, 0, getWindowHeight(), 0, texSize.y, true) + coordinatesStart.y;
    string texX = ofToString(texCoords.x);
    string texY = ofToString(texCoords.y);
	ofDrawBitmapString("tex:   x " + texX+"\n       y "+texY, ofPoint(5, guiHelperFbo.getHeight()/2 - 15));
	ofDrawBitmapString("mouse: x " + ofToString((int)mouse.x)+"\n       y "+ofToString((int)mouse.y), ofPoint(5, guiHelperFbo.getHeight() - 20));
	ofPopStyle();
	guiHelperFbo.end();
	
    float helperScale = 1.5;
	ofPushMatrix();
	ofTranslate(mouse.x, mouse.y);
	if (perspective.getCorner(ofxGLWarper::BOTTOM_RIGHT).x - mouse.x < guiHelperFbo.getWidth()*helperScale) {
		ofTranslate(-guiHelperFbo.getWidth()*helperScale, 0);
	}
	if (perspective.getCorner(ofxGLWarper::BOTTOM_RIGHT).y - mouse.y < guiHelperFbo.getHeight()*helperScale) {
		ofTranslate(0, -guiHelperFbo.getHeight()*helperScale);
	}
	ofScale(helperScale, helperScale);
	guiHelperFbo.draw(0, 0, guiHelperFbo.getTextureReference().getWidth(), guiHelperFbo.getTextureReference().getHeight());
	ofPopMatrix();
	
	ofPushStyle();
	ofSetColor(255, 255, 255);
	ofLine(mouse.x, 0, mouse.x, ofGetHeight());
	ofLine(0, mouse.y, ofGetWidth(), mouse.y);
	ofPopStyle();
	
	ofPopStyle();
	gui.draw();

}

float Controller::getWindowWidth(){
    return originalSize.x;
}
float Controller::getWindowHeight(){
    return originalSize.y;
}

void Controller::setScissor(ofRectangle scissor, bool updateGui){
    this->scissor = scissor;
    if(updateGui){
        gui.getIntSlider("Scissor Start X") = scissor.x;
        gui.getIntSlider("Scissor Start Y") = scissor.y;
        
        gui.getIntSlider("Scissor End X") = scissor.x + scissor.width;
        gui.getIntSlider("Scissor End Y") = scissor.y + scissor.height;
        saveHistoryEntry();
    }
}
ofRectangle Controller::getScissor(){
    return scissor;
}
void Controller::enableScissor(bool updateGui){
    scissorEnabled = true;
    if(updateGui){
        gui.getToggle("Scissor Active") = scissorEnabled;
        saveHistoryEntry();
    }
}
void Controller::disableScissor(bool updateGui){
    scissorEnabled = false;
    if(updateGui){
        gui.getToggle("Scissor Active") = scissorEnabled;
        saveHistoryEntry();
    }
}
bool Controller::isScissorEnabled(){
    return scissorEnabled;
}

void Controller::setBrighteness(float value, bool updateGui){
    postBrt = value;
    if(updateGui){
        gui.getFloatSlider("Brighteness") = postBrt;
        saveHistoryEntry();
    }
}
void Controller::setContrast(float value, bool updateGui){
    postCon = value;
    if(updateGui){
        gui.getFloatSlider("Contrast") = value;
        saveHistoryEntry();
    }
}
void Controller::setSaturation(float value, bool updateGui){
    postSat = value;
    if(updateGui){
        gui.getFloatSlider("Saturation") = value;
        saveHistoryEntry();
    }
}
void Controller::setRedMultiplier(float value, bool updateGui){
    postRMult = value;
    if(updateGui){
        gui.getFloatSlider("Red Multiplier") = value;
        saveHistoryEntry();
    }
}
void Controller::setGreenMultipler(float value, bool updateGui){
    postGMult = value;
    if(updateGui){
        gui.getFloatSlider("Green Multipler") = value;
        saveHistoryEntry();
    }
}
void Controller::setBlueMultipler(float value, bool updateGui){
    postBMult = value;
    if(updateGui){
        gui.getFloatSlider("Blue Multipler") = value;
        saveHistoryEntry();
    }
}

void Controller::selectVertex(float mouseX, float mouseY){
    
    ofVec4f transformed = perspective.fromScreenToWarpCoord(mouseX, mouseY, 0);
    
    ofPoint input(transformed.x, transformed.y);
    bool emptySelection = true;
    for (int i = 0; i < controlQuads.size(); i++) {
		ControlQuad * c_quad = controlQuads[i];
		for (int ci = 0; ci < 4; ci++) {
			int index = c_quad->index + ci;
			SelectablePoint * c_vertex;
			if(ci == 0) c_vertex = &(c_quad->TL);
			else if(ci == 1) c_vertex = &(c_quad->BL);
			else if(ci == 2) c_vertex = &(c_quad->TR);
			else if(ci == 3) c_vertex = &(c_quad->BR);
			ofRectangle handle( c_vertex->x - 25, c_vertex->y - 25, 50, 50 );
			
			if(handle.inside(input)){
				c_vertex->selected = !c_vertex->selected;
				if(c_vertex->selected){
					emptySelection = false;
					controlMesh.setColor(index, ofFloatColor(1.0,0.0,1.0,1.0));
				}
				else{
					controlMesh.setColor(index, ofFloatColor(1.0,1.0,0.0,1.0));
				}
			}
		}
    }
	if(emptySelection){
		for (int i = 0; i < controlQuads.size(); i++) {
			ControlQuad * c_quad = controlQuads[i];
			for (int ci = 0; ci < 4; ci++) {
				int index = c_quad->index + ci;
				
				SelectablePoint * c_vertex;
				if(ci == 0) c_vertex = &(c_quad->TL);
				else if(ci == 1) c_vertex = &(c_quad->BL);
				else if(ci == 2) c_vertex = &(c_quad->TR);
				else if(ci == 3) c_vertex = &(c_quad->BR);
				ofRectangle handle( c_vertex->x - 25, c_vertex->y - 25, 50, 50 );
				
				c_vertex->selected = false;
				controlMesh.setColor(index, ofFloatColor(1.0,1.0,0.0,1.0));
			}
		}
	}
}
ofPoint Controller::getInteractionOffset(float mouseX, float mouseY){
	ofVec4f transformed = perspective.fromScreenToWarpCoord(mouseX, mouseY, 0);
    ofPoint input(transformed.x, transformed.y);
    for (int i = 0; i < controlQuads.size(); i++) {
		ControlQuad * c_quad = controlQuads[i];
		for (int ci = 0; ci < 4; ci++) {
			SelectablePoint * c_vertex;
			if(ci == 0) c_vertex = &(c_quad->TL);
			else if(ci == 1) c_vertex = &(c_quad->BL);
			else if(ci == 2) c_vertex = &(c_quad->TR);
			else if(ci == 3) c_vertex = &(c_quad->BR);
			if (c_vertex->selected) {
				ofRectangle handle( c_vertex->x - 40, c_vertex->y - 40, 80, 80 );
				
				if(handle.inside(input)){
					ofPoint offset = input - *c_vertex;
					return offset ;
					break;
				}
			}
		}
    }
	
	return ofPoint(0);
}
void Controller::updateVertices(){
	//upadate the control quads
	for (int c_i = 0; c_i < controlQuads.size(); c_i++) {
		ControlQuad * c_quad = controlQuads[c_i];
		bool needsUpdate = false;
		for (int c_pi = 0; c_pi < 4; c_pi++) {
			int index = c_quad->index + c_pi;
			SelectablePoint * c_vertex;
			if(c_pi == 0) c_vertex = &(c_quad->TL);
			else if(c_pi == 1) c_vertex = &(c_quad->BL);
			else if(c_pi == 2) c_vertex = &(c_quad->TR);
			else if(c_pi == 3) c_vertex = &(c_quad->BR);
			if (c_vertex->selected) {
				*c_vertex += interactionOffset;
				controlMesh.setVertex(index, *c_vertex);
				needsUpdate = true;
			}
		}
		if(needsUpdate){
			// update the internal quads
			for (int i_i = 0; i_i < c_quad->internalQuads.size(); i_i++) {
				InternalQuad * i_quad = c_quad->internalQuads[i_i];
				
				// interpolate the edges
				vector<ofPoint> left, right;
				left.resize(resolution.y+1);
				right.resize(resolution.y+1);
				
				for (int y = 0; y <= resolution.y; y++) {
					left[y] =	c_quad->TL + ((c_quad->BL - c_quad->TL) / resolution.y) * (float)y;
					right[y] =	c_quad->TR + ((c_quad->BR - c_quad->TR) / resolution.y) * (float)y;
				}
				// interpolate all internal points
				vector<vector<SelectablePoint> >grid;
				grid.resize(resolution.x+1);
				for (int x = 0; x <= resolution.x; x++) {
					grid[x].resize(resolution.x+1);
					for (int y = 0; y <= resolution.y; y++) {
						ofPoint l = left[y];
						ofPoint r = right[y];
						
						grid[x][y].set(l + ((r - l) / resolution.x) * (float)x);
					}
				}
				
				// update the internal quads
				i_quad->TL = grid[i_quad->x][i_quad->y];
				i_quad->BL = grid[i_quad->x][i_quad->y+1];
				i_quad->TR = grid[i_quad->x+1][i_quad->y];
				i_quad->BR = grid[i_quad->x+1][i_quad->y+1];
				
				// update the mesh				
				internalMesh.setVertex(i_quad->index + 0, i_quad->TL);
				internalMesh.setVertex(i_quad->index + 1, i_quad->BL);
				internalMesh.setVertex(i_quad->index + 2, i_quad->TR);
				
				internalMesh.setVertex(i_quad->index + 3, i_quad->TR);
				internalMesh.setVertex(i_quad->index + 4, i_quad->BL);
				internalMesh.setVertex(i_quad->index + 5, i_quad->BR);
			}
		}
    }
	
}

void Controller::resetVertices(bool saveInHistory){
    // trick to reset vertices is to fake something has changed and call onGridChange
    gridSize.x = -1;
    int dummy = 0;
    onGridChange(dummy);
    
    //unset the gui change flag
    guiHasChanged = false;
    
    if(saveInHistory)saveHistoryEntry();
}
void Controller::saveVertices(float * handler){
	int i = 0;
	for (int c_i = 0; c_i <controlQuads.size(); c_i++) {
		ControlQuad * c_quad = controlQuads[c_i];
		handler[i++] = c_quad->index;
		handler[i++] = c_quad->TL.x;
		handler[i++] = c_quad->TL.y;
		handler[i++] = c_quad->BL.x;
		handler[i++] = c_quad->BL.y;
		handler[i++] = c_quad->TR.x;
		handler[i++] = c_quad->TR.y;
		handler[i++] = c_quad->BR.x;
		handler[i++] = c_quad->BR.y;
		for (int i_i = 0; i_i < c_quad->internalQuads.size(); i_i++) {
			InternalQuad * i_quad = c_quad->internalQuads[i_i];
			handler[i++] = i_quad->index;
			handler[i++] = i_quad->x;
			handler[i++] = i_quad->y;
			handler[i++] = i_quad->TL.x;
			handler[i++] = i_quad->TL.y;
			handler[i++] = i_quad->BL.x;
			handler[i++] = i_quad->BL.y;
			handler[i++] = i_quad->TR.x;
			handler[i++] = i_quad->TR.y;
			handler[i++] = i_quad->BR.x;
			handler[i++] = i_quad->BR.y;
		}
	}
}
void Controller::loadVertices(float * handler){
	int i = 0;
	for (int c_i = 0; c_i <controlQuads.size(); c_i++) {
		ControlQuad * c_quad = controlQuads[c_i];
		c_quad->index = handler[i++];
		c_quad->TL.x = handler[i++];
		c_quad->TL.y = handler[i++];
		c_quad->BL.x = handler[i++];
		c_quad->BL.y = handler[i++];
		c_quad->TR.x = handler[i++];
		c_quad->TR.y = handler[i++];
		c_quad->BR.x = handler[i++];
		c_quad->BR.y = handler[i++];
		
		controlMesh.setVertex(c_quad->index + 0, c_quad->TL);
		controlMesh.setVertex(c_quad->index + 1, c_quad->BL);
		controlMesh.setVertex(c_quad->index + 2, c_quad->TR);
		controlMesh.setVertex(c_quad->index + 3, c_quad->BR);
		
		for (int i_i = 0; i_i < c_quad->internalQuads.size(); i_i++) {
			InternalQuad * i_quad = c_quad->internalQuads[i_i];
			i_quad->index = handler[i++];
			i_quad->x = handler[i++];
			i_quad->y = handler[i++];
			i_quad->TL.x = handler[i++];
			i_quad->TL.y = handler[i++];
			i_quad->BL.x = handler[i++];
			i_quad->BL.y = handler[i++];
			i_quad->TR.x = handler[i++];
			i_quad->TR.y = handler[i++];
			i_quad->BR.x = handler[i++];
			i_quad->BR.y = handler[i++];
			
			internalMesh.setVertex(i_quad->index + 0, i_quad->TL);
			internalMesh.setVertex(i_quad->index + 1, i_quad->BL);
			internalMesh.setVertex(i_quad->index + 2, i_quad->TR);
			
			internalMesh.setVertex(i_quad->index + 3, i_quad->TR);
			internalMesh.setVertex(i_quad->index + 4, i_quad->BL);
			internalMesh.setVertex(i_quad->index + 5, i_quad->BR);
		}
	}
}

void Controller::resetPerspective(bool saveInHistory){
    perspective.setCorner(ofxGLWarper::TOP_LEFT, originalPerspective[0]);
    perspective.setCorner(ofxGLWarper::TOP_RIGHT, originalPerspective[1]);
    perspective.setCorner(ofxGLWarper::BOTTOM_RIGHT, originalPerspective[2]);
    perspective.setCorner(ofxGLWarper::BOTTOM_LEFT, originalPerspective[3]);
    
    if(saveInHistory)saveHistoryEntry();
}
void Controller::savePerspective(ofxXmlSettings & handler){
	perspective.saveToXml(handler);
}
void Controller::loadPerspective(ofxXmlSettings & handler){
	perspective.loadFromXml(handler);
}

void Controller::saveGUI(ofxXmlSettings & handler){
	gui.saveTo(handler);
}
void Controller::loadGUI(ofxXmlSettings & handler){
	gui.loadFrom(handler);
    bool dummyb=0;
	int dummyi=0;
	float dummyf=0;
	onGridChange(dummyi); // will also update the coordinates;
	onEnablePerspective((bool&)gui.getToggle("Perspective Warp"));
	onBlendChange(dummyf);
    onScissorEnabled((bool&)gui.getToggle("Scissor Active"));
    onScissorChange(dummyi);
	onPostProcessingValueChanged(dummyf);
}

void Controller::saveHistoryEntry(){
	// generate the entry
	HistoryEntry* entry = new HistoryEntry();
	// allocate vertices data
	const int numControlQuads = controlQuads.size();
	const int controlQuadSize = 8 + 1;  // points + index
	const int numInternalQuads = (resolution.x) * (resolution.y);
	const int internalQuadSize = 8 + 2 + 1; // points + xy + index
	const int dataSize = numControlQuads * controlQuadSize + (numControlQuads * numInternalQuads) * internalQuadSize;
	entry->verticesData = new float[dataSize];
	
	// push data to the entry
	saveGUI(entry->guiData);
	savePerspective(entry->perspectiveData);
	saveVertices(entry->verticesData);
	
	// delete any "newer" history
	while (history.size() > historyIndex+1) {
		delete history[history.size()-1];
		history.pop_back();
	}
	// pushback and increase index
	history.push_back(entry);
	historyIndex++;
		
	// make sure to delete too old history
	while (history.size() > OFX_WARP_BLEND_TOOL_MAX_HISTORY) {
		delete history[0];
		history.erase(history.begin());
	}
}
void Controller::loadHistoryEntry(int index){
	if(!history.size()) return;
    if(index < 0) index = 0;
	else if(index > history.size()-1) index = history.size()-1;
    if(historyIndex == index) return;
	historyIndex = index;
	
	HistoryEntry* entry = history[index];
	loadGUI(entry->guiData);
	loadPerspective(entry->perspectiveData);
	loadVertices(entry->verticesData);
}

void Controller::onSave(){
    ofDirectory::createDirectory(safename, true, true);
        
    ofxXmlSettings perspectiveSettings;
    savePerspective(perspectiveSettings);
    perspectiveSettings.saveFile(perspectiveFile);
		
    ofxXmlSettings guiSettings;
    saveGUI(guiSettings);
    guiSettings.saveFile(guiFile);
		
    const int numControlQuads = controlQuads.size();
    const int controlQuadSize = 8 + 1;  // points + index
    const int numInternalQuads = (resolution.x) * (resolution.y);
    const int internalQuadSize = 8 + 2 + 1; // points + xy + index
    const int dataSize = numControlQuads * controlQuadSize + (numControlQuads * numInternalQuads) * internalQuadSize;
    float * data = new float[dataSize];
    saveVertices(data);
    ofFile writeFile;
    if(writeFile.open(meshFile, ofFile::WriteOnly, true)){
        writeFile.write((char*) data, sizeof(float) * dataSize );
    }
    delete data;
}
void Controller::onLoad(){
    // Check if there is no saved file, if not reset
    ofxXmlSettings perspectiveSettings;
    if(perspectiveSettings.loadFile(perspectiveFile)){
        loadPerspective(perspectiveSettings);
    }
    else{
        resetPerspective(false); // false means we wont generate a history entry
    }
		
    ofxXmlSettings guiSettings;
    guiSettings.loadFile(guiFile);
    loadGUI(guiSettings);
		
		
    fstream readFile;
    readFile.open(ofToDataPath(meshFile).c_str(), ios::in | ios::binary);
    if(readFile.good()){
			
        const int numControlQuads = controlQuads.size();
        const int controlQuadSize = 8 + 1;  // points + index
        const int numInternalQuads = (resolution.x) * (resolution.y);
        const int internalQuadSize = 8 + 2 + 1; // points + xy + index
        const int dataSize = numControlQuads * controlQuadSize + (numControlQuads * numInternalQuads) * internalQuadSize;
        float * data = new float[dataSize];
        readFile.read((char*)data, sizeof(float) * dataSize);
        loadVertices(data);
        delete data;
			
    }else{
        ofLogWarning("ofxWarpBlendTool::Controller:: unable to load mesh file: "+meshFile);
    }
		
    // Save initial history entry
    saveHistoryEntry();
}
void Controller::onResetPerspective(){
	resetPerspective();
}
void Controller::onResetMesh(){
	resetVertices();
}
void Controller::onBlendChange(float & value){
	if(blendT == gui.getFloatSlider("Blend Top")
	   && blendB == gui.getFloatSlider("Blend Down")
	   && blendL == gui.getFloatSlider("Blend Left")
	   && blendR == gui.getFloatSlider("Blend Right")){
		return;
	}
	guiHasChanged = true; // flag that gui has changed
	
	blendT = gui.getFloatSlider("Blend Top");
	blendB = gui.getFloatSlider("Blend Down");
	blendL = gui.getFloatSlider("Blend Left");
	blendR = gui.getFloatSlider("Blend Right");
}
void Controller::onEnablePerspective(bool & value){
	guiHasChanged = true; // flag that gui changed
    if(value){
        perspective.activate();
    }
    else{
        perspective.deactivate();
    }
}
void Controller::onGridChange(int & value){
	if(resolution.x == gui.getIntSlider("Horizontal Resolution")
	   && resolution.y == gui.getIntSlider("Vertical Resolution")
	   && gridSize.x == gui.getIntSlider("Grid Columns")
	   && gridSize.y == gui.getIntSlider("Grid Rows")){
		return;
	}
	guiHasChanged = true; // flag that gui has changed
	
	
    resolution.x = gui.getIntSlider("Horizontal Resolution");
	resolution.y = gui.getIntSlider("Vertical Resolution");
    gridSize.x = gui.getIntSlider("Grid Columns");
    gridSize.y = gui.getIntSlider("Grid Rows");
    
    // Generate the quads
	while (controlQuads.size()) {
		while (controlQuads[0]->internalQuads.size()) {
			delete controlQuads[0]->internalQuads[0];
			controlQuads[0]->internalQuads.erase(controlQuads[0]->internalQuads.begin());
		}
		delete controlQuads[0];
		controlQuads.erase(controlQuads.begin());
	}
    controlMesh.clearVertices();
    internalMesh.clearVertices();
    internalMesh.clearColors();
    
    ofVec2f c_Size(getWindowWidth() / gridSize.x, getWindowHeight() / gridSize.y);
    ofVec2f i_Size = c_Size / resolution;
    int c_index = 0;
    int i_index = 0;
	
    for (int y = 0; y < (int)gridSize.y; y++) {
		ofVec2f c_RowStart =  y * ofVec2f(0, c_Size.y);
        
        for (int x = 0; x < (int)gridSize.x; x++) {
            ofVec2f c_Start = c_RowStart + x * ofVec2f(c_Size.x, 0);
            
            ControlQuad* c_quad = new ControlQuad();
            c_quad->index = c_index;
            c_quad->TL.set(c_Start.x, c_Start.y, 0);
            c_quad->BL.set(c_Start.x, c_Start.y + c_Size.y, 0);
            c_quad->TR.set(c_Start.x + c_Size.x, c_Start.y, 0);
            c_quad->BR.set(c_Start.x + c_Size.x, c_Start.y + c_Size.y, 0);
            controlQuads.push_back(c_quad);
            
            controlMesh.addVertex(c_quad->TL);
            controlMesh.addVertex(c_quad->BL);
            controlMesh.addVertex(c_quad->TR);
            controlMesh.addVertex(c_quad->BR);
            
            controlMesh.addColor(ofFloatColor(1.0,1.0,0.0,1.0));
            controlMesh.addColor(ofFloatColor(1.0,1.0,0.0,1.0));
            controlMesh.addColor(ofFloatColor(1.0,1.0,0.0,1.0));
            controlMesh.addColor(ofFloatColor(1.0,1.0,0.0,1.0));
            
            c_index+=4;
            
			
            for (int i_y = 0; i_y < resolution.y; i_y++) {
                ofVec2f i_RowStart = c_Start + i_y * ofVec2f(0, i_Size.y);
                
                for (int i_x = 0; i_x < resolution.x; i_x++) {
                    ofVec2f i_Start = i_RowStart + i_x * ofVec2f(i_Size.x, 0);
                    
                    InternalQuad* i_quad = new InternalQuad();
                    i_quad->index = i_index;
					i_quad->x = i_x;
					i_quad->y = i_y;
                    i_quad->TL.set(i_Start.x, i_Start.y);
                    i_quad->BL.set(i_Start.x, i_Start.y + i_Size.y);
                    i_quad->TR.set(i_Start.x + i_Size.x, i_Start.y);
                    i_quad->BR.set(i_Start.x + i_Size.x, i_Start.y + i_Size.y);
					
					c_quad->internalQuads.push_back(i_quad);
                    
                    internalMesh.addVertex(i_quad->TL);
                    internalMesh.addVertex(i_quad->BL);
                    internalMesh.addVertex(i_quad->TR);
                    
                    internalMesh.addVertex(i_quad->TR);
                    internalMesh.addVertex(i_quad->BL);
                    internalMesh.addVertex(i_quad->BR);
					
                    internalMesh.addColor(ofFloatColor(1.0,1.0,1.0,1.0));
                    internalMesh.addColor(ofFloatColor(1.0,1.0,1.0,1.0));
                    internalMesh.addColor(ofFloatColor(1.0,1.0,1.0,1.0));
                    internalMesh.addColor(ofFloatColor(1.0,1.0,1.0,1.0));
					internalMesh.addColor(ofFloatColor(1.0,1.0,1.0,1.0));
                    internalMesh.addColor(ofFloatColor(1.0,1.0,1.0,1.0));
                    
                    i_index+=6;
                }
            }
        }
    }
    
    // Generate the coordinates;
    float dummy = 0;
	onCoordinatesChange(dummy);
}
void Controller::onCoordinatesChange(float & value){
	
	guiHasChanged = true; // flag that gui has changed
	
	coordinatesStart.x =  gui.getFloatSlider("UV Start X");
	coordinatesStart.y =  gui.getFloatSlider("UV Start Y");
	coordinatesEnd.x =  gui.getFloatSlider("UV End X");
	coordinatesEnd.y =  gui.getFloatSlider("UV End Y");
	
	internalMesh.clearTexCoords();
	
    ofVec2f uvSize = coordinatesEnd - coordinatesStart;
    ofVec2f uvCellSize = uvSize / gridSize;
    ofVec2f i_Size = uvCellSize / resolution;
    for (int y = 0; y < gridSize.y; y++) {
        ofVec2f uvRowStart = coordinatesStart + y * ofVec2f(0, uvCellSize.y);
        
        for (int x = 0; x < gridSize.x; x++) {
            ofVec2f uvCellStart = uvRowStart + x * ofVec2f(uvCellSize.x, 0);
            
            
            for (int i_y = 0; i_y < resolution.y; i_y++) {
                ofVec2f i_RowStart = uvCellStart + i_y * ofVec2f(0, i_Size.y);
                
                for (int i_x = 0; i_x < resolution.x; i_x++) {
                    ofVec2f i_Start = i_RowStart + i_x * ofVec2f(i_Size.x, 0);
                    
					ofVec2f TL = ofVec2f(i_Start.x, i_Start.y);
					ofVec2f BL = ofVec2f(i_Start.x, i_Start.y + i_Size.y);
					ofVec2f TR = ofVec2f(i_Start.x + i_Size.x, i_Start.y);
					ofVec2f BR = ofVec2f(i_Start.x + i_Size.x, i_Start.y + i_Size.y);
					
					internalMesh.addTexCoord(TL);
					internalMesh.addTexCoord(BL);
					internalMesh.addTexCoord(TR);
					
					internalMesh.addTexCoord(TR);
					internalMesh.addTexCoord(BL);
					internalMesh.addTexCoord(BR);
                }
            }
        }
    }
}
void Controller::onScissorEnabled(bool & value){
    guiHasChanged = true; // flag that gui has changed
    if(value){
        enableScissor(false);
    }
    else disableScissor(false);
}
void Controller::onScissorChange(int & value){
    guiHasChanged = true; // flag that gui has changed
    setScissor(ofRectangle(ofPoint(gui.getIntSlider("Scissor Start X"),gui.getIntSlider("Scissor Start Y")),
               ofPoint(gui.getIntSlider("Scissor End X"),gui.getIntSlider("Scissor End Y"))), false);
}
void Controller::onPostProcessingValueChanged(float & value){
	guiHasChanged = true; // flag that gui has changed
	if(postBrt != gui.getFloatSlider("Brighteness")) setBrighteness(gui.getFloatSlider("Brighteness"), false);
	if(postSat != gui.getFloatSlider("Saturation")) setSaturation(gui.getFloatSlider("Saturation"), false);
	if(postCon != gui.getFloatSlider("Contrast")) setContrast(gui.getFloatSlider("Contrast"), false);
	if(postRMult != gui.getFloatSlider("Red Multiplier")) setRedMultiplier(gui.getFloatSlider("Red Multiplier"), false);
	if(postGMult != gui.getFloatSlider("Green Multiplier")) setGreenMultipler(gui.getFloatSlider("Green Multiplier"), false);
	if(postBMult != gui.getFloatSlider("Blue Multipler")) setBlueMultipler(gui.getFloatSlider("Blue Multipler"), false);
}
void Controller::onGuiLinesThicknessChange(int &value){
    guiHasChanged = true; // flag that gui has changed
    guiLineThickness = value;
}
void Controller::onPerspectiveChange(ofxGLWarper::CornerLocation & cornerLocation){
    perspectiveHasChanged = true;
}

void Controller::keyPressed(ofKeyEventArgs & args){
    // UNDO/REDO
    bool undo = false;
    bool redo = false;
#ifdef TARGET_OSX
    if(args.key =='z' || args.key =='Z'){
        if (ofGetModifierPressed(OF_KEY_SPECIAL)){
            if(ofGetModifierPressed(OF_KEY_SHIFT)){
                redo = true;
            }
            else{
                undo = true;
            }
        }
    }
#endif
#ifdef TARGET_LINUX
    if(args.key =='z' || args.key =='Z'){
        if (ofGetModifierPressed(OF_KEY_CONTROL)){
            if(ofGetModifierPressed(OF_KEY_SHIFT)){
                redo = true;
            }
            else{
                undo = true;
            }
        }
    }
#endif
#ifdef TARGET_WIN32
    if(args.key =='z' || args.key =='Z'){
        if (ofGetModifierPressed(OF_KEY_CONTROL)){
            if(ofGetModifierPressed(OF_KEY_SHIFT)){
                redo = true;
            }
            else{
                undo = true;
            }
        }
    }
    if(args.key =='y' || args.key =='Y'){
        if (ofGetModifierPressed(OF_KEY_CONTROL)){
            redo = true;   
        }
    }
#endif
    if(undo) loadHistoryEntry(historyIndex - 1);
    if(redo) loadHistoryEntry(historyIndex + 1);
    
    // SAVE/LOAD
    bool load = false;
    bool save = false;
    
#ifdef TARGET_OSX
    if(args.key =='r' || args.key =='R'){
        if (ofGetModifierPressed(OF_KEY_SPECIAL)){
            load = true;
        }
    }
    if(args.key =='s' || args.key =='R'){
        if (ofGetModifierPressed(OF_KEY_SPECIAL)){
            save = true;
        }
    }
#else
    if(args.key =='r' || args.key =='S'){
        if (ofGetModifierPressed(OF_KEY_CONTROL)){
            load = true;
        }
    }
    if(args.key =='s' || args.key =='S'){
        if (ofGetModifierPressed(OF_KEY_CONTROL)){
            save = true;
        }
    }
#endif
    if(load) onLoad();
    if(save) onSave();
    
}
void Controller::keyReleased(ofKeyEventArgs & args){
}
void Controller::mouseMoved(ofMouseEventArgs & args){
	mouse.set(args.x,args.y);
}
void Controller::mouseDragged(ofMouseEventArgs & args){
	mouse.set(args.x,args.y);
	if(!perspective.isActive()){
		interactionOffset = getInteractionOffset(args.x, args.y);
		updateVertices();
	}
}
void Controller::mousePressed(ofMouseEventArgs & args){
	if(!perspective.isActive()){
		tempInteractionOffset = interactionOffset;
		// double click detected
		if ((ofGetElapsedTimeMillis() - lastClickTime) < 300) {
			selectVertex(args.x, args.y);
		}
	}
	lastClickTime = ofGetElapsedTimeMillis();
}
void Controller::mouseReleased(ofMouseEventArgs & args){
	// detect changes in gui
	if(guiHasChanged) saveHistoryEntry();
    guiHasChanged = false;
    
    // detect change in perspective
    if(perspectiveHasChanged) saveHistoryEntry();
    perspectiveHasChanged = false;
	
	if(!perspective.isActive()){
		// if detect change in vertices, saveHistory
		if(tempInteractionOffset != interactionOffset){
			saveHistoryEntry();
		}
	}
    
    
}

/*
 this part controls whether events get through to the object or not. if the
 object is not drawn using drawGUI(), we know here because drawGUI() never set the
 drawn flag. in that case, we unregister events. if it changes from being off
 to on, then we register the events again.
 */
void Controller::drawEvent(ofEventArgs& args) {
	bool prevDrawing = drawing;
	drawing = drawn;
	if(drawing != prevDrawing) {
		if(drawing) {
			ofRegisterMouseEvents(this);
			ofRegisterKeyEvents(this);
            
            if(gui.getToggle("Perspective Warp")) perspective.activate();
		} else {
			ofUnregisterMouseEvents(this);
			ofUnregisterKeyEvents(this);
            perspective.deactivate();
		}
	}
	drawn = false; // turn the drawn flag off, for draw() to turn back on
}
