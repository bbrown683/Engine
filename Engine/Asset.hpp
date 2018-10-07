#pragma once

class Driver;
class Asset {
public:
	explicit Asset(Driver* pDriver);
	virtual ~Asset();
	virtual bool load(const char* pFilename) = 0;
protected:
	Driver* getDriver() const;
private:
	Driver* m_pDriver;
};