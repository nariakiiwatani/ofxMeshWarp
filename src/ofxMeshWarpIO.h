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
			glm::vec3 point;
			glm::vec2 coord;
			glm::vec3 normal;
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
			ofVec4f uv_rect;
		};
	};
	class Saver {
	public:
		void addMesh(std::shared_ptr<Mesh> mesh);
		void addMeshes(std::vector<std::shared_ptr<Mesh>> mesh);
		void save(const std::string &filename) const;
		void save(ofBuffer &buffer) const;
	private:
		std::vector<std::shared_ptr<Mesh>> meshes_;
	};
	class Loader {
	public:
		std::vector<std::shared_ptr<Mesh>> load(const std::string &filename) const;
		std::vector<std::shared_ptr<Mesh>> load(const ofBuffer &buffer) const;
		std::vector<std::shared_ptr<Mesh>> load(const char *data, std::size_t size) const;
	};
}

}}
using ofxMeshWarpSave = ofx::MeshWarp::IO::Saver;
using ofxMeshWarpLoad = ofx::MeshWarp::IO::Loader;
