#pragma once

#include "ofxMeshWarpController.h"
#include "ofEvents.h"

namespace ofx{namespace MeshWarp{
namespace Editor {
	class ManagedController : public ControllerBase {
	public:
		void add(Mesh *target);
		void clear();
		void clearOperation();

		void setAnchorPoint(float x, float y);
		void setTranslation(float x, float y);
		void setScale(float s);

		void drawCustom();

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
