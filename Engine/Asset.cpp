#include "Asset.hpp"
#include "Driver.hpp"

Asset::Asset(Driver* pDriver) : m_pDriver(pDriver) {}

Asset::~Asset() {}

Driver* Asset::getDriver() const {
	return m_pDriver;
}
