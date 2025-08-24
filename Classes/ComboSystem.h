#ifndef __COMBO_SYSTEM_H__
#define __COMBO_SYSTEM_H__

#include "cocos2d.h"

class ComboSystem
{
public:
    ComboSystem();
    ~ComboSystem();
    
    void Initialize(const cocos2d::Vec2& labelPosition);
    void Update();
    void Reset();
    void UpdateDisplay();
    int CalculateBonus(int baseScore);
    
    void AddToParent(cocos2d::Node* parent, int zOrder = 100);
    void RemoveFromParent();
    
    int GetComboCount() const { return comboCount; }
    bool IsComboActive() const { return comboCount >= 2; }
    
private:
    cocos2d::Label* comboLabel;
    cocos2d::Node* parentNode;

    int comboCount;
    float lastMergeTime;
    float comboTimeWindow;
    
    float getCurrentTime() const;
};

#endif