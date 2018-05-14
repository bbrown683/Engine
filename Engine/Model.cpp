#include "Model.hpp"

bool Model::load(const char* filename) {
	const aiScene* scene = m_Importer.ReadFile(filename, aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_GenUVCoords |
		aiProcess_GenNormals |
		aiProcess_FlipUVs);

	if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		return false;

	for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
		aiMesh* aMesh = scene->mMeshes[i];

		for (unsigned int j = 0; j < aMesh->mNumVertices; j++) {
				
		}
	}
	return true;
}
