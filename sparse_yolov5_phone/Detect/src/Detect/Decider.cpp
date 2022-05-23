#include "pch.h"
#include <Detect/Decider.h>

#include <numeric>

namespace Detect
{

Decider::Decider(
    int nClassId,
    int nMinArea,
    float fConfThreshold,
    int nConfViewCount,
    float fZScoreThreshold,
    int nZScoreWindowSize,
    bool bCheckExists) :
    m_nClassId(nClassId),
    m_nMinArea(nMinArea),
    m_fConfThreshold(fConfThreshold),
    m_nConfViewCount(nConfViewCount),
    m_fZScoreThreshold(fZScoreThreshold),
    m_nZScoreWindowSize(nZScoreWindowSize),
    m_bCheckExists(bCheckExists)
{
    m_bLastExists = m_bCheckExists;
}

Decider::~Decider(void)
{
}

Decision Decider::decide(std::vector<Detection>& detections)
{
    Decision decision;

    decision.fConf = 0.001f;
    decision.nArea = 0;
    for (const Detection& detection : detections)
    {
        if (m_nClassId != detection.classId ||
            detection.box.area() < m_nMinArea)
            continue;

        if (decision.fConf < detection.conf)
        {
            decision.fConf = detection.conf;
            decision.nArea = detection.box.area();
        }
    }

    m_lstZScoreConfs.push_back(decision.fConf);
    while (m_nZScoreWindowSize + 1 < m_lstZScoreConfs.size())
        m_lstZScoreConfs.pop_front();

    decision.fZScore = _getZScore();

    // Z-Score가 낮을 경우에만 최근 연속 Conf값으로 상태 업데이트
    decision.bExists = m_bLastExists;
    if (decision.fZScore < m_fZScoreThreshold)
    {
        bool bCurrentExists = (m_fConfThreshold < decision.fConf);
        m_lstExists.push_back(bCurrentExists);
        while (m_nConfViewCount < m_lstExists.size())
            m_lstExists.pop_front();

        decision.bExists = m_bCheckExists;
        for (std::list<bool>::iterator it = m_lstExists.begin();
            m_lstExists.end() != it; it++)
        {
            if (m_bCheckExists != *it)
            {
                decision.bExists = !m_bCheckExists;
                break;
            }
        }
    }
    m_bLastExists = decision.bExists;

    return decision;
}

float Decider::_getZScore()
{
    float fZScore = 0.0f;

    if (m_nZScoreWindowSize < 2 ||
        m_nZScoreWindowSize + 1 != m_lstZScoreConfs.size())
        return fZScore;

    std::list<float>::iterator itLast = std::prev(std::end(m_lstZScoreConfs));

    float fSum = std::accumulate(std::begin(m_lstZScoreConfs), itLast, 0.0f);
    float fMean = fSum / m_nZScoreWindowSize;

    float fAccum = 0.0f;
    std::for_each(std::begin(m_lstZScoreConfs), itLast,
        [&](const float& f) { fAccum += ((f - fMean) * (f - fMean)); });
    
    float fStdev = sqrt(fAccum / (m_nZScoreWindowSize - 1));
    
    // 원래의 Z-Score에 Mean을 곱하여 변화량을 줄임.
    if (0 < fStdev)
        fZScore = (abs(m_lstZScoreConfs.back() - fMean) / fStdev) * fMean;

    return fZScore;
}

}