#pragma once

#include "ofEvents.h"
#include "ofxMeshWarp.h"
#include "ofRectangle.h"
#include "ofPolyline.h"

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
		vector<MeshPoint*> getBox(int top_left_index) const;
		vector<int> getBoxIndices(int top_left_index) const;
		bool isHitBox(const ofVec2f &test, int top_left_index) const;
		bool isHitLine(const ofVec2f &test, int index0, int index1, float room, float &pos) const;
		vector<MeshPoint*> getColPoints(int point_index) const;
		vector<MeshPoint*> getRowPoints(int point_index) const;
	private:
		Mesh *target_;
	};
	
	class ControllerBase {
	public:
		ControllerBase();
		~ControllerBase();
		void add(Mesh *target);
		void enable();
		void disable();
		void setEnable(bool set) { set?enable():disable(); }
		bool isEnabled() const { return is_enabled_; }
		void clear();
		void draw();
		
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
		set<Mesh*> meshes_;
		bool is_enabled_ = false;
		virtual void drawCustom(){};
	};

	class PointController : public ControllerBase
	{
	public:
		void clearOperation();
		void drawCustom();
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
		
		bool isGrabbing() const { return mouse_op_.pressed_state==MouseOperation::STATE_GRABBING; }
		bool isMakingRect() const { return mouse_op_.pressed_state==MouseOperation::STATE_MAKING_RECT; }
		virtual bool isToggleNode() const { return ofGetKeyPressed(OF_KEY_ALT); }
		virtual bool isSlide() const { return ofGetKeyPressed(OF_KEY_SHIFT); }
		virtual bool isAdditive() const { return ofGetKeyPressed(OF_KEY_SHIFT); }
		virtual bool isAlternative() const { return ofGetKeyPressed(OF_KEY_COMMAND); }
		virtual bool isArrowKeyJump() const { return ofGetKeyPressed(OF_KEY_SHIFT); }
		virtual bool isEditCoord() const { return ofGetKeyPressed(OF_KEY_ALT); }
		virtual bool isEditVertex() const { return !ofGetKeyPressed(OF_KEY_ALT); }
		
		MeshPoint* getHit(const ofVec2f &test) const;
	};
	class DivideController  : public ControllerBase
	{
	public:
		void clearOperation();
		void drawCustom();
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
		HitInfo getHitInfo(const ofVec2f &test);
	};
}
}}
using ofxMeshWarpControllerBase = ofx::MeshWarp::Editor::ControllerBase;
using ofxMeshWarpPointController = ofx::MeshWarp::Editor::PointController;
using ofxMeshWarpMeshDivider = ofx::MeshWarp::Editor::DivideController;
