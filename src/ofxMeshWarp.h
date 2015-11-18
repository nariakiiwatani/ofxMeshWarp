#pragma once

#include "ofConstants.h"
#include "ofVec2f.h"
#include "ofVec3f.h"
#include "ofColor.h"

namespace ofx{namespace MeshWarp{
class MeshPoint
{
public:
	MeshPoint()
	:point_(0,0,0),coord_(0,0),normal_(0,0,1),color_(ofColor::white),is_node_(true)
	{}
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
	void setNode(bool set) { is_node_=set; }
	void toggleNode() { is_node_^=true; }
	
	void glPoint() {
		glColor4ubv(&color_[0]);
		glNormal3fv(&normal_[0]);
		glTexCoord2fv(&coord_[0]);
		glVertex3fv(&point_[0]);
	}
	void glArbPoint(const ofVec2f &uv_size) {
		glColor4ubv(&color_[0]);
		glNormal3fv(&normal_[0]);
		glTexCoord2fv(&(coord_*uv_size)[0]);
		glVertex3fv(&point_[0]);
	}

private:
	ofVec3f point_;
	ofVec2f coord_;
	ofVec3f normal_;
	ofColor color_;
	bool is_node_;
};


class Mesh
{
public:
	Mesh();
	~Mesh();
	void setup(int div_x, int div_y, float w, float h);
	void setTexCoordSize(float u, float v);
	void divideX(int pos);
	void divideY(int pos);
	void reset(float w, float h);
	void drawMesh();
	void drawWireframe();
	vector<MeshPoint*> getPoints();
	int getDivX() { return div_x_; }
	int getDivY() { return div_y_; }
	const ofVec2f& getTexCoordSize() { return uv_size_; }
private:
	int div_x_, div_y_;
	ofVec2f uv_size_;
	vector<int> indices_;
	vector<MeshPoint> mesh_;	
	int getIndex(int x, int y) const { return indices_[y*div_x_+x]; }
};
}}
using ofxMeshWarp = ofx::MeshWarp::Mesh;
using ofxMeshWarpPoint = ofx::MeshWarp::MeshPoint;
