#pragma once
#include "ITouchLayer.h"
#include <vector>
#include <map>
#include <atomic>

class ITouchTarget;
class IEngineInputIntl;

class TouchLayer :
    public ITouchLayer
{
    struct TouchArea {
        bool m_bIsActive;
        bool m_bIsRectangular;
        IntRect m_rect;
        unsigned int m_unAction;
        ITouchTarget* m_pTarget;
        int m_nTouchValue;
        bool m_bScrollVert;
        bool m_bScrollHoriz;
        bool m_bContinuousUpdates;
    };
public:
    TouchLayer(int xOrigin, int yOrigin, int nWidth, int nHeight, ITouchTarget* pTarget, unsigned int unOutsideAreaAction);
    TouchLayer(float xOrigin, float yOrigin, float fWidth, float fHeight, ITouchTarget* pTarget, unsigned int unOutsideAreaAction);
    virtual ~TouchLayer();

    static void SetEngineInput(IEngineInputIntl* pEngineInput);

    // ITouchLayer declarations
public:
    virtual void SetLayerActive(bool bActive);
    virtual bool GetLayerActive() const;
    virtual void AddToInputEngine();
    virtual void RemoveFromInputEngine();
    virtual void SetYScrollDelta(float yScrollDelta);
    virtual void SetXScrollDelta(float xScrollDelta);
    virtual unsigned int AddTouchArea(float xOrigin, float yOrigin, float fWidth, float fHeight, ITouchTarget* pTarget, unsigned int unAction, int nTouchValue);
    virtual void SetScrollingBehaviour(int unId, bool bScrollsVertically, bool bScrollsHorizontally);
    virtual void SetTouchTargetNeedsContinuousUpdates(int unId, bool bContinuousUpdates);
    virtual void SetTouchAreaActive(unsigned int unId, bool bActive);
    virtual Touch GetTouchTarget(int x, int y);
    virtual void MoveLayer(float fXOffset, float fYOffset);

public:

private:
    TouchArea* GetTouchArea(unsigned int unId);
    bool PointOutsideRect(int x, int y, const IntRect& rect);
    bool PointOutsideScrolledRect(int x, int y, const IntRect& rect, bool bCanScrollVert, bool bCanScrollHoriz);


private:
    bool m_bActive;
    unsigned int m_unOutsideAreaAction;
    IntRect m_layerRect;
    float m_yScrollDelta;
    float m_xScrollDelta;
    ITouchTarget* m_pTarget;

    float m_fXOffset;
    float m_fYOffset;

    std::vector<TouchArea*> m_vcTouchAreas;
    std::map<unsigned int, TouchArea*> m_mpTouchAreas;

    static std::atomic<unsigned int> s_unNextId;
    static IEngineInputIntl* s_pEngineInput;
};

