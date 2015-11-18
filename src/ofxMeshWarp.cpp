#include "ofxMeshWarp.h"
#include "ofTexture.h"

using namespace ofx::MeshWarp;
Mesh::Mesh()
:uv_size_(1,1)
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
	mesh_.clear();
	for(int i = 0, num = div_x_*div_y_; i < num; ++i) {
		mesh_.push_back(MeshPoint());
	}
	reset(w,h);
}
void Mesh::setTexCoordSize(float u, float v)
{
	uv_size_.set(u,v);
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
void Mesh::reset(float w, float h)
{
	indices_.clear();
	for(int i = 0, num = div_x_*div_y_; i < num; ++i) {
		MeshPoint &point = mesh_[i];
		ofVec2f coord = ofVec2f(i%div_x_,i/div_x_)/ofVec2f(div_x_-1,div_y_-1);
		point.setCoord(coord);
		point.setPoint(coord*ofVec2f(w,h));
		indices_.push_back(i);
	}
}
void Mesh::solve()
{
	// 枠上を決定
	auto solveFlameX = [&](int y) {
		vector<MeshPoint*> work;
		work.clear();
		work.push_back(&mesh_[getIndex(0,y)]);
		for(int x = 1; x < div_x_; ++x) {
			MeshPoint *p1 = &mesh_[getIndex(x,y)];
			if(p1->isNode() || x == div_x_-1) {
				if(work.size() > 1) {
					MeshPoint *p0 = work[0];
					for(int w = 1, num = work.size(); w < num; ++w) {
						*work[w] = MeshPoint::getLerped(*p0, *p1, w/(float)num);
						work[w]->setNode(false);
					}
				}
				work.clear();
			}
			work.push_back(p1);
		}
	};
	auto solveFlameY = [&](int x) {
		vector<MeshPoint*> work;
		work.clear();
		work.push_back(&mesh_[getIndex(x,0)]);
		for(int y = 1; y < div_y_; ++y) {
			MeshPoint *p1 = &mesh_[getIndex(x,y)];
			if(p1->isNode() || y == div_y_-1) {
				if(work.size() > 1) {
					MeshPoint *p0 = work[0];
					for(int w = 1, num = work.size(); w < num; ++w) {
						*work[w] = MeshPoint::getLerped(*p0, *p1, w/(float)num);
						work[w]->setNode(false);
					}
				}
				work.clear();
			}
			work.push_back(p1);
		}
	};
	auto solveInternal = [&](int x, int y) {
		auto getDistanceToNode = [&](int x, int y, int diff_x, int diff_y) ->int {
			int ret = 0;
			while(0 < x && x < div_x_-1 && 0 < y && y < div_y_-1) {
				x += diff_x; y += diff_y;
				++ret;
				if(mesh_[getIndex(x,y)].isNode()) break;
			}
			return ret;
		};
		int to_u = getDistanceToNode(x,y,0,-1);
		int to_d = getDistanceToNode(x,y,0, 1);
		int to_l = getDistanceToNode(x,y,-1,0);
		int to_r = getDistanceToNode(x,y, 1,0);
		auto getRatio = [&](int self) ->float {
			float inv_sum = 1/(float)to_u+1/(float)to_d+1/(float)to_l+1/(float)to_r;
			return ofMap(1/(float)self, 0, inv_sum, 0, 1, false);
		};
		float r_u = getRatio(to_u);
		float r_d = getRatio(to_d);
		float r_l = getRatio(to_l);
		float r_r = getRatio(to_r);
		
		MeshPoint &u = mesh_[getIndex(x,y-to_u)];
		MeshPoint &d = mesh_[getIndex(x,y+to_d)];
		MeshPoint &l = mesh_[getIndex(x-to_l,y)];
		MeshPoint &r = mesh_[getIndex(x+to_r,y)];
		MeshPoint &dst = mesh_[getIndex(x,y)];
		dst.setPoint(u.point()*r_u+d.point()*r_d+l.point()*r_l+r.point()*r_r);
		dst.setCoord(u.coord()*r_u+d.coord()*r_d+l.coord()*r_l+r.coord()*r_r);
		dst.setNormal(u.normal()*r_u+d.normal()*r_d+l.normal()*r_l+r.normal()*r_r);
		dst.setColor(u.color()*r_u+d.color()*r_d+l.color()*r_l+r.color()*r_r);
	};
	solveFlameX(0);
	solveFlameX(div_y_-1);
	solveFlameY(0);
	solveFlameY(div_x_-1);
	for(int y = 1; y < div_y_-1; ++y) {
		for(int x = 1; x < div_x_-1; ++x) {
			if(!mesh_[getIndex(x,y)].isNode()) {
				solveInternal(x,y);
			}
		}
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
				mesh_[getIndex(x  ,y  )].glArbPoint(uv_size_);
				mesh_[getIndex(x  ,y+1)].glArbPoint(uv_size_);
				mesh_[getIndex(x+1,y  )].glArbPoint(uv_size_);
				
				mesh_[getIndex(x+1,y  )].glArbPoint(uv_size_);
				mesh_[getIndex(x  ,y+1)].glArbPoint(uv_size_);
				mesh_[getIndex(x+1,y+1)].glArbPoint(uv_size_);
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
				mesh_[getIndex(x,y)].glArbPoint(uv_size_);
			}
			for(int x = 0; x < div_x_-1; ++x) {
				mesh_[getIndex(x,y+1)].glArbPoint(uv_size_);
				mesh_[getIndex(x+1,y)].glArbPoint(uv_size_);
			}
		}
		for(int x = div_x_; --x>=0;) {
			mesh_[getIndex(x,div_y_-1)].glArbPoint(uv_size_);
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
