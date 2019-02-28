#pragma once

#include "ofEvents.h"
#include "ofxMeshWarp.h"
#include "ofRectangle.h"
#include "ofPolyline.h"

namespace ofx{namespace MeshWarp{
namespace Editor {
	class PointHelper : public std::pair<MeshPoint*,MeshPoint> {
	public:
		PointHelper(MeshPoint *p):pair(p,*p){}
		void setVertex(const glm::vec2 &point) { first->setPoint(glm::vec3(point,0)); }
		void setVertexMove(const glm::vec2 &move) { first->setPoint(second.point()+move); }
		void moveVertex(const glm::vec2 &move) { first->setPoint(first->point()+move); }
		void resetVertex() { first->setPoint(second.point()); }
		void setCoord(const glm::vec2 &point) { first->setCoord(point); }
		void setCoordMove(const glm::vec2 &move) { first->setCoord(second.coord()+move); }
		void moveCoord(const glm::vec2 &move) { first->setCoord(first->coord()+move); }
		void resetCoord() { first->setCoord(second.coord()); }
		void setColor(const ofFloatColor &color) { first->setColor(color); }
		void resetColor() { first->setColor(second.color()); }
		void setAlpha(float alpha) { first->setAlpha(alpha); }
		void addAlpha(float alpha) { first->setAlpha(first->alpha()+alpha); }
		void resetAlpha() { first->setAlpha(second.alpha()); }
	};
	class MeshHelper
	{
	public:
		MeshHelper(Mesh *m):target_(m){}
		MeshPoint* getHit(const glm::vec2 &test, float room, int index=0) const;
		std::vector<MeshPoint*> getHit(const ofRectangle &test) const;
		std::vector<MeshPoint*> getBox(int top_left_index) const;
		std::vector<int> getBoxIndices(int top_left_index) const;
		bool isHitBox(const glm::vec2 &test, int top_left_index) const;
		bool isHitLine(const glm::vec2 &test, int index0, int index1, float room, float &pos) const;
		std::vector<MeshPoint*> getColPoints(int point_index) const;
		std::vector<MeshPoint*> getRowPoints(int point_index) const;
	private:
		Mesh *target_;
	};
	
	class ControllerBase {
	public:
		ControllerBase();
		virtual ~ControllerBase();
		void add(std::shared_ptr<Mesh> target);
		void enable();
		void disable();
		void setEnable(bool set) { set?enable():disable(); }
		bool isEnabled() const { return is_enabled_; }
		void clear();
		void draw() const;
		void drawFace() const;
		
		void setAnchorPoint(float x, float y) { setAnchorPoint(glm::vec2(x,y)); }
		void setAnchorPoint(const glm::vec2 &point) { anchor_point_ = point; }
		void setTranslation(float x, float y) { setTranslation(glm::vec2(x,y)); }
		void setTranslation(const glm::vec2 &point) { translation_ = point; }
		void setScale(float s) { assert(s>0); scale_ = s; }
		
		virtual bool isEditing() const { return false; }
		
		virtual void clearOperation(){};

		virtual void mousePressed(ofMouseEventArgs &args){}
		virtual void mouseReleased(ofMouseEventArgs &args){}
		virtual void mouseMoved(ofMouseEventArgs &args){}
		virtual void mouseDragged(ofMouseEventArgs &args){}
		virtual void mouseScrolled(ofMouseEventArgs &args){}
		virtual void mouseEntered(ofMouseEventArgs &args){}
		virtual void mouseExited(ofMouseEventArgs &args){}
		virtual void keyPressed(ofKeyEventArgs &args){}
		virtual void keyReleased(ofKeyEventArgs &args){}
	protected:
		std::set<std::shared_ptr<Mesh>> meshes_;
		bool is_enabled_ = false;
		virtual void drawCustom() const{};
		
		glm::vec2 anchor_point_=glm::vec2(0,0);
		glm::vec2 translation_=glm::vec2(0,0);
		float scale_=1;
		glm::vec2 screenToLocal(glm::vec2 src) const { return (src-translation_)/scale_+anchor_point_; }
		glm::vec2 localToScreen(glm::vec2 src) const { return (src-anchor_point_)*scale_+translation_; }
	};

	class PointController : public ControllerBase
	{
	public:
		void clearOperation();
		void drawCustom() const;
		bool isEditing() const;

		void setUVResolution(const glm::vec2 &res) { screen_to_coord_ = res; }
		
		virtual void mousePressed(ofMouseEventArgs &args);
		virtual void mouseReleased(ofMouseEventArgs &args);
		virtual void mouseMoved(ofMouseEventArgs &args);
		virtual void mouseDragged(ofMouseEventArgs &args);
		virtual void mouseScrolled(ofMouseEventArgs &args);
		virtual void mouseEntered(ofMouseEventArgs &args);
		virtual void mouseExited(ofMouseEventArgs &args);
		virtual void keyPressed(ofKeyEventArgs &args);
		virtual void keyReleased(ofKeyEventArgs &args);
	protected:
		struct MouseOperation {
			MeshPoint *hover = nullptr;
			std::vector<MeshPoint*> inside_rect;
			glm::vec2 pressed_pos, pos;
			std::vector<PointHelper> edit;
			enum {
				STATE_NONE,
				STATE_GRABBING,
				STATE_MAKING_RECT,
			} pressed_state = STATE_NONE;
		} mouse_op_;
		std::set<MeshPoint*> selected_;
		float point_size_ = 10;
		glm::vec2 screen_to_coord_=glm::vec2(1024.f,1024.f);
		float scroll_to_alpha_ = -1/100.f;
		
		bool isGrabbing() const { return mouse_op_.pressed_state==MouseOperation::STATE_GRABBING; }
		bool isMakingRect() const { return mouse_op_.pressed_state==MouseOperation::STATE_MAKING_RECT; }
		virtual bool isToggleNode() const { return ofGetKeyPressed(OF_KEY_ALT); }
		virtual bool isSlide() const { return ofGetKeyPressed(OF_KEY_SHIFT); }
		virtual bool isAdditive() const { return ofGetKeyPressed(OF_KEY_SHIFT); }
		virtual bool isAlternative() const { return ofGetKeyPressed(OF_KEY_COMMAND); }
		virtual bool isArrowKeyJump() const { return ofGetKeyPressed(OF_KEY_SHIFT); }
		virtual bool isEditCoord() const { return ofGetKeyPressed(OF_KEY_ALT); }
		virtual bool isEditVertex() const { return !ofGetKeyPressed(OF_KEY_ALT); }
		
		MeshPoint* getHit(const glm::vec2 &test) const;
	};
	class DivideController  : public ControllerBase
	{
	public:
		void clearOperation();
		void drawCustom() const;
		bool isEditing() const;
		
		virtual void mousePressed(ofMouseEventArgs &args);
		virtual void mouseReleased(ofMouseEventArgs &args);
		virtual void mouseMoved(ofMouseEventArgs &args);
		virtual void mouseDragged(ofMouseEventArgs &args);
		virtual void mouseScrolled(ofMouseEventArgs &args);
		virtual void mouseEntered(ofMouseEventArgs &args);
		virtual void mouseExited(ofMouseEventArgs &args);
		virtual void keyPressed(ofKeyEventArgs &args);
		virtual void keyReleased(ofKeyEventArgs &args);
	protected:
		float line_hit_size_ = 10;
		struct HitInfo {
			Mesh *mesh = nullptr;
			int area_index = -1;
			int line_index_0 = -1, line_index_1 = -1;
			float pos_intersection = -1;
			bool isArea() const { return area_index; }
			bool isLine() const { return line_index_0 != -1 && line_index_1 != -1; }
			bool isLineX() const { return isLine() && abs(line_index_0-line_index_1)==1; }
			bool isLineY() const { return isLine() && !isLineX(); }
		} hit_info_;
		
		virtual bool isDivide() const { return !ofGetKeyPressed(OF_KEY_ALT); }
		virtual bool isReduce() const { return ofGetKeyPressed(OF_KEY_ALT); }
		HitInfo getHitInfo(const glm::vec2 &test) const;
	};
}
}}
using ofxMeshWarpControllerBase = ofx::MeshWarp::Editor::ControllerBase;
using ofxMeshWarpPointController = ofx::MeshWarp::Editor::PointController;
using ofxMeshWarpMeshDivider = ofx::MeshWarp::Editor::DivideController;
