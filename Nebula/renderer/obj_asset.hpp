#pragma once

#include "asset.hpp"

class ObjAsset : public Asset {
public:
	ObjAsset(Driver* pDriver);
	virtual bool load(const char* pFilename) override;
};