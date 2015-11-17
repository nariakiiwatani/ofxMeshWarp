#include "ofxMeshWarpIO.h"

using namespace ofx::MeshWarp;
using namespace ofx::MeshWarp::IO;

void PointHelper::get(ofBuffer &buffer) const
{
	PointData data;
	data.point = target_->point();
	data.coord = target_->coord();
	data.normal = target_->normal();
	data.color = target_->color();
	buffer.append((const char*)(&data), sizeof(PointData));
}
int PointHelper::set(const char *buffer)
{
	PointData *data = (PointData*)buffer;
	target_->setPoint(data->point);
	target_->setCoord(data->coord);
	target_->setNormal(data->normal);
	target_->setColor(data->color);
	return sizeof(PointData);
}
void MeshHelper::get(ofBuffer &buffer) const
{
	MeshData data;
	data.divx = target_->getDivX();
	data.divy = target_->getDivY();
	data.uv_size = target_->getTexCoordSize();
	buffer.append((const char*)(&data), sizeof(MeshData));
}
int MeshHelper::set(const char *buffer)
{
	MeshData *data = (MeshData*)buffer;
	target_->setup(data->divx, data->divy, 1, 1);
	target_->setTexCoordSize(data->uv_size[0], data->uv_size[1]);
	return sizeof(MeshData);
}
Saver::Saver(Mesh *mesh, string filename)
{
	ofBuffer buffer;
	MeshHelper(mesh).get(buffer);
	vector<MeshPoint*> points = mesh->getPoints();
	for(auto &p : points) {
		PointHelper(p).get(buffer);
	}
	ofBufferToFile(filename, buffer, true);
}
Loader::Loader(Mesh *mesh, string filename)
{
	ofBuffer buffer = ofBufferFromFile(filename, true);
	const char *ptr = buffer.getData();
	ptr += MeshHelper(mesh).set(ptr);
	vector<MeshPoint*> points = mesh->getPoints();
	for(auto &p : points) {
		ptr += PointHelper(p).set(ptr);
	}
}

