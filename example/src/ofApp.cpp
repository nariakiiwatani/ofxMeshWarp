#include "ofApp.h"
#include "ofxMeshWarpIO.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofLoadImage(tex_, "of.png");
	mesh_.setup(4,4,512,512);
	mesh_.setTexCoordSize(tex_.getWidth(), tex_.getHeight());
	controller_.add(&mesh_);
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
	tex_.bind();
	mesh_.drawMesh();
	tex_.unbind();
	controller_.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	switch(key) {
		case 's':
			ofxMeshWarpSave(&mesh_, "hoge.txt");
			break;
		case 'l':
			controller_.clear();
			ofxMeshWarpLoad(&mesh_, "hoge.txt");
			controller_.add(&mesh_);
			break;
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