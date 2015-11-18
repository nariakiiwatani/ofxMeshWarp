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
		
		void mousePressed(ofMouseEventArgs &args);
		void mouseReleased(ofMouseEventArgs &args);
		void mouseMoved(ofMouseEventArgs &args);
		void mouseDragged(ofMouseEventArgs &args);
		void mouseScrolled(ofMouseEventArgs &args);
		void mouseEntered(ofMouseEventArgs &args);
		void mouseExited(ofMouseEventArgs &args);
		void keyPressed(ofKeyEventArgs &args);
		void keyReleased(ofKeyEventArgs &args);
	private:
		set<Mesh*> meshes_;
		
		bool is_enabled_;
		struct MouseOperation {
			MeshPoint *hover;
			vector<MeshPoint*> inside_rect;
			ofVec2f pressed_pos, pos;
			vector<PointHelper> edit;
			enum {
				STATE_NONE,
				STATE_GRABBING_VERTEX,
				STATE_GRABBING_COORD,
				STATE_MAKING_RECT,
			} pressed_state;
		} mouse_op_;
		set<MeshPoint*> selected_;
		float point_size_;
		float screen_to_coord_;
		
		bool isGrabbingVertex() const { return mouse_op_.pressed_state==MouseOperation::STATE_GRABBING_VERTEX; }
		bool isGrabbingCoord() const { return mouse_op_.pressed_state==MouseOperation::STATE_GRABBING_COORD; }
		bool isMakingRect() const { return mouse_op_.pressed_state==MouseOperation::STATE_MAKING_RECT; }
		bool isToggleNode() const { return ofGetKeyPressed(OF_KEY_ALT); }
		bool isSlide() const { return ofGetKeyPressed(OF_KEY_SHIFT); }
		bool isAdditive() const { return ofGetKeyPressed(OF_KEY_SHIFT); }
		bool isMultiSelect() const { return ofGetKeyPressed(OF_KEY_COMMAND); }
		bool isArrowKeyJump() const { return ofGetKeyPressed(OF_KEY_SHIFT); }
	};
}
}}
using ofxMeshWarpController = ofx::MeshWarp::Editor::Controller;
