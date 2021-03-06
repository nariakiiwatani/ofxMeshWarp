#pragma once

#include "ofConstants.h"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "ofColor.h"
#include "ofMesh.h"
#include "ofRectangle.h"

namespace ofx{namespace MeshWarp{
class MeshPoint
{
public:
	static MeshPoint getLerped(const MeshPoint &a, const MeshPoint &b, float mix) {
		MeshPoint ret;
		ret.point_ = glm::mix(a.point_, b.point_, mix);
		ret.coord_ = glm::mix(a.coord_, b.coord_, mix);
		ret.normal_ = glm::mix(a.normal_, b.normal_, mix);
		ret.color_ = a.color_.getLerped(b.color_, mix);
		return ret;
	}
	const glm::vec3 &point() const { return point_; }
	const glm::vec2 &coord() const { return coord_; }
	const glm::vec3 &normal() const { return normal_; }
	const ofFloatColor &color() const { return color_; }
	float alpha() const { return color_.a; }
	bool isNode() const { return is_node_; }
	void setPoint(const glm::vec3 &point) { point_=point; }
	void setCoord(const glm::vec2 &coord) { coord_=coord; }
	void setNormal(const glm::vec3 &normal) { normal_=normal; }
	void setColor(const ofFloatColor &color) { color_=color; }
	void setAlpha(float alpha) { color_.a=alpha; }
	void setNodal(bool set) { is_node_=set; }
	void toggleNodal() { is_node_^=true; }

private:
	glm::vec3 point_ = glm::vec3(0,0,0);
	glm::vec2 coord_ = glm::vec2(0,0);
	glm::vec3 normal_ = glm::vec3(0,0,1);
	ofFloatColor color_ = ofFloatColor::white;
	bool is_node_ = true;
};


class Mesh
{
public:
	void setup(const ofRectangle &rect, int div_x, int div_y);
	void setDirty() { dirty_ = true; }
	bool isDirty() const { return dirty_; }
	void update();
	OF_DEPRECATED_MSG("use setUVRect instead", void setTexCoordSize(float u, float v));
	void setUVRect(const ofRectangle &uv);
	void setChildMeshResolution(int resolution);
	void divideCol(int pos, float ratio);
	void divideRow(int pos, float ratio);
	void reduceCol(int pos);
	void reduceRow(int pos);
	void reset(const ofRectangle &rect);
	void drawMesh() const;
	void drawWireframe() const;
	void drawDetailedWireframe() const;
	std::vector<MeshPoint*> getPoints();
	const ofMesh& getOfMesh() const { return of_mesh_; }
	int getDivX() const { return div_x_; }
	int getDivY() const { return div_y_; }
	const ofRectangle& getUVRect() const { return uv_rect_; }
	int getChildMeshResolution() const { return child_mesh_resolution_; }
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
	ofRectangle uv_rect_ = ofRectangle(0,0,1,1);
	std::vector<ofIndexType> indices_;
	std::vector<MeshPoint> mesh_;
	ofMesh of_mesh_;
	ofIndexType getIndex(int x, int y) const { return indices_[y*div_x_+x]; }
	int child_mesh_resolution_ = 8;
	void drawChildMesh() const;
	Mesh makeChildMesh(int x, int y, int resolution) const;
	struct {
		bool point=true, coord=true, normal=true, color=true;
	} interpolate_flags_;
	bool isEnabledAnyInterpolation() const {
		return interpolate_flags_.point
		|| interpolate_flags_.coord
		|| interpolate_flags_.normal
		|| interpolate_flags_.color;
	}
	std::vector<std::vector<Mesh>> child_meshes_;
	bool dirty_=true;
	void solve();
	void refreshChildMeshes();
};
}}
using ofxMeshWarp = ofx::MeshWarp::Mesh;
using ofxMeshWarpPoint = ofx::MeshWarp::MeshPoint;
