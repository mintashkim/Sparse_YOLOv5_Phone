#pragma once

#include <Detect/Utils.h>

namespace Detect
{

struct Decision
{
	float fConf;
	int nArea;
	float fZScore;
	bool bExists;
};

class Decider
{
public:
	Decider(
		int nClassId,
		int nMinArea,
		float fConfThreshold,
		int nConfViewCount,
		float fZScoreThreshold,
		int nZScoreWindowSize,
		bool bCheckExists);
	virtual ~Decider(void);

	Decision decide(std::vector<Detection>& detections);

private:
	float _getZScore();

	int m_nClassId;
	int m_nMinArea;
	float m_fConfThreshold;
	int m_nConfViewCount;
	float m_fZScoreThreshold;
	int m_nZScoreWindowSize;

	bool m_bCheckExists;
	bool m_bLastExists;

	std::list<float> m_lstZScoreConfs;
	std::list<bool> m_lstExists;
};

}