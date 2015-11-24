#pragma once

#include "ofEvents.h"
#include "ofxMeshWarp.h"
#include "ofRectangle.h"

namespace ofx{namespace MeshWarp{
namespace Editor {
	class PointHelper : public pair<MeshPoint*,MeshPoint> {
	public:
		PointHelper(MeshPoint *p):pair(p,*p){}
		void setVertex(const ofVec2f &point) { first->setPoint(point); }
		void setVertexMove(const ofVec2f &move) { first->setPoint(second.point()+move); }
		void moveVertex(const ofVec2f &move) { first->setPoint(first->point()+move); }
		void resetVertex() { first->setPoint(second.point()); }
		void setCoord(const ofVec2f &point) { first->setCoord(point); }
		void setCoordMove(const ofVec2f &move) { first->setCoord(second.coord()+move); }
		void moveCoord(const ofVec2f &move) { first->setCoord(first->coord()+move); }
		void resetCoord() { first->setCoord(second.coord()); }
	};
	class MeshHelper
	{
	public:
		MeshHelper(Mesh *m):target_(m){}
		MeshPoint* getHit(const ofVec2f &test, float room, int index=0) const;
		vector<MeshPoint*> getHit(const ofRectangle &test) const;
	private:
		Mesh *target_;
	};

	class Controller
	{
	public:
		Controller();
		~Controller();
		void add(Mesh *target);
		void clear();
		void enable();
		void disable();
		void setEnable(bool set) { set?enable():disable(); }
		bool isEnabled() { return is_enabled_; }
		void draw();
		
		virtual void mousePressed(ofMouseEventArgs &args);
		virtual void mouseReleased(ofMouseEventArgs &args);
		virtual void mouseMoved(ofMouseEventArgs &args);
		virtual void mouseDragged(ofMouseEventArgs &args);
		virtual void mouseScrolled(ofMouseEventArgs &args);
		virtual void mouseEntered(ofMouseEventArgs &args);
		virtual void mouseExited(ofMouseEventArgs &args);
		virtual void keyPressed(ofKeyEventArgs &args);
		virtual void keyReleased(ofKeyEventArgs &args);
	private:
		set<Mesh*> meshes_;
		
		bool is_enabled_ = false;
		struct MouseOperation {
			MeshPoint *hover = nullptr;
			vector<MeshPoint*> inside_rect;
			ofVec2f pressed_pos, pos;
			vector<PointHelper> edit;
			enum {
				STATE_NONE,
				STATE_GRABBING,
				STATE_MAKING_RECT,
			} pressed_state = STATE_NONE;
		} mouse_op_;
		set<MeshPoint*> selected_;
		float point_size_ = 10;
		float screen_to_coord_ = 1/100.f;
		
		virtual bool isGrabbing() const { return mouse_op_.pressed_state==MouseOperation::STATE_GRABBING; }
		virtual bool isMakingRect() const { return mouse_op_.pressed_state==MouseOperation::STATE_MAKING_RECT; }
		virtual bool isToggleNode() const { return ofGetKeyPressed(OF_KEY_ALT); }
		virtual bool isSlide() const { return ofGetKeyPressed(OF_KEY_SHIFT); }
		virtual bool isAdditive() const { return ofGetKeyPressed(OF_KEY_SHIFT); }
		virtual bool isAlternative() const { return ofGetKeyPressed(OF_KEY_COMMAND); }
		virtual bool isArrowKeyJump() const { return ofGetKeyPressed(OF_KEY_SHIFT); }
		virtual bool isEditCoord() const { return ofGetKeyPressed(OF_KEY_ALT); }
		virtual bool isEditVertex() const { return !ofGetKeyPressed(OF_KEY_ALT); }
		
		MeshPoint* getHit(const ofVec2f &test) const;
	};
}
}}
using ofxMeshWarpController = ofx::MeshWarp::Editor::Controller;
