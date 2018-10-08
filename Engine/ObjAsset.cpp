#include "ObjAsset.hpp"

#include "thirdparty/loguru/loguru.hpp"
#include "thirdparty/tinyobjloader/tiny_obj_loader.h"

ObjAsset::ObjAsset(Driver* pDriver) : Asset(pDriver) {}

bool ObjAsset::load(const char* pFilename) {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string error;
	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &error, pFilename))
		return false;
	if(!error.empty())
		LOG_F(WARNING, "%s - %s", pFilename, error.c_str());
	for (size_t s = 0; s < attrib.vertices.size() / 3; s++) {
		
	}
	return true;
}
