#pragma once

#include "Image.hpp"

enum class PBRTextureType {
	Color,
	Displacement,
	Metallic,
	Normal,
	Roughness,
};

struct PBRMaterial {
public:

private:
	Image m_Color;
	Image m_Displacement;
	Image m_Metallic;
	Image m_Normal;
	Image m_Roughness;
};