#include "ofxMeshWarpController.h"
#include "ofGraphics.h"

using namespace ofx::MeshWarp;
using namespace ofx::MeshWarp::Editor;

PointController::PointController()
{
//	enable();
}
PointController::~PointController()
{
	disable();
}
void PointController::enable()
{
	if(!is_enabled_) {
		ofRegisterMouseEvents(this);
		ofRegisterKeyEvents(this);
		is_enabled_ = true;
	}
}
void PointController::disable()
{
	if(is_enabled_) {
		ofUnregisterMouseEvents(this);
		ofUnregisterKeyEvents(this);
		mouse_op_.hover = nullptr;
		mouse_op_.inside_rect.clear();
		mouse_op_.edit.clear();
		selected_.clear();
		mouse_op_.pressed_state = MouseOperation::STATE_NONE;
		is_enabled_ = false;
	}
}
void PointController::add(Mesh *mesh)
{
	meshes_.insert(mesh);
}
void PointController::clear()
{
	mouse_op_.hover = nullptr;
	mouse_op_.inside_rect.clear();
	mouse_op_.edit.clear();
	selected_.clear();
	mouse_op_.pressed_state = MouseOperation::STATE_NONE;
	meshes_.clear();
}
void PointController::draw()
{
	auto drawCircle = [&](MeshPoint* p, float size_add=0) {
		float size = point_size_*(p->isNode()?1:0.5f)+size_add;
		ofDrawCircle(p->point(), size);
	};
	ofPushStyle();
	for(auto &mesh : meshes_) {
		mesh->drawWireframe();
	}
	ofSetColor(ofColor::green);
	ofNoFill();
	for(auto &mesh : meshes_) {
		auto points = mesh->getPoints();
		for(auto &p : points) {
			drawCircle(p);
		}
	}
	ofSetColor(ofColor::green);
	ofFill();
	if(mouse_op_.hover) {
		drawCircle(mouse_op_.hover, 2);
	}
	for(auto &p : mouse_op_.inside_rect) {
		drawCircle(p, 2);
	}
	ofSetColor(ofColor::red);
	ofFill();
	for(auto &p : selected_) {
		drawCircle(p);
	}
	for(auto &p : mouse_op_.edit) {
		drawCircle(p.first);
	}
	if(isMakingRect()) {
		ofFill();
		ofSetColor(ofColor::white, 128);
		ofDrawRectangle(ofRectangle(mouse_op_.pressed_pos, mouse_op_.pos));
	}
	ofPopStyle();
}
void PointController::mousePressed(ofMouseEventArgs &args)
{
	mouse_op_.pressed_pos = args;
	mouse_op_.pressed_state = MouseOperation::STATE_NONE;
	switch(args.button) {
		case OF_MOUSE_BUTTON_LEFT:
			if(mouse_op_.hover) {
				mouse_op_.pressed_state = MouseOperation::STATE_GRABBING;
			}
			else {
				mouse_op_.pressed_state = MouseOperation::STATE_MAKING_RECT;
			}
			break;
		case OF_MOUSE_BUTTON_MIDDLE:
			if(mouse_op_.hover) {
				mouse_op_.pressed_state = MouseOperation::STATE_GRABBING;
			}
			break;
	}
	switch(mouse_op_.pressed_state) {
		case MouseOperation::STATE_GRABBING: {
			const auto &it = selected_.insert(mouse_op_.hover);
			if(it.second) {
				if(!isAlternative() && !isAdditive()) {
					selected_.clear();
					selected_.insert(mouse_op_.hover);
				}
			}
			else {
				if(isAlternative()) {
					selected_.erase(mouse_op_.hover);
					mouse_op_.pressed_state = MouseOperation::STATE_NONE;
				}
			}
			if(isToggleNode()) {
				bool set = !mouse_op_.hover->isNode();
				mouse_op_.hover->setNodal(set);
				for(auto &p : selected_) {
					p->setNodal(set);
				}
				mouse_op_.pressed_state = MouseOperation::STATE_NONE;
			}
			else if(isGrabbing()) {
				for(auto &p : selected_) {
					mouse_op_.edit.push_back(p);
				}
				mouse_op_.hover = nullptr;
			}
		}	break;
		default:
			break;
	}
}
void PointController::mouseReleased(ofMouseEventArgs &args)
{
	if(isMakingRect()) {
		if(!isAlternative() && !isAdditive()) {
			selected_.clear();
		}
		for(auto &p : mouse_op_.inside_rect) {
			const auto &it = selected_.insert(p);
			if(isAlternative() && !it.second) {
				selected_.erase(it.first);
			}
		}
		mouse_op_.inside_rect.clear();
	}
	else {
		mouse_op_.edit.clear();
	}
	mouse_op_.pressed_state = MouseOperation::STATE_NONE;
	mouse_op_.hover = getHit(args);
}
void PointController::mouseMoved(ofMouseEventArgs &args)
{
	mouse_op_.pos = args;
	mouse_op_.hover = getHit(args);
}
MeshPoint* PointController::getHit(const ofVec2f &test) const
{
	for(auto &mesh : meshes_) {
		MeshHelper m(mesh);
		if(MeshPoint *hover = MeshHelper(mesh).getHit(test, point_size_)) {
			return hover;
		}
	}
	return nullptr;
}

void PointController::mouseDragged(ofMouseEventArgs &args)
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
		bool moved_any = false;
		for(auto &work : mouse_op_.edit) {
			if(work.first->isNode()) {
				if(isEditVertex()) {
					work.setVertexMove(delta);
					work.resetCoord();
				}
				if(isEditCoord()) {
					work.setCoordMove(delta*screen_to_coord_);
					work.resetVertex();
				}
				moved_any = true;
			}
		}
		if(moved_any) {
			for(auto &m : meshes_) {
				m->solve();
			}
		}
	}
}
void PointController::mouseScrolled(ofMouseEventArgs &args)
{
}
void PointController::mouseEntered(ofMouseEventArgs &args)
{
}
void PointController::mouseExited(ofMouseEventArgs &args)
{
}
void PointController::keyPressed(ofKeyEventArgs &args)
{
	ofVec2f delta;
	switch(args.key) {
		case OF_KEY_UP:		delta = ofVec2f(0,-1); break;
		case OF_KEY_DOWN:	delta = ofVec2f(0, 1); break;
		case OF_KEY_LEFT:	delta = ofVec2f(-1,0); break;
		case OF_KEY_RIGHT:	delta = ofVec2f(1, 0); break;
	}
	if(delta.lengthSquared() > 0) {
		bool moved_any = false;
		for(auto &p : selected_) {
			if(p->isNode()) {
				if(isEditVertex()) {
					PointHelper(p).moveVertex(delta*(isArrowKeyJump()?10:1));
				}
				if(isEditCoord()){
					PointHelper(p).moveCoord(delta*(isArrowKeyJump()?10:1)*screen_to_coord_);
				}
				moved_any = true;
			}
		}
		if(moved_any) {
			for(auto &m : meshes_) {
				m->solve();
			}
		}
		mouse_op_.hover = getHit(mouse_op_.pos);
	}
}
void PointController::keyReleased(ofKeyEventArgs &args)
{
}

// ==========
DivideController::DivideController()
{
	//	enable();
}
DivideController::~DivideController()
{
	disable();
}
void DivideController::enable()
{
	if(!is_enabled_) {
		ofRegisterMouseEvents(this);
		ofRegisterKeyEvents(this);
		is_enabled_ = true;
	}
}
void DivideController::disable()
{
	if(is_enabled_) {
		ofUnregisterMouseEvents(this);
		ofUnregisterKeyEvents(this);
		is_enabled_ = false;
	}
}
void DivideController::add(Mesh *mesh)
{
	meshes_.insert(mesh);
}
void DivideController::clear()
{
	meshes_.clear();
}
void DivideController::draw()
{
	if(hit_info_.mesh) {
		const auto &points = hit_info_.mesh->getPoints();
		ofPushStyle();
		if(hit_info_.area_index != -1) {
			ofSetColor(ofColor::green);
			const auto &box = MeshHelper(hit_info_.mesh).getBox(hit_info_.area_index);
			glBegin(GL_TRIANGLE_STRIP);
			for(auto &p : box) {
				glVertex2f(p->point().x, p->point().y);
			}
			glEnd();
		}
		if(hit_info_.line_index_0 != -1 && hit_info_.line_index_1 != -1) {
			ofSetLineWidth(line_hit_size_*2);
			ofSetColor(ofColor::red);
			ofDrawLine(points[hit_info_.line_index_0]->point(), points[hit_info_.line_index_1]->point());
			ofSetColor(ofColor::blue);
			ofDrawCircle(points[hit_info_.line_index_0]->point().getInterpolated(points[hit_info_.line_index_1]->point(), hit_info_.pos_intersection), 10);
		}
		ofPopStyle();
	}
}

void DivideController::mousePressed(ofMouseEventArgs &args)
{
}
void DivideController::mouseReleased(ofMouseEventArgs &args)
{
}
void DivideController::mouseMoved(ofMouseEventArgs &args)
{
	hit_info_ = getHitInfo(args);
}
void DivideController::mouseDragged(ofMouseEventArgs &args)
{
}
void DivideController::mouseScrolled(ofMouseEventArgs &args)
{
}
void DivideController::mouseEntered(ofMouseEventArgs &args)
{
}
void DivideController::mouseExited(ofMouseEventArgs &args)
{
}
void DivideController::keyPressed(ofKeyEventArgs &args)
{
}
void DivideController::keyReleased(ofKeyEventArgs &args)
{
}
DivideController::HitInfo DivideController::getHitInfo(const ofVec2f &test)
{
	HitInfo info;
	bool is_hit = false;
	for(auto &m : meshes_) {
		const auto &points = m->getPoints();
		int div_x = m->getDivX();
		int div_y = m->getDivY();
		for(int y = 0; y < div_y-1; ++y) {
			for(int x = 0; x < div_x-1; ++x) {
				int index = y*div_x+x;
				if(MeshHelper(m).isHitBox(test, index)) {
					info.mesh = m;
					info.area_index = index;
					is_hit = true;
				}
			}
		}
	}
	if(is_hit) {
		const auto &indices = MeshHelper(info.mesh).getBoxIndices(info.area_index);
		assert(indices.size() == 4);
		auto check = [this,&info,&test](int index0, int index1) {
			if(MeshHelper(info.mesh).isHitLine(test, index0, index1, line_hit_size_, info.pos_intersection)) {
				info.line_index_0 = index0; info.line_index_1 = index1;
				return true;
			}
			return false;
		};
		if(check(indices[0], indices[1])) return info;
		if(check(indices[0], indices[2])) return info;
		if(check(indices[1], indices[3])) return info;
		if(check(indices[2], indices[3])) return info;
	}
	return info;
}


// ==========
MeshPoint* MeshHelper::getHit(const ofVec2f &test, float room, int index) const
{
	const vector<MeshPoint*> &points = target_->getPoints();
	for(auto &p : points) {
		if(index < 0) {
			return nullptr;
		}
		if(test.squareDistance(p->point()) < room*room && index-- == 0) {
			return p;
		}
	}
	return nullptr;
}
vector<MeshPoint*> MeshHelper::getHit(const ofRectangle &test) const
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
vector<MeshPoint*> MeshHelper::getBox(int top_left_index) const
{
	vector<MeshPoint*> ret;
	auto indices = getBoxIndices(top_left_index);
	const auto &points = target_->getPoints();
	for(auto i : indices) {
		ret.emplace_back(points[i]);
	}
	return ret;
}
vector<int> MeshHelper::getBoxIndices(int top_left_index) const
{
	vector<int> ret;;
	if(top_left_index+1 % target_->getDivX()+1 == 0) {
		ofLogWarning(__FILE__, "cannot get box with edge index(right)");
		return ret;
	}
	if(top_left_index >= (target_->getDivX()+1)*target_->getDivY()) {
		ofLogWarning(__FILE__, "cannot get box with edge index(bottom)");
		return ret;
	}
	ret.emplace_back(top_left_index);
	ret.emplace_back(top_left_index+1);
	ret.emplace_back(top_left_index+target_->getDivX());
	ret.emplace_back(top_left_index+target_->getDivX()+1);
	return ret;
}
bool MeshHelper::isHitBox(const ofVec2f &test, int top_left_index) const
{
	const auto &box = getBox(top_left_index);
	assert(box.size() == 4);
	ofPolyline poly;
	poly.addVertex(box[0]->point());
	poly.addVertex(box[1]->point());
	poly.addVertex(box[3]->point());
	poly.addVertex(box[2]->point());
	return poly.inside(test);
}
bool MeshHelper::isHitLine(const ofVec2f &test, int index0, int index1, float room, float &pos) const
{
	const auto &points = target_->getPoints();
	const ofPoint &pivot = points[index0]->point();
	const ofPoint &anchor = points[index1]->point();
	float angle = atan2((test-pivot).y, (test-pivot).x)-atan2((anchor-pivot).y, (anchor-pivot).x);
	ofPoint alternative = ofPoint(test).getRotatedRad(-angle*2, pivot, ofVec3f(0,0,1));
	ofPoint intersection = (test+alternative)/2.f;
	if(pivot.x == anchor.x) {
		pos = ofMap(intersection.y, pivot.y, anchor.y, 0, 1, false);
	}
	else {
		pos = ofMap(intersection.x, pivot.x, anchor.x, 0, 1, false);
	}
	if(0<=pos&&pos<=1) {
		return (test-intersection).lengthSquared() < room*room;
	}
	return false;
}

