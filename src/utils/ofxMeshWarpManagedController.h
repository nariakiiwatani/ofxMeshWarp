#pragma once

#include "ofxMeshWarpController.h"
#include "ofEvents.h"

namespace ofx{namespace MeshWarp{
namespace Editor {
	class ManagedController {
	public:
		ManagedController();
		~ManagedController();
		void add(Mesh *target);
		void enable();
		void disable();
		void setEnable(bool set) { set?enable():disable(); }
		bool isEnabled() const { return is_enabled_; }
		void clear();
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
		bool is_enabled_ = false;
		bool is_mover_active_ = false;
		bool is_divider_active_ = false;
		PointController mover_;
		DivideController divider_;
		vector<ofxMeshWarpControllerBase*> controllers_{&mover_,&divider_};
		void activateMover();
		void activateDivider();
	};
}
}}
using ofxMeshWarpController = ofx::MeshWarp::Editor::ManagedController;
