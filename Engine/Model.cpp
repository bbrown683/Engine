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

	for (unsigned int mesh = 0; mesh < scene->mNumMeshes; mesh++) {
		aiMesh* pMesh = scene->mMeshes[mesh];

		for (unsigned int vertex = 0; vertex < pMesh->mNumVertices; vertex++) {
            if (pMesh->HasPositions()) {
                aiVector3D aPosition = pMesh->mVertices[vertex];
            }

            if (pMesh->HasTextureCoords(0)) {
                aiVector3D aTexture = pMesh->mTextureCoords[0][vertex];
            }

            if (pMesh->HasNormals()) {
                aiVector3D aNormal = pMesh->mNormals[vertex];
            }

            if (pMesh->HasVertexColors(0)) {
                aiColor4D aColor = pMesh->mColors[0][vertex];
            }
		}
	}
	return true;
}
