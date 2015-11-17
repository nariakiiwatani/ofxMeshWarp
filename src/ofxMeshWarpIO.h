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
			ofColor color;
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
			int divx, divy;
			ofVec2f uv_size;
		};
	};
	class Saver {
	public:
		Saver(Mesh *mesh, string filename);
	};
	class Loader {
	public:
		Loader(Mesh *mesh, string filename);
	};
}

}}
using ofxMeshWarpSave = ofx::MeshWarp::IO::Saver;
using ofxMeshWarpLoad = ofx::MeshWarp::IO::Loader;
