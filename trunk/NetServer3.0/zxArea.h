#pragma once

#include <string>
#include <vector>

#include "Common.h"

class CzxArea
{
public:
	CzxArea(void);
	~CzxArea(void);

public:
	int m_nAreaID;
	char m_szAreaName[32];
	char m_szUpdateTime[64];
	std::vector<zxPosition> m_vctAreaPositions;
	double m_dSunnyLimitedSpeed;
	double m_dRainyLimitedSpeed;
	int m_nIsRun;
	int m_nMapID;
	int m_nTrafficCount;
};

