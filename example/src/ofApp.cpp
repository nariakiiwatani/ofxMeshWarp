#include "ofApp.h"
#include "ofxMeshWarpIO.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofLoadImage(tex_, "of.png");
	mesh_ = shared_ptr<ofxMeshWarp>(new ofxMeshWarp());
	mesh_->setup(4,4,512,512);
	mesh_->setTexCoordSize(tex_.getWidth(), tex_.getHeight());
	controller_.add(mesh_.get());
	controller_.enable();
}

//--------------------------------------------------------------
void ofApp::update(){
	ofSetWindowTitle(ofToString(ofGetFrameRate(),2));
	mesh_->update();
}

//--------------------------------------------------------------
void ofApp::draw(){
	tex_.bind();
//	mesh_->drawMesh();
	controller_.drawFace();
	tex_.unbind();
	controller_.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	switch(key) {
		case 's': {
			ofxMeshWarpSave saver;
			saver.addMesh(mesh_.get());
			saver.save("hoge.txt");
		}	break;
		case 'l': {
			ofxMeshWarpLoad loader;
			const vector<ofxMeshWarp*> &result = loader.load("hoge.txt");
			if(!result.empty()) {
				controller_.clear();
				mesh_ = shared_ptr<ofxMeshWarp>(result[0]);
				controller_.add(mesh_.get());
			}
		}	break;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
