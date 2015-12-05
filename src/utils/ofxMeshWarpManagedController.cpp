#include "ofxMeshWarpManagedController.h"
#include "ofGraphics.h"

using namespace ofx::MeshWarp;
using namespace ofx::MeshWarp::Editor;

ManagedController::ManagedController()
{
//	enable();
}
ManagedController::~ManagedController()
{
	disable();
}
void ManagedController::enable()
{
	if(!is_enabled_) {
		ofRegisterMouseEvents(this);
		ofRegisterKeyEvents(this);
		is_enabled_ = true;
	}
}
void ManagedController::disable()
{
	if(is_enabled_) {
		ofUnregisterMouseEvents(this);
		ofUnregisterKeyEvents(this);
		is_enabled_ = false;
		for_each(controllers_.begin(), controllers_.end(), [&](ControllerBase *t){t->clearOperation();});
	}
}
void ManagedController::add(Mesh *mesh)
{
	meshes_.insert(mesh);
	for_each(controllers_.begin(), controllers_.end(), [&](ControllerBase *t){t->add(mesh);});
}
void ManagedController::clear()
{
	meshes_.clear();
	for_each(controllers_.begin(), controllers_.end(), [&](ControllerBase *t){t->clear();});
}
void ManagedController::draw()
{
	for_each(meshes_.begin(), meshes_.end(), [&](Mesh *m){m->drawWireframe();});
	if(is_mover_active_) {
		mover_.drawCustom();
	}
	if(is_divider_active_) {
		divider_.drawCustom();
	}
}

void ManagedController::activateMover()
{
	is_mover_active_ = true;
	is_divider_active_ = false;
	divider_.clearOperation();
}
void ManagedController::activateDivider()
{
	is_mover_active_ = false;
	is_divider_active_ = true;
}
void ManagedController::mousePressed(ofMouseEventArgs &args)
{
	if(is_mover_active_) {
		mover_.mousePressed(args);
	}
	if(is_divider_active_) {
		divider_.mousePressed(args);
	}
}
void ManagedController::mouseReleased(ofMouseEventArgs &args)
{
	mover_.mouseReleased(args);
	divider_.mouseReleased(args);
	if(mover_.isEditing()) {
		activateMover();
	}
	else {
		if(divider_.isEditing()) {
			activateDivider();
		}
		else {
			activateMover();
		}
	}
}
void ManagedController::mouseMoved(ofMouseEventArgs &args)
{
	mover_.mouseMoved(args);
	if(mover_.isEditing()) {
		activateMover();
	}
	else {
		divider_.mouseMoved(args);
		if(divider_.isEditing()) {
			activateDivider();
		}
		else {
			activateMover();
		}
	}
}
void ManagedController::mouseDragged(ofMouseEventArgs &args)
{
	if(is_mover_active_) {
		mover_.mouseDragged(args);
	}
	if(is_divider_active_) {
		divider_.mouseDragged(args);
	}
}
void ManagedController::mouseScrolled(ofMouseEventArgs &args)
{
	if(is_mover_active_) {
		mover_.mouseScrolled(args);
	}
	if(is_divider_active_) {
		divider_.mouseScrolled(args);
	}
}
void ManagedController::mouseEntered(ofMouseEventArgs &args)
{
	if(is_mover_active_) {
		mover_.mouseEntered(args);
	}
	if(is_divider_active_) {
		divider_.mouseEntered(args);
	}
}
void ManagedController::mouseExited(ofMouseEventArgs &args)
{
	if(is_mover_active_) {
		mover_.mouseExited(args);
	}
	if(is_divider_active_) {
		divider_.mouseExited(args);
	}
}
void ManagedController::keyPressed(ofKeyEventArgs &args)
{
	if(is_mover_active_) {
		mover_.keyPressed(args);
	}
	if(is_divider_active_) {
		divider_.keyPressed(args);
	}
}
void ManagedController::keyReleased(ofKeyEventArgs &args)
{
	if(is_mover_active_) {
		mover_.keyReleased(args);
	}
	if(is_divider_active_) {
		divider_.keyReleased(args);
	}
}
