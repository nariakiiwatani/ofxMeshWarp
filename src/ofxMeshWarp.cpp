#include "ofxMeshWarp.h"
#include "ofTexture.h"
#include <numeric>

using namespace ofx::MeshWarp;
using namespace std;

void Mesh::setup(int div_x, int div_y, float w, float h)
{
	if(div_x < 1 || div_y < 1) {
		ofLogError(__FILE__, "div_x and div_y must be bigger than 1");
		return;
	}
	div_x_ = div_x;
	div_y_ = div_y;
	reset(w,h);
	of_mesh_.setMode(OF_PRIMITIVE_TRIANGLES);
}
void Mesh::update()
{
	if(dirty_) {
		solve();
	}
}
void Mesh::setTexCoordSize(float u, float v)
{
	setUVRect(ofRectangle(0,0,u,v));
}
void Mesh::setUVRect(const ofRectangle &rect)
{
	uv_rect_ = rect;
	setDirty();
}
void Mesh::setChildMeshResolution(int resolution)
{
	child_mesh_resolution_ = resolution;
	setDirty();
}
void Mesh::divideCol(int pos, float ratio)
{
	if(pos < 0 || div_x_-1 <= pos) {
		ofLogError(__FILE__, "index out of bounds: %d", pos);
		return;
	}
    vector<ofIndexType>indices = indices_;
    for (int y = 0; y < div_y_; ++y) {
        // insert indices
        auto it = begin(indices) + (y*div_x_) + (pos + 1) + y;    // the index position where to insert
        indices.insert(it, (ofIndexType)mesh_.size())+1;

        // push point into mesh
        MeshPoint &a = mesh_[getIndex(pos, y)];                 // the index before the one we want to insert. on the left
        MeshPoint &b = mesh_[getIndex(pos + 1, y)];             // the index after the one we want to insert. on the right
        MeshPoint point = MeshPoint(MeshPoint::getLerped(a, b, ratio)); // find the location of the point, based on ratio
        mesh_.push_back(point);
    }
	indices_ = indices;
	++div_x_;
	setDirty();
}
void Mesh::divideRow(int pos, float ratio)
{
	if(pos < 0 || div_y_-1 <= pos) {
		ofLogError(__FILE__, "index out of bounds: %d", pos);
		return;
	}
	auto indices = indices_;
	auto it = begin(indices)+(pos+1)*div_x_;
	for(int x = 0; x < div_x_; ++x) {
		MeshPoint &a = mesh_[getIndex(x,pos)];
		MeshPoint &b = mesh_[getIndex(x,pos+1)];
		MeshPoint point = MeshPoint(MeshPoint::getLerped(a, b, ratio));
		it = indices.insert(it, (ofIndexType)mesh_.size())+1;
		mesh_.push_back(point);
	}
	indices_ = indices;
	++div_y_;
	setDirty();
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
		indices_.erase(begin(indices_)+index);
	}
	--div_x_;
	setDirty();
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
	auto from = begin(indices_)+pos*div_x_;
	auto to = from+div_x_;
	indices_.erase(from, to);
	--div_y_;
	setDirty();
}
void Mesh::reset(float w, float h)
{
	int num = div_x_*div_y_;
	mesh_.resize(num);
	for(int i = 0; i < num; ++i) {
		MeshPoint &point = mesh_[i];
		glm::vec2 coord = glm::vec2(i%div_x_,i/div_x_)/glm::vec2(div_x_-1,div_y_-1);
		point.setCoord(coord);
		point.setPoint(glm::vec3(coord*glm::vec2(w,h), 0));
	}
	indices_.resize(num);
	iota(begin(indices_), end(indices_), 0);
	setDirty();
}
void Mesh::solve()
{
	if(!isEnabledAnyInterpolation()) {
		return;
	}
	auto solveFlameX = [&](int y) {
		vector<MeshPoint*> work;
		work.clear();
		work.push_back(&mesh_[getIndex(0,y)]);
		for(int x = 1; x < div_x_; ++x) {
			MeshPoint *p1 = &mesh_[getIndex(x,y)];
			if(p1->isNode() || x == div_x_-1) {
				if(work.size() > 1) {
					MeshPoint *p0 = work[0];
					for(size_t w = 1, num = work.size(); w < num; ++w) {
						MeshPoint lerped = MeshPoint::getLerped(*p0, *p1, w/(float)num);
						if(isEnabledPointInterpolation()) work[w]->setPoint(lerped.point());
						if(isEnabledCoordInterpolation()) work[w]->setCoord(lerped.coord());
						if(isEnabledNormalInterpolation()) work[w]->setNormal(lerped.normal());
						if(isEnabledColorInterpolation()) work[w]->setColor(lerped.color());
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
					for(size_t w = 1, num = work.size(); w < num; ++w) {
						MeshPoint lerped = MeshPoint::getLerped(*p0, *p1, w/(float)num);
						if(isEnabledPointInterpolation()) work[w]->setPoint(lerped.point());
						if(isEnabledCoordInterpolation()) work[w]->setCoord(lerped.coord());
						if(isEnabledNormalInterpolation()) work[w]->setNormal(lerped.normal());
						if(isEnabledColorInterpolation()) work[w]->setColor(lerped.color());
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
		if(isEnabledPointInterpolation()) {
			glm::vec3 point = [&around,&ratio](){glm::vec3 ret;for(auto &p:around)ret+=p.first->point()*ratio(p.second);return ret;}();
			dst.setPoint(point);
		}
		if(isEnabledCoordInterpolation()) {
			glm::vec2 coord = [&around,&ratio](){glm::vec2 ret;for(auto &p:around)ret+=p.first->coord()*ratio(p.second);return ret;}();
			dst.setCoord(coord);
		}
		if(isEnabledNormalInterpolation()) {
			glm::vec3 normal = [&around,&ratio](){glm::vec3 ret;for(auto &p:around)ret+=p.first->normal()*ratio(p.second);return ret;}();
			dst.setNormal(normal);
		}
		if(isEnabledColorInterpolation()) {
			ofVec4f color = [&around,&ratio]() {
				ofVec4f ret;
				for(auto &p:around) {
					auto color = p.first->color();
					ret += ofVec4f(color[0], color[1], color[2], color[3])*ratio(p.second);
				}
				return ret;
			}();
			dst.setColor(ofFloatColor(color[0],color[1],color[2],color[3]));
		}
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
	if(child_mesh_resolution_ > 1) {
		refreshChildMeshes();
	}
	
	of_mesh_.clear();
	for(auto &p :mesh_) {
		glm::vec2 coord = p.coord();
		coord.x =ofMap(coord.x, 0, 1, uv_rect_.getLeft(), uv_rect_.getRight());
		coord.y =ofMap(coord.y, 0, 1, uv_rect_.getTop(), uv_rect_.getBottom());
		of_mesh_.addTexCoord(coord);
		of_mesh_.addColor(p.color());
		of_mesh_.addNormal(p.normal());
		of_mesh_.addVertex(p.point());
	}
	for(int y = 0; y < div_y_-1; ++y) {
		for(int x = 0; x < div_x_-1; ++x) {
			of_mesh_.addIndex(getIndex(x  ,y  ));
			of_mesh_.addIndex(getIndex(x  ,y+1));
			of_mesh_.addIndex(getIndex(x+1,y  ));
			of_mesh_.addIndex(getIndex(x+1,y  ));
			of_mesh_.addIndex(getIndex(x  ,y+1));
			of_mesh_.addIndex(getIndex(x+1,y+1));
		}
	}

	dirty_ = false;
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
	iota(begin(indices_), end(indices_), 0);
	dirty_ = true;
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

void Mesh::drawMesh() const
{
	if(child_mesh_resolution_ > 1) {
		drawChildMesh();
	}
	else {
		of_mesh_.draw();
	}
}

Mesh Mesh::makeChildMesh(int x, int y, int resolution) const
{
	Mesh mesh;
	mesh.setChildMeshResolution(1);
	mesh.setUVRect(uv_rect_);
	mesh.setup(resolution, resolution, 1,1);
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
void Mesh::drawChildMesh() const
{
	if(child_mesh_resolution_ < 2) {
		ofLogWarning(__FILE__, "child mesh resolution should be more than 2: %d", child_mesh_resolution_);
		return;
	}
	for(int x = 0; x < div_x_-1; ++x) {
		for(int y = 0; y < div_y_-1; ++y) {
			child_meshes_[x][y].drawMesh();
		}
	}
}
void Mesh::drawWireframe() const
{
	for(int y = 0; y < div_y_; ++y) {
		glBegin(GL_LINE_STRIP);
		for(int x = 0; x < div_x_; ++x) {
			glm::vec3 pos = mesh_[getIndex(x,y)].point();
			glVertex3fv(&pos[0]);
		}
		glEnd();
	}
	for(int x = 0; x < div_x_; ++x) {
		glBegin(GL_LINE_STRIP);
		for(int y = 0; y < div_y_; ++y) {
			glm::vec3 pos = mesh_[getIndex(x,y)].point();
			glVertex3fv(&pos[0]);
		}
		glEnd();
	}
}
void Mesh::drawDetailedWireframe() const
{
	if(child_mesh_resolution_ > 1) {
		for(int x = 0; x < div_x_-1; ++x) {
			for(int y = 0; y < div_y_-1; ++y) {
				child_meshes_[x][y].drawWireframe();
			}
		}
	}
	else {
		drawWireframe();
	}
}

void Mesh::refreshChildMeshes()
{
	child_meshes_.resize(div_x_-1);
	for(int x = 0; x < div_x_-1; ++x) {
		child_meshes_[x].resize(div_y_-1);
		for(int y = 0; y < div_y_-1; ++y) {
			child_meshes_[x][y] = makeChildMesh(x, y, child_mesh_resolution_);
		}
	}
}

