#include "ofxWarpBlendTool.h"
using namespace ofxWarpBlendTool;

//--------------------------------------------------------------
void Controller::setup(ofTexture * texture, string name, float guiWidth, ofPoint initialOffset){
	// Register draw event for the enable/disable magic
	ofAddListener(ofEvents().draw, this, &Controller::drawEvent);
	drawing = drawn = false;
	
	// Some intial setup
	this->texture = texture;
	this->name = name;
	this->initialOffset = initialOffset;
	guiHelperFbo.allocate(150, 30);
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
	
	// Perpective
    perspective.setup(0, 0, texture->getWidth(), texture->getHeight());
    perspective.setCornerSensibility(0.02);
    perspective.deactivate();
    ofAddListener(perspective.changeEvent, this, &Controller::onPerspectiveChange);
    
	// Meshes
    controlMesh.setMode(OF_PRIMITIVE_POINTS);
	internalMesh.setMode(OF_PRIMITIVE_TRIANGLES);
	
	// GUI
    gui.setup(name, guiFile);
	gui.setPosition(initialOffset);
	
	ofxButton * load = new ofxButton();
    load->setup("Load", guiWidth);
    load->addListener(this, &Controller::onLoad);
    gui.add(load);
	
	ofxButton * save = new ofxButton();
    save->setup("Save", guiWidth);
    save->addListener(this, &Controller::onSave);
    gui.add(save);
    
    ofxToggle * enablePerspective = new ofxToggle();
    enablePerspective->setup("Perspective Warp", true, guiWidth);
    enablePerspective->addListener(this, &Controller::onEnablePerpective);
    gui.add(enablePerspective);
    
    ofxIntSlider * resolutionX = new ofxIntSlider();
    resolutionX->setup("Horizontal Resolution", 4, 1, 20, guiWidth);
    resolutionX->addListener(this, &Controller::onGridChange);
    gui.add(resolutionX);
	
	ofxIntSlider * resolutionY = new ofxIntSlider();
    resolutionY->setup("Vertical Resolution", 4, 1, 20, guiWidth);
    resolutionY->addListener(this, &Controller::onGridChange);
    gui.add(resolutionY);
    
    ofxIntSlider * columns = new ofxIntSlider();
    columns->setup("Grid Columns", 4, 1, 20, guiWidth);
    columns->addListener(this, &Controller::onGridChange);
    gui.add(columns);
    
    ofxIntSlider * rows = new ofxIntSlider();
    rows->setup("Grid Rows", 4, 1, 20, guiWidth);
    rows->addListener(this, &Controller::onGridChange);
    gui.add(rows);
    
    ofxFloatSlider * startX = new ofxFloatSlider();
    startX->setup("UV Start X", 0, 0, texture->getWidth(), guiWidth);
    startX->addListener(this, &Controller::onCoordinatesChange);
    gui.add(startX);
    
    ofxFloatSlider * startY = new ofxFloatSlider();
    startY->setup("UV Start Y", 0, 0, texture->getHeight(), guiWidth);
    startY->addListener(this, &Controller::onCoordinatesChange);
    gui.add(startY);
    
    ofxFloatSlider * endX = new ofxFloatSlider();
    endX->setup("UV End X", texture->getWidth(), 0, texture->getWidth(), guiWidth);
    endX->addListener(this, &Controller::onCoordinatesChange);
    gui.add(endX);
    
    ofxFloatSlider * endY = new ofxFloatSlider();
    endY->setup("UV End Y", texture->getHeight(), 0, texture->getHeight(), guiWidth);
    endY->addListener(this, &Controller::onCoordinatesChange);
    gui.add(endY);
	
	ofxFloatSlider * blendTSlider = new ofxFloatSlider();
    blendTSlider->setup("Blend Top", 0, 0, texture->getHeight(), guiWidth);
    blendTSlider->addListener(this, &Controller::onBlendChange);
    gui.add(blendTSlider);
	
	ofxFloatSlider * blendDSlider = new ofxFloatSlider();
    blendDSlider->setup("Blend Down", 0, 0, texture->getHeight(), guiWidth);
    blendDSlider->addListener(this, &Controller::onBlendChange);
    gui.add(blendDSlider);
	
	ofxFloatSlider * blendLSlider = new ofxFloatSlider();
    blendLSlider->setup("Blend Left", 0, 0, texture->getWidth(), guiWidth);
    blendLSlider->addListener(this, &Controller::onBlendChange);
    gui.add(blendLSlider);
	
	ofxFloatSlider * blendRSlider = new ofxFloatSlider();
    blendRSlider->setup("Blend Right", 0, 0, texture->getWidth(), guiWidth);
    blendRSlider->addListener(this, &Controller::onBlendChange);
    gui.add(blendRSlider);
	
	ofxIntSlider * lineThickness = new ofxIntSlider();
    lineThickness->setup("GUI Lines Thickness", 1, 1, 10, guiWidth);
    lineThickness->addListener(this, &Controller::onGuiLinesThicknessChange);
    gui.add(lineThickness);
    
	
	// load settings
	bool dummy = true;
	onLoad(dummy);
}
void Controller::draw(){
	perspective.begin();
	texture->bind();
	internalMesh.draw();
	texture->unbind();
	
	if(blendT>0){
		glBegin(GL_TRIANGLE_STRIP);
		glColor4f(0, 0, 0, 1);
		glVertex3f( 0.0f, 0.0f, 0.0f );
		glVertex3f( texture->getWidth(), 0.0f, 0.0f );
		glColor4f(0, 0, 0, 0);
		glVertex3f( 0.0f, blendT, 0.0f );		
		glVertex3f( texture->getWidth(), blendT, 0.0f );
		glEnd();
	}
	if(blendB>0){
		glBegin(GL_TRIANGLE_STRIP);
		glColor4f(0, 0, 0, 1);
		glVertex3f( 0.0f, texture->getHeight(), 0.0f );
		glVertex3f( texture->getWidth(), texture->getHeight(), 0.0f );
		glColor4f(0, 0, 0, 0);
		glVertex3f( 0.0f, texture->getHeight() - blendB, 0.0f );
		glVertex3f( texture->getWidth(), texture->getHeight() - blendB, 0.0f );
		glEnd();
	}
	if(blendL>0){
		glBegin(GL_TRIANGLE_STRIP);
		glColor4f(0, 0, 0, 1);
		glVertex3f( 0.0f, 0.0f, 0.0f );
		glVertex3f( 0.0f,  texture->getHeight(), 0.0f );
		glColor4f(0, 0, 0, 0);
		glVertex3f( blendL, 0.0f, 0.0f );
		glVertex3f( blendL,  texture->getHeight(), 0.0f );
		glEnd();
	}
	if(blendR>0){
		glBegin(GL_TRIANGLE_STRIP);
		glColor4f(0, 0, 0, 1);
		glVertex3f( texture->getWidth(), 0.0f, 0.0f );
		glVertex3f( texture->getWidth(), texture->getHeight(), 0.0f );
		glColor4f(0, 0, 0, 0);
		glVertex3f( texture->getWidth() - blendR, 0.0f, 0.0f );
		glVertex3f( texture->getWidth() - blendR, texture->getHeight(), 0.0f );
		glEnd();
	}
	
	perspective.end();
}
void Controller::drawGui(){
	drawn = true; // for the enable/disable magic
	
	ofPushStyle();
	ofSetLineWidth(guiLineThickness);
    
	perspective.begin();
	ofPushStyle();
	ofNoFill();
	ofSetColor(255);
	if(blendT>0) ofRect(0,0,texture->getWidth(), blendT);
	if(blendB>0) ofRect(0,texture->getHeight() - blendB,texture->getWidth(), blendB);
	if(blendL>0) ofRect(0,0,blendL, texture->getHeight());
	if(blendR>0) ofRect(texture->getWidth() - blendR ,0,blendR, texture->getHeight());
	ofPopStyle();
	
	if(perspective.isActive()){
		ofPushStyle();
		ofNoFill();
		ofSetColor(255);
		ofRect(0, 0, texture->getWidth(), texture->getHeight());
		
		glPointSize(10);
		glBegin(GL_POINTS);
		glVertex3f(0, 0, 0);
		glVertex3f(0, texture->getHeight(), 0);
		glVertex3f(texture->getWidth(), 0, 0);
		glVertex3f(texture->getWidth(), texture->getHeight(), 0);
		glEnd();
		glPointSize(1);
		ofPopStyle();
	}
	else{
		glPointSize(2);
		internalMesh.drawVertices();
		glPointSize(10);
		controlMesh.drawVertices();
		glPointSize(2);
		
	}
	perspective.end();
	
	
	guiHelperFbo.begin();
	ofClear(0, 0, 0, 200);
	
	ofPushStyle();
	ofSetColor(255, 255, 255);
	ofVec4f transformed = perspective.fromScreenToWarpCoord(mouse.x, mouse.y, 0);
	ofDrawBitmapString("texture: " + ofToString((int)transformed.x)+","+ofToString((int)transformed.y), ofPoint(5, 5));
	ofDrawBitmapString("screen:  " + ofToString((int)mouse.x)+","+ofToString((int)mouse.y), ofPoint(5, 15));
	ofPopStyle();
	guiHelperFbo.end();
	
	ofPushMatrix();
	ofTranslate(mouse.x, mouse.y);
	if (ofGetWidth() - mouse.x < 300) {
		ofTranslate(-300, 0);
	}
	if (mouse.y < 60) {
		ofTranslate(0, 60);
	}
	ofScale(2.0, 2.0);
	guiHelperFbo.draw(0, 0, guiHelperFbo.getTextureReference().getWidth(), -guiHelperFbo.getTextureReference().getHeight());
	ofPopMatrix();
	
	ofPushStyle();
	ofSetColor(255, 255, 255);
	ofLine(mouse.x, 0, mouse.x, ofGetHeight());
	ofLine(0, mouse.y, ofGetWidth(), mouse.y);
	ofPopStyle();
	
	ofPopStyle();
	gui.draw();

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
					controlMesh.setColor(index, ofFloatColor(1.0,1.0,0.0,1.0));
				}
				else{
					controlMesh.setColor(index, ofFloatColor(1.0,1.0,1.0,1.0));
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
				controlMesh.setColor(index, ofFloatColor(1.0,1.0,1.0,1.0));
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
			int index = c_quad->index + ci;
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

void Controller::savePerspective(ofxXmlSettings & handler){
	perspective.saveToXml(handler);
}
void Controller::loadPerspective(ofxXmlSettings & handler){
	perspective.loadFromXml(handler);
}

void Controller::saveGUI(ofxXmlSettings & handler){
	gui.saveToXml(handler);
}
void Controller::loadGUI(ofxXmlSettings & handler){
	gui.loadFromXml(handler);
	int dummyi=0;
	float dummyf=0;
	onGridChange(dummyi); // will also update the coordinates;
	onEnablePerpective(gui.getToggle("Perspective Warp"));
	onBlendChange(dummyf);
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

void Controller::onSave(bool &value){
	if(value){
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
}
void Controller::onLoad(bool &value){
	if(value){
		// Check if there is no saved file, if not, apply initial offset
		ofxXmlSettings perspectiveSettings;
		if(perspectiveSettings.loadFile(perspectiveFile)){
			loadPerspective(perspectiveSettings);
		}
		else{
			perspectiveSettings.clear();
			perspectiveSettings.addTag("corners");
			perspectiveSettings.pushTag("corners");
			perspectiveSettings.addTag("corner");
			float offX = initialOffset.x/texture->getWidth();
			float offY = initialOffset.y/texture->getHeight();
			perspectiveSettings.setValue("corner:x",offX , 0);
			perspectiveSettings.setValue("corner:y",offY, 0);
			perspectiveSettings.setValue("corner:x",offX + 1.0, 1);
			perspectiveSettings.setValue("corner:y",offY, 1);
			perspectiveSettings.setValue("corner:x",offX + 1.0, 2);
			perspectiveSettings.setValue("corner:y",offY + 1.0, 2);
			perspectiveSettings.setValue("corner:x",offX, 3);
			perspectiveSettings.setValue("corner:y",offY + 1.0, 3);
			perspectiveSettings.popTag();

			loadPerspective(perspectiveSettings);			
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
}
void Controller::onBlendChange(float & value){
	if(blendT == gui.getFloatSlider("Blend Top")
	   && blendB == gui.getFloatSlider("Blend Down")
	   && blendL == gui.getFloatSlider("Blend Left")
	   && blendL == gui.getFloatSlider("Blend Right")){
		return;
	}
	guiHasChanged = true; // flag that gui has changed
	
	blendT = gui.getFloatSlider("Blend Top");
	blendB = gui.getFloatSlider("Blend Down");
	blendL = gui.getFloatSlider("Blend Left");
	blendR = gui.getFloatSlider("Blend Right");
}
void Controller::onEnablePerpective(bool & value){
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
    
    ofVec2f c_Size(texture->getWidth() / gridSize.x, texture->getHeight() / gridSize.y);
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
            
            controlMesh.addColor(ofFloatColor(1.0,1.0,1.0,1.0));
            controlMesh.addColor(ofFloatColor(1.0,1.0,1.0,1.0));
            controlMesh.addColor(ofFloatColor(1.0,1.0,1.0,1.0));
            controlMesh.addColor(ofFloatColor(1.0,1.0,1.0,1.0));
            
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
    if(args.key =='z'){
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
    if(args.key =='z'){
        if (ofGetModifierPressed(OF_KEY_CTRL)){
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
    if(args.key =='z'){
        if (ofGetModifierPressed(OF_KEY_CTRL)){
            if(ofGetModifierPressed(OF_KEY_SHIFT)){
                redo = true;
            }
            else{
                undo = true;
            }
        }
    }
    if(args.key =='y'){
        if (ofGetModifierPressed(OF_KEY_CTRL)){
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
    if(args.key =='r'){
        if (ofGetModifierPressed(OF_KEY_SPECIAL)){
            load = true;
        }
    }
    if(args.key =='s'){
        if (ofGetModifierPressed(OF_KEY_SPECIAL)){
            save = true;
        }
    }
#else
    if(args.key =='r'){
        if (ofGetModifierPressed(OF_KEY_CTRL)){
            load = true;
        }
    }
    if(args.key =='s'){
        if (ofGetModifierPressed(OF_KEY_CTRL)){
            save = true;
        }
    }
#endif
    bool dummy = true;
    if(load) onLoad(dummy);
    if(save) onSave(dummy);
    
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
 object is not drawn using draw(), we know here because draw() never set the
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
		} else {
			ofUnregisterMouseEvents(this);
			ofUnregisterKeyEvents(this);
		}
	}
	drawn = false; // turn the drawn flag off, for draw() to turn back on
}