/*
MIT License

Copyright (c) 2018 Ben Brown

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

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
