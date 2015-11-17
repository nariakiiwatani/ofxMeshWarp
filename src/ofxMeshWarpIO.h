#pragma once

#include "ofxMeshWarpDef.h"
#include "ofxMeshWarp.h"
#include "ofFileUtils.h"

OFX_MESH_WARP_BEGIN_NAMESPACE

namespace io {
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

OFX_MESH_WARP_END_NAMESPACE

using ofxMeshWarpSave = ofx::MeshWarp::io::Saver;
using ofxMeshWarpLoad = ofx::MeshWarp::io::Loader;
