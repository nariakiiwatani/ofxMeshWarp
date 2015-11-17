#include "ofxMeshWarpController.h"
#include "ofGraphics.h"

OFX_MESH_WARP_BEGIN_NAMESPACE
Editor::Controller::Controller()
{
	ofRegisterMouseEvents(this);
	ofRegisterKeyEvents(this);
	point_size_ = 10;
	screen_to_coord_ = 1/100.f;
	mouse_op_.pressed_state = MouseOperation::STATE_NONE;
}
Editor::Controller::~Controller()
{
	ofUnregisterMouseEvents(this);
	ofUnregisterKeyEvents(this);
}
void Editor::Controller::add(Mesh *mesh)
{
	meshes_.insert(mesh);
}
void Editor::Controller::clear()
{
	mouse_op_.hover = NULL;
	mouse_op_.inside_rect.clear();
	mouse_op_.edit.clear();
	selected_.clear();
	meshes_.clear();
}
void Editor::Controller::draw()
{
	ofPushStyle();
	for(auto &mesh : meshes_) {
		mesh->drawWireframe();
	}
	ofSetColor(ofColor::green);
	ofNoFill();
	for(auto &mesh : meshes_) {
		auto points = mesh->getPoints();
		for(auto &p : points) {
			ofDrawCircle(p->point(), point_size_);
		}
	}
	ofSetColor(ofColor::green);
	ofFill();
	if(mouse_op_.hover) {
		ofDrawCircle(mouse_op_.hover->point(), point_size_+2);
	}
	for(auto &p : mouse_op_.inside_rect) {
		ofDrawCircle(p->point(), point_size_+2);
	}
	ofSetColor(ofColor::red);
	ofFill();
	for(auto &p : selected_) {
		ofDrawCircle(p->point(), point_size_);
	}
	for(auto &p : mouse_op_.edit) {
		ofDrawCircle(p.first->point(), point_size_);
	}
	if(isMakingRect()) {
		ofFill();
		ofSetColor(ofColor::white, 128);
		ofDrawRectangle(ofRectangle(mouse_op_.pressed_pos, mouse_op_.pos));
	}
	ofPopStyle();
}
void Editor::Controller::mousePressed(ofMouseEventArgs &args)
{
	mouse_op_.pressed_pos = args;
	if(mouse_op_.hover) {
		mouse_op_.pressed_state = MouseOperation::STATE_GRABBING;
		const auto &it = selected_.insert(mouse_op_.hover);
		if(it.second) {
			if(!isMultiSelect() && !isAdditive()) {
				selected_.clear();
				selected_.insert(mouse_op_.hover);
			}
		}
		else {
			if(isMultiSelect()) {
				selected_.erase(mouse_op_.hover);
				mouse_op_.pressed_state = MouseOperation::STATE_NONE;
			}
		}
		if(isGrabbing()) {
			for(auto &p : selected_) {
				mouse_op_.edit.push_back(p);
			}
			mouse_op_.hover = NULL;
		}
	}
	else {
		mouse_op_.pressed_state = MouseOperation::STATE_MAKING_RECT;
	}
}
void Editor::Controller::mouseReleased(ofMouseEventArgs &args)
{
	if(isMakingRect()) {
		if(!isMultiSelect() && !isAdditive()) {
			selected_.clear();
		}
		for(auto &p : mouse_op_.inside_rect) {
			const auto &it = selected_.insert(p);
			if(isMultiSelect() && !it.second) {
				selected_.erase(it.first);
			}
		}
		mouse_op_.inside_rect.clear();
	}
	else {
		mouse_op_.edit.clear();
	}
	mouse_op_.pressed_state = MouseOperation::STATE_NONE;
	mouseMoved(args);
}
void Editor::Controller::mouseMoved(ofMouseEventArgs &args)
{
	mouse_op_.hover = NULL;
	mouse_op_.pos = args;
	for(auto &mesh : meshes_) {
		MeshHelper m(mesh);
		if(MeshPoint *hover = MeshHelper(mesh).getHit(args, point_size_)) {
			mouse_op_.hover = hover;
			break;
		}
	}
}
void Editor::Controller::mouseDragged(ofMouseEventArgs &args)
{
	mouse_op_.pos = args;
	if(isMakingRect()) {
		mouse_op_.inside_rect.clear();
		ofRectangle rect(mouse_op_.pressed_pos, args);
		for(auto &mesh : meshes_) {
			const auto &hit = MeshHelper(mesh).getHit(rect);
			mouse_op_.inside_rect.insert(mouse_op_.inside_rect.end(), hit.begin(), hit.end());
		}
	}
	else if(isGrabbing()) {
		ofVec2f delta = args-mouse_op_.pressed_pos;
		if(isSlide()) {
			delta.x = abs(delta.x)<abs(delta.y)?0:delta.x;
			delta.y = abs(delta.y)<abs(delta.x)?0:delta.y;
		}
		for(auto &work : mouse_op_.edit) {
			if(isEditCoord()) {
				work.setCoordMove(delta*screen_to_coord_);
				work.resetVertex();
			}
			else if(isEditVertex()) {
				work.setVertexMove(delta);
				work.resetCoord();
			}
		}
	}
}
void Editor::Controller::mouseScrolled(ofMouseEventArgs &args)
{
}
void Editor::Controller::mouseEntered(ofMouseEventArgs &args)
{
}
void Editor::Controller::mouseExited(ofMouseEventArgs &args)
{
}
void Editor::Controller::keyPressed(ofKeyEventArgs &args)
{
	ofVec2f delta;
	switch(args.key) {
		case OF_KEY_UP:		delta = ofVec2f(0,-1); break;
		case OF_KEY_DOWN:	delta = ofVec2f(0, 1); break;
		case OF_KEY_LEFT:	delta = ofVec2f(-1,0); break;
		case OF_KEY_RIGHT:	delta = ofVec2f(1, 0); break;
	}
	for(auto &p : selected_) {
		if(isEditCoord()) {
			PointHelper(p).moveCoord(delta*(isArrowKeyJump()?10:1)*screen_to_coord_);
		}
		else if(isEditVertex()) {
			PointHelper(p).moveVertex(delta*(isArrowKeyJump()?10:1));
		}
	}
}
void Editor::Controller::keyReleased(ofKeyEventArgs &args)
{
}


MeshPoint* Editor::MeshHelper::getHit(const ofVec2f &test, float room, int index) const
{
	const vector<MeshPoint*> &points = target_->getPoints();
	for(auto &p : points) {
		if(index < 0) {
			return NULL;
		}
		if(test.squareDistance(p->point()) < room*room && index-- == 0) {
			return p;
		}
	}
	return NULL;
}
vector<MeshPoint*> Editor::MeshHelper::getHit(const ofRectangle &test) const
{
	vector<MeshPoint*> ret;
	const vector<MeshPoint*> &points = target_->getPoints();
	for(auto &p : points) {
		if(test.inside(p->point())) {
			ret.push_back(p);
		}
	}
	return ret;
}
OFX_MESH_WARP_END_NAMESPACE