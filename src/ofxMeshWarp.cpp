#include "ofxMeshWarp.h"
#include "ofTexture.h"
#include <numeric>

using namespace ofx::MeshWarp;

void Mesh::setup(int div_x, int div_y, float w, float h)
{
	if(div_x < 1 || div_y < 1) {
		ofLogError(__FILE__, "div_x and div_y must be bigger than 1");
		return;
	}
	div_x_ = div_x;
	div_y_ = div_y;
	reset(w,h);
}
void Mesh::setTexCoordSize(float u, float v)
{
	uv_size_.set(u,v);
}

void Mesh::divideCol(int pos, float ratio)
{
	if(pos < 0 || div_x_-1 <= pos) {
		ofLogError(__FILE__, "index out of bounds: %d", pos);
		return;
	}
	vector<int>indices = indices_;
	vector<int>::iterator it = indices.begin()+pos+1;
	for(int y = 0; y < div_y_; ++y) {
		MeshPoint &a = mesh_[getIndex(pos,y)];
		MeshPoint &b = mesh_[getIndex(pos+1,y)];
		MeshPoint point = MeshPoint(MeshPoint::getLerped(a, b, ratio));
		it = indices.insert(it, mesh_.size())+div_x_+1;
		mesh_.push_back(point);
	}
	indices_ = indices;
	++div_x_;
}
void Mesh::divideRow(int pos, float ratio)
{
	if(pos < 0 || div_x_-1 <= pos) {
		ofLogError(__FILE__, "index out of bounds: %d", pos);
		return;
	}
	vector<int>indices = indices_;
	vector<int>::iterator it = indices.begin()+(pos+1)*div_x_;
	for(int x = 0; x < div_x_; ++x) {
		MeshPoint &a = mesh_[getIndex(x,pos)];
		MeshPoint &b = mesh_[getIndex(x,pos+1)];
		MeshPoint point = MeshPoint(MeshPoint::getLerped(a, b, ratio));
		it = indices.insert(it, mesh_.size())+1;
		mesh_.push_back(point);
	}
	indices_ = indices;
	++div_y_;
}
void Mesh::reduceCol(int pos)
{
	if(div_x_ <= 2) {
		ofLogError(__FILE__, "cannot reduce col anymore.");
		return;
	}
	if(pos < 0 || div_x_ <= pos) {
		ofLogError(__FILE__, "index out of bounds: %d", pos);
		return;
	}
	for(int i = div_y_; --i >= 0;) {
		int index = i*div_x_+pos;
		indices_.erase(indices_.begin()+index);
	}
	--div_x_;
}
void Mesh::reduceRow(int pos)
{
	if(div_y_ <= 2) {
		ofLogError(__FILE__, "cannot reduce row anymore.");
		return;
	}
	if(pos < 0 || div_y_ <= pos) {
		ofLogError(__FILE__, "index out of bounds: %d", pos);
		return;
	}
	auto from = indices_.begin()+pos*div_x_;
	auto to = from+div_x_;
	indices_.erase(from, to);
	--div_y_;
}
void Mesh::reset(float w, float h)
{
	int num = div_x_*div_y_;
	mesh_.resize(num);
	for(int i = 0; i < num; ++i) {
		MeshPoint &point = mesh_[i];
		ofVec2f coord = ofVec2f(i%div_x_,i/div_x_)/ofVec2f(div_x_-1,div_y_-1);
		point.setCoord(coord);
		point.setPoint(coord*ofVec2f(w,h));
	}
	indices_.resize(num);
	iota(indices_.begin(),indices_.end(), 0);
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

void Mesh::gc()
{
	if(mesh_.size() == indices_.size()) {
		ofLogNotice(__FILE__, "no need for gc-ing");
		return;
	}
	vector<MeshPoint> neu;
	for(auto &i : indices_) {
		neu.push_back(mesh_[i]);
	}
	neu.swap(mesh_);
	iota(indices_.begin(),indices_.end(), 0);
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
	if(child_mesh_resolution_ > 1) {
		drawChildMesh();
	}
	else {
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
}

Mesh Mesh::makeChildMesh(int x, int y, int resolution)
{
	Mesh mesh;
	mesh.setup(resolution, resolution, 1,1);
	mesh.setTexCoordSize(uv_size_.x, uv_size_.y);
	mesh.setChildMeshResolution(1);
	auto isCorner = [resolution](int x, int y) {
		return (x==0&&y==0)||(x==resolution-1&&y==resolution-1);
	};
	mesh.mesh_[mesh.getIndex(0,0)] = mesh_[getIndex(x,y)];
	mesh.mesh_[mesh.getIndex(resolution-1,0)] = mesh_[getIndex(x+1,y)];
	mesh.mesh_[mesh.getIndex(0,resolution-1)] = mesh_[getIndex(x,y+1)];
	mesh.mesh_[mesh.getIndex(resolution-1,resolution-1)] = mesh_[getIndex(x+1,y+1)];
	for(int iy = 0; iy < resolution; ++iy) {
		for(int ix = 0; ix < resolution; ++ix) {
			mesh.mesh_[mesh.getIndex(ix,iy)].setNodal(isCorner(ix,iy));
		}
	}
	mesh.solve();
	return mesh;
}
void Mesh::drawChildMesh()
{
	if(child_mesh_resolution_ < 2) {
		ofLogWarning(__FILE__, "child mesh resolution should be more than 2: %d", child_mesh_resolution_);
		return;
	}
	for(int y = 0; y < div_y_-1; ++y) {
		for(int x = 0; x < div_x_-1; ++x) {
			Mesh &&mesh = makeChildMesh(x, y, child_mesh_resolution_);
			mesh.drawMesh();
		}
	}
}
void Mesh::drawWireframe()
{
	for(int y = 0; y < div_y_; ++y) {
		glBegin(GL_LINE_STRIP);
		for(int x = 0; x < div_x_; ++x) {
			mesh_[getIndex(x,y)].glPoint();
		}
		glEnd();
	}
	for(int x = 0; x < div_x_; ++x) {
		glBegin(GL_LINE_STRIP);
		for(int y = 0; y < div_y_; ++y) {
			mesh_[getIndex(x,y)].glPoint();
		}
		glEnd();
	}
}
