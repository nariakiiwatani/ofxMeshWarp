#pragma once

#include "ofConstants.h"
#include "ofVec2f.h"
#include "ofVec3f.h"
#include "ofColor.h"
#include "ofMesh.h"

namespace ofx{namespace MeshWarp{
class MeshPoint
{
public:
	static MeshPoint getLerped(const MeshPoint &a, const MeshPoint &b, float mix) {
		MeshPoint ret;
		ret.point_ = a.point_.getInterpolated(b.point_, mix);
		ret.coord_ = a.coord_.getInterpolated(b.coord_, mix);
		ret.normal_ = a.normal_.getInterpolated(b.normal_, mix);
		ret.color_ = a.color_.getLerped(b.color_, mix);
		return ret;
	}
	const ofVec3f &point() const { return point_; }
	const ofVec2f &coord() const { return coord_; }
	const ofVec3f &normal() const { return normal_; }
	const ofColor &color() const { return color_; }
	bool isNode() const { return is_node_; }
	void setPoint(const ofVec3f &point) { point_=point; }
	void setCoord(const ofVec2f &coord) { coord_=coord; }
	void setNormal(const ofVec3f &normal) { normal_=normal; }
	void setColor(const ofColor &color) { color_=color; }
	void setNodal(bool set) { is_node_=set; }
	void toggleNodal() { is_node_^=true; }

private:
	ofVec3f point_ = ofVec3f(0,0,0);
	ofVec2f coord_ = ofVec2f(0,0);
	ofVec3f normal_ = ofVec3f(0,0,1);
	ofColor color_ = ofColor::white;
	bool is_node_ = true;
};


class Mesh
{
public:
	void setup(int div_x, int div_y, float w, float h);
	void setDirty() { dirty_ = true; }
	void update();
	void setTexCoordSize(float u, float v);
	void setChildMeshResolution(int resolution);
	void divideCol(int pos, float ratio);
	void divideRow(int pos, float ratio);
	void reduceCol(int pos);
	void reduceRow(int pos);
	void reset(float w, float h);
	void drawMesh();
	void drawWireframe();
	void drawDetailedWireframe();
	vector<MeshPoint*> getPoints();
	int getDivX() { return div_x_; }
	int getDivY() { return div_y_; }
	const ofVec2f& getTexCoordSize() { return uv_size_; }
	int getChildMeshResolution() { return child_mesh_resolution_; }
	void gc();
	void setEnablePointInterpolation(bool set) { interpolate_flags_.point=set; }
	void setEnableCoordInterpolation(bool set) { interpolate_flags_.coord=set; }
	void setEnableNormalInterpolation(bool set) { interpolate_flags_.normal=set; }
	void setEnableColorInterpolation(bool set) { interpolate_flags_.color=set; }
	bool isEnabledPointInterpolation() const { return interpolate_flags_.point; }
	bool isEnabledCoordInterpolation() const { return interpolate_flags_.coord; }
	bool isEnabledNormalInterpolation() const { return interpolate_flags_.normal; }
	bool isEnabledColorInterpolation() const { return interpolate_flags_.color; }
private:
	int div_x_, div_y_;
	ofVec2f uv_size_ = ofVec2f(1,1);
	vector<int> indices_;
	vector<MeshPoint> mesh_;
	ofMesh of_mesh_;
	int getIndex(int x, int y) const { return indices_[y*div_x_+x]; }
	int child_mesh_resolution_ = 8;
	void drawChildMesh();
	Mesh makeChildMesh(int x, int y, int resolution);
	struct {
		bool point=true, coord=true, normal=true, color=true;
	} interpolate_flags_;
	bool isEnabledAnyInterpolation() const {
		return interpolate_flags_.point
		|| interpolate_flags_.coord
		|| interpolate_flags_.normal
		|| interpolate_flags_.color;
	}
	vector<vector<Mesh>> child_meshes_;
	bool dirty_=true;
	void solve();
	void refreshChildMeshes();
};
}}
using ofxMeshWarp = ofx::MeshWarp::Mesh;
using ofxMeshWarpPoint = ofx::MeshWarp::MeshPoint;
