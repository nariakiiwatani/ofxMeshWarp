#include "ofxMeshWarpController.h"
#include "ofGraphics.h"
#include "glm/vec3.hpp"

using namespace ofx::MeshWarp;
using namespace ofx::MeshWarp::Editor;
using namespace std;

ControllerBase::ControllerBase()
{
//	enable();
}
ControllerBase::~ControllerBase()
{
	disable();
}
void ControllerBase::enable()
{
	if(!is_enabled_) {
		ofRegisterMouseEvents(this);
		ofRegisterKeyEvents(this);
		is_enabled_ = true;
	}
}
void ControllerBase::disable()
{
	if(is_enabled_) {
		ofUnregisterMouseEvents(this);
		ofUnregisterKeyEvents(this);
		is_enabled_ = false;
		clearOperation();
	}
}
void ControllerBase::add(shared_ptr<Mesh> mesh)
{
	meshes_.insert(mesh);
}
void ControllerBase::clear()
{
	clearOperation();
	meshes_.clear();
}
void ControllerBase::draw() const
{
	ofPushMatrix();
	ofTranslate(translation_);
	ofScale(scale_, scale_);
	ofTranslate(-anchor_point_);
	for(auto &mesh : meshes_) {
		mesh->drawWireframe();
	}
	drawCustom();
	ofPopMatrix();
}
void ControllerBase::drawFace() const
{
	ofPushMatrix();
	ofTranslate(translation_);
	ofScale(scale_, scale_);
	ofTranslate(-anchor_point_);
	for(auto &mesh : meshes_) {
		mesh->drawMesh();
	}
	ofPopMatrix();
}
// ==========
void PointController::clearOperation()
{
	mouse_op_.hover = nullptr;
	mouse_op_.inside_rect.clear();
	mouse_op_.edit.clear();
	selected_.clear();
	mouse_op_.pressed_state = MouseOperation::STATE_NONE;
}
void PointController::drawCustom() const
{
	auto drawCircle = [&](MeshPoint* p, float size_add=0) {
		float size = point_size_*(p->isNode()?1:0.5f)+size_add;
		ofDrawCircle(p->point(), size/scale_);
	};
	ofPushStyle();
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

bool PointController::isEditing() const
{
	return mouse_op_.hover!=nullptr || isGrabbing() || isMakingRect();
}
void PointController::mousePressed(ofMouseEventArgs &args)
{
	glm::vec2 local = screenToLocal(args);
	mouse_op_.pressed_pos = local;
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
				for(auto &m : meshes_) {
					m->setDirty();
				}
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
	glm::vec2 local = screenToLocal(args);
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
	mouse_op_.hover = getHit(local);
}
void PointController::mouseMoved(ofMouseEventArgs &args)
{
	glm::vec2 local = screenToLocal(args);
	mouse_op_.pos = local;
	mouse_op_.hover = getHit(local);
}
MeshPoint* PointController::getHit(const glm::vec2 &test) const
{
	for(auto &mesh : meshes_) {
		MeshHelper m(mesh.get());
		if(MeshPoint *hover = m.getHit(test, point_size_/scale_)) {
			return hover;
		}
	}
	return nullptr;
}

void PointController::mouseDragged(ofMouseEventArgs &args)
{
	glm::vec2 local = screenToLocal(args);
	mouse_op_.pos = local;
	if(isMakingRect()) {
		mouse_op_.inside_rect.clear();
		ofRectangle rect(mouse_op_.pressed_pos, local);
		for(auto &mesh : meshes_) {
			const auto &hit = MeshHelper(mesh.get()).getHit(rect);
			mouse_op_.inside_rect.insert(end(mouse_op_.inside_rect), begin(hit), end(hit));
		}
	}
	else if(isGrabbing()) {
		glm::vec2 delta = local-mouse_op_.pressed_pos;
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
				m->setDirty();
			}
		}
	}
}
void PointController::mouseScrolled(ofMouseEventArgs &args)
{
	float delta = args.scrollY*scroll_to_alpha_;
	bool moved_any = false;
	for(auto &p : selected_) {
		if(p->isNode()) {
			float alpha = ofClamp(p->alpha()+delta, 0, ofFloatColor::limit());
			PointHelper(p).setAlpha(alpha);
			moved_any = true;
		}
	}
	if(moved_any) {
		for(auto &m : meshes_) {
			m->setDirty();
		}
	}
}
void PointController::mouseEntered(ofMouseEventArgs &args)
{
}
void PointController::mouseExited(ofMouseEventArgs &args)
{
}
void PointController::keyPressed(ofKeyEventArgs &args)
{
	glm::vec2 delta;
	switch(args.key) {
		case OF_KEY_UP:		delta = glm::vec2(0,-1)/scale_; break;
		case OF_KEY_DOWN:	delta = glm::vec2(0, 1)/scale_; break;
		case OF_KEY_LEFT:	delta = glm::vec2(-1,0)/scale_; break;
		case OF_KEY_RIGHT:	delta = glm::vec2(1, 0)/scale_; break;
	}
	if(glm::length2(delta) > 0) {
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
				m->setDirty();
			}
		}
		mouse_op_.hover = getHit(mouse_op_.pos);
	}
}
void PointController::keyReleased(ofKeyEventArgs &args)
{
}

// ==========
void DivideController::clearOperation()
{
	hit_info_ = HitInfo();
}
void DivideController::drawCustom() const
{
	for(auto &mesh : meshes_) {
		mesh->drawWireframe();
	}
	if(hit_info_.mesh) {
		const auto &points = hit_info_.mesh->getPoints();
		if(isDivide()) {
			ofPushStyle();
			if(hit_info_.isLineX()) {
				const auto &points0 = MeshHelper(hit_info_.mesh).getColPoints(hit_info_.line_index_0);
				const auto &points1 = MeshHelper(hit_info_.mesh).getColPoints(hit_info_.line_index_1);
				assert(points0.size() == points1.size());
				ofSetColor(ofColor::green);
				glBegin(GL_LINE_STRIP);
				for(size_t i = 0, num = points0.size(); i < num; ++i) {
					glm::vec2 p = glm::mix(points0[i]->point(), points1[i]->point(), hit_info_.pos_intersection);
					glVertex2fv(&p[0]);
				}
				glEnd();
			}
			if(hit_info_.isLineY()) {
				const auto &points0 = MeshHelper(hit_info_.mesh).getRowPoints(hit_info_.line_index_0);
				const auto &points1 = MeshHelper(hit_info_.mesh).getRowPoints(hit_info_.line_index_1);
				assert(points0.size() == points1.size());
				ofSetColor(ofColor::green);
				glBegin(GL_LINE_STRIP);
				for(size_t i = 0, num = points0.size(); i < num; ++i) {
					glm::vec2 p = glm::mix(points0[i]->point(), points1[i]->point(), hit_info_.pos_intersection);
					glVertex2fv(&p[0]);
				}
				glEnd();
			}
			ofPopStyle();
		}
		if(isReduce()) {
			ofPushStyle();
			if(hit_info_.isLineX()) {
				const auto &points = MeshHelper(hit_info_.mesh).getRowPoints(hit_info_.line_index_0);
				ofSetColor(ofColor::red);
				glBegin(GL_LINE_STRIP);
				for(auto &p : points) {
					glVertex2f(p->point().x, p->point().y);
				}
				glEnd();
			}
			if(hit_info_.isLineY()) {
				const auto &points = MeshHelper(hit_info_.mesh).getColPoints(hit_info_.line_index_0);
				ofSetColor(ofColor::red);
				glBegin(GL_LINE_STRIP);
				for(auto &p : points) {
					glVertex2f(p->point().x, p->point().y);
				}
				glEnd();
			}
			ofPopStyle();
		}
	}
}

bool DivideController::isEditing() const
{
	return hit_info_.isLine();
}

void DivideController::mousePressed(ofMouseEventArgs &args)
{
	glm::vec2 local = screenToLocal(args);
	bool dirty = false;
	if(isDivide()) {
		if(hit_info_.isLineX()) {
			hit_info_.mesh->divideCol(hit_info_.line_index_0%hit_info_.mesh->getDivX(), hit_info_.pos_intersection);
			dirty = true;
		}
		if(hit_info_.isLineY()) {
			hit_info_.mesh->divideRow(hit_info_.line_index_0/hit_info_.mesh->getDivX(), hit_info_.pos_intersection);
			dirty = true;
		}
	}
	if(isReduce()) {
		if(hit_info_.isLineX()) {
			hit_info_.mesh->reduceRow(hit_info_.line_index_0/hit_info_.mesh->getDivX());
			dirty = true;
		}
		if(hit_info_.isLineY()) {
			hit_info_.mesh->reduceCol(hit_info_.line_index_0%hit_info_.mesh->getDivX());
			dirty = true;
		}
	}
	if(dirty) {
		hit_info_.mesh->setDirty();
		hit_info_ = getHitInfo(local);
	}
}
void DivideController::mouseReleased(ofMouseEventArgs &args)
{
}
void DivideController::mouseMoved(ofMouseEventArgs &args)
{
	glm::vec2 local = screenToLocal(args);
	hit_info_ = getHitInfo(local);
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
DivideController::HitInfo DivideController::getHitInfo(const glm::vec2 &test) const
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
				if(MeshHelper(m.get()).isHitBox(test, index)) {
					info.mesh = m.get();
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
			if(MeshHelper(info.mesh).isHitLine(test, index0, index1, line_hit_size_/scale_, info.pos_intersection)) {
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
MeshPoint* MeshHelper::getHit(const glm::vec2 &test, float room, int index) const
{
	const vector<MeshPoint*> &points = target_->getPoints();
	for(auto &p : points) {
		if(index < 0) {
			return nullptr;
		}
		if(glm::distance2(glm::vec3(test,0), p->point()) < room*room && index-- == 0) {
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
bool MeshHelper::isHitBox(const glm::vec2 &test, int top_left_index) const
{
	const auto &box = getBox(top_left_index);
	assert(box.size() == 4);
	ofPolyline poly;
	poly.addVertex(box[0]->point());
	poly.addVertex(box[1]->point());
	poly.addVertex(box[3]->point());
	poly.addVertex(box[2]->point());
	return poly.inside(test.x, test.y);
}
bool MeshHelper::isHitLine(const glm::vec2 &test, int index0, int index1, float room, float &pos) const
{
	const auto &points = target_->getPoints();
	const glm::vec3 &pivot = points[index0]->point();
	const glm::vec3 &anchor = points[index1]->point();
	float angle = atan2((test-pivot).y, (test-pivot).x)-atan2((anchor-pivot).y, (anchor-pivot).x);
	glm::vec3 alternative = glm::rotate(glm::vec3(test,0)-pivot, -angle*2, glm::vec3(0,0,1)) + pivot;
	glm::vec3 intersection = (glm::vec3(test,0)+alternative)/2.f;
	if(pivot.x == anchor.x) {
		pos = ofMap(intersection.y, pivot.y, anchor.y, 0, 1, false);
	}
	else {
		pos = ofMap(intersection.x, pivot.x, anchor.x, 0, 1, false);
	}
	if(0<=pos&&pos<=1) {
		return glm::distance2(glm::vec3(test,0), intersection) < room*room;
	}
	return false;
}

vector<MeshPoint*> MeshHelper::getColPoints(int point_index) const
{
	vector<MeshPoint*> ret;
	int div_x = target_->getDivX();
	const auto &points = target_->getPoints();
	for(size_t index = point_index%div_x, num = points.size(); index < num; index += div_x) {
		ret.emplace_back(points[index]);
	}
	return ret;
}
vector<MeshPoint*> MeshHelper::getRowPoints(int point_index) const
{
	vector<MeshPoint*> ret;
	int div_x = target_->getDivX();
	int start = (point_index/div_x)*div_x;
	const auto &points = target_->getPoints();
	for(int index = start; index < start+div_x; ++index) {
		ret.emplace_back(points[index]);
	}
	return ret;
}
