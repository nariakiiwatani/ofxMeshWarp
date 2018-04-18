#pragma once

#include "ofxMeshWarp.h"
#include "ofFileUtils.h"

namespace ofx{namespace MeshWarp{
namespace IO {
	class PointHelper {
	public:
		PointHelper(MeshPoint *p):target_(p){}
		void get(ofBuffer &buffer) const;
		int set(const char *buffer);
	private:
		MeshPoint *target_;
		struct PointData {
			ofVec3f point;
			ofVec2f coord;
			ofVec3f normal;
			ofFloatColor color;
			bool is_node;
		};
	};
	class MeshHelper {
	public:
		MeshHelper(Mesh *m):target_(m){}
		void get(ofBuffer &buffer) const;
		int set(const char *buffer);
	private:
		Mesh *target_;
		struct MeshData {
			int divx, divy, resolution;
			ofRectangle uv_rect;
		};
	};
	class Saver {
	public:
		void addMesh(Mesh *mesh);
		void addMeshes(vector<Mesh*> &mesh);
		void save(const string &filename) const;
		void save(ofBuffer &buffer) const;
	private:
		vector<Mesh*> meshes_;
	};
	class Loader {
	public:
		vector<Mesh*> load(const string &filename) const;
		vector<Mesh*> load(const ofBuffer &buffer) const;
		vector<Mesh*> load(const char *data, size_t size) const;
	};
}

}}
using ofxMeshWarpSave = ofx::MeshWarp::IO::Saver;
using ofxMeshWarpLoad = ofx::MeshWarp::IO::Loader;
