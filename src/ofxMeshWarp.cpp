#include "ofxMeshWarp.h"
#include "ofTexture.h"

OFX_MESH_WARP_BEGIN_NAMESPACE
Mesh::Mesh()
{
}
Mesh::~Mesh()
{
}
void Mesh::setup(int div_x, int div_y, float w, float h)
{
	if(div_x < 1 || div_y < 1) {
		ofLogError("div_x and div_y must be bigger than 1");
		return;
	}
	div_x_ = div_x;
	div_y_ = div_y;
	w_ = w;
	h_ = h;
	mesh_.clear();
	for(int i = 0, num = div_x_*div_y_; i < num; ++i) {
		mesh_.push_back(MeshPoint());
	}
	reset();
}
void Mesh::divideX(int pos)
{
	if(pos >= div_x_-1) {
		ofLogError("cannot divide by bigger index than div_x_");
		return;
	}
	vector<int>indices = indices_;
	vector<int>::iterator it = indices.begin()+pos+1;
	for(int y = 0; y < div_y_; ++y) {
		MeshPoint &a = mesh_[getIndex(pos,y)];
		MeshPoint &b = mesh_[getIndex(pos+1,y)];
		MeshPoint point = MeshPoint(MeshPoint::getLerped(a, b, 0.5f));
		it = indices.insert(it, mesh_.size())+div_x_+1;
		mesh_.push_back(point);
	}
	indices_ = indices;
	++div_x_;
}
void Mesh::divideY(int pos)
{
	if(pos >= div_y_-1) {
		ofLogError("cannot divide by bigger index than div_y_");
		return;
	}
	vector<int>indices = indices_;
	vector<int>::iterator it = indices.begin()+(pos+1)*div_x_;
	for(int x = 0; x < div_x_; ++x) {
		MeshPoint &a = mesh_[getIndex(x,pos)];
		MeshPoint &b = mesh_[getIndex(x,pos+1)];
		MeshPoint point = MeshPoint(MeshPoint::getLerped(a, b, 0.5f));
		it = indices.insert(it, mesh_.size())+1;
		mesh_.push_back(point);
	}
	indices_ = indices;
	++div_y_;
}
void Mesh::reset()
{
	indices_.clear();
	for(int i = 0, num = div_x_*div_y_; i < num; ++i) {
		MeshPoint &point = mesh_[i];
		ofVec2f coord = ofVec2f(i%div_x_,i/div_x_)/ofVec2f(div_x_-1,div_y_-1);
		point.setCoord(coord);
		point.setPoint(coord*ofVec2f(w_,h_));
		indices_.push_back(i);
	}
}
vector<MeshPoint*> Mesh::getPoints()
{
	vector<MeshPoint*> ret;
	for(int y = 0; y < div_y_; ++y) {
		for(int x = 0; x < div_x_; ++x) {
			ret.push_back(&mesh_[getIndex(x,y)]);
		}
	}
	return ret;
}

void Mesh::drawMesh()
{
	glBegin(GL_TRIANGLES);
	if(ofGetUsingArbTex()) {
		for(int y = 0; y < div_y_-1; ++y) {
			for(int x = 0; x < div_x_-1; ++x) {
				mesh_[getIndex(x  ,y  )].glArbPoint(w_, h_);
				mesh_[getIndex(x  ,y+1)].glArbPoint(w_, h_);
				mesh_[getIndex(x+1,y  )].glArbPoint(w_, h_);
				
				mesh_[getIndex(x+1,y  )].glArbPoint(w_, h_);
				mesh_[getIndex(x  ,y+1)].glArbPoint(w_, h_);
				mesh_[getIndex(x+1,y+1)].glArbPoint(w_, h_);
			}
		}
	}
	else {
		for(int y = 0; y < div_y_-1; ++y) {
			for(int x = 0; x < div_x_-1; ++x) {
				mesh_[getIndex(x  ,y  )].glPoint();
				mesh_[getIndex(x  ,y+1)].glPoint();
				mesh_[getIndex(x+1,y  )].glPoint();
				
				mesh_[getIndex(x+1,y  )].glPoint();
				mesh_[getIndex(x  ,y+1)].glPoint();
				mesh_[getIndex(x+1,y+1)].glPoint();
			}
		}
	}
	glEnd();
}
void Mesh::drawWireframe()
{
	glBegin(GL_LINE_STRIP);
	if(ofGetUsingArbTex()) {
		for(int y = 0; y < div_y_-1; ++y) {
			for(int x = div_x_; --x>=0;) {
				mesh_[getIndex(x,y)].glArbPoint(w_, h_);
			}
			for(int x = 0; x < div_x_-1; ++x) {
				mesh_[getIndex(x,y+1)].glArbPoint(w_, h_);
				mesh_[getIndex(x+1,y)].glArbPoint(w_, h_);
			}
		}
		for(int x = div_x_; --x>=0;) {
			mesh_[getIndex(x,div_y_-1)].glArbPoint(w_, h_);
		}
	}
	else {
		for(int y = 0; y < div_y_-1; ++y) {
			for(int x = div_x_; --x>=0;) {
				mesh_[getIndex(x,y)].glPoint();
			}
			for(int x = 0; x < div_x_-1; ++x) {
				mesh_[getIndex(x,y+1)].glPoint();
				mesh_[getIndex(x+1,y)].glPoint();
			}
		}
		for(int x = div_x_; --x>=0;) {
			mesh_[getIndex(x,div_y_-1)].glPoint();
		}
	}
	glEnd();
}
OFX_MESH_WARP_END_NAMESPACE