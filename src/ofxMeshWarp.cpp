#include "ofxMeshWarp.h"
#include "ofTexture.h"

using namespace ofx::MeshWarp;

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
		point.setNodal(false);
		indices_.push_back(i);
	}
	mesh_[getIndex(0,0)].setNodal(true);
	mesh_[getIndex(div_x_-1,0)].setNodal(true);
	mesh_[getIndex(0,div_y_-1)].setNodal(true);
	mesh_[getIndex(div_x_-1,div_y_-1)].setNodal(true);
}
void Mesh::solve()
{
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
						work[w]->setNodal(false);
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
						work[w]->setNodal(false);
					}
				}
				work.clear();
			}
			work.push_back(p1);
		}
	};
	auto solveInternal = [&](int x, int y) {
		assert(0 < x && x < div_x_-1 && 0 < y && y < div_y_-1);
		auto getDistanceToNode = [&](int x, int y, int diff_x, int diff_y) mutable ->pair<MeshPoint*,int> {
			int ret = 0;
			while(0 < x && x < div_x_-1 && 0 < y && y < div_y_-1) {
				x += diff_x; y += diff_y;
				++ret;
				if(mesh_[getIndex(x,y)].isNode()) break;
			}
			assert(ret>0);
			return make_pair(&mesh_[getIndex(x,y)],ret);
		};
		map<MeshPoint*,int> around{
			getDistanceToNode(x,y, -1,-1),
			getDistanceToNode(x,y, -1, 0),
			getDistanceToNode(x,y, -1, 1),
			getDistanceToNode(x,y,  1,-1),
			getDistanceToNode(x,y,  1, 0),
			getDistanceToNode(x,y,  1,+1),
			getDistanceToNode(x,y,  0,-1),
			getDistanceToNode(x,y,  0,+1),
		};
		float inv_sum = [&around]()->float{float ret=0;for(auto &p:around)ret+=1/(float)p.second;return ret;}();
		auto ratio = [&around,inv_sum](int self)->float {
			return ofMap(1/(float)self, 0, inv_sum, 0, 1, false);
		};
		MeshPoint &dst = mesh_[getIndex(x,y)];
		ofVec3f point = [&around,&ratio](){ofVec3f ret;for(auto &p:around)ret+=p.first->point()*ratio(p.second);return ret;}();
		ofVec2f coord = [&around,&ratio](){ofVec2f ret;for(auto &p:around)ret+=p.first->coord()*ratio(p.second);return ret;}();
		ofVec3f normal = [&around,&ratio](){ofVec3f ret;for(auto &p:around)ret+=p.first->normal()*ratio(p.second);return ret;}();
		ofColor color = [&around,&ratio](){ofColor ret;for(auto &p:around)ret+=p.first->color()*ratio(p.second);return ret;}();
		dst.setPoint(point);
		dst.setCoord(coord);
		dst.setNormal(normal);
		dst.setColor(color);
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
