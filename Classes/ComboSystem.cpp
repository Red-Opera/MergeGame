#include "ComboSystem.h"

USING_NS_CC;

ComboSystem::ComboSystem()
    : comboCount(0)
    , lastMergeTime(0.0f)
    , comboTimeWindow(1.5f)
    , comboLabel(nullptr)
    , parentNode(nullptr)
{
}

ComboSystem::~ComboSystem()
{
    if (comboLabel && parentNode)
        comboLabel->removeFromParent();
}

void ComboSystem::Initialize(const Vec2& labelPosition)
{
    comboLabel = Label::createWithSystemFont("", "Arial", 32);
    comboLabel->setColor(Color3B::ORANGE);
    comboLabel->setPosition(labelPosition);
    comboLabel->setVisible(false);
}

void ComboSystem::Update()
{
    float currentTime = getCurrentTime();
    
    if (comboCount == 0 || (currentTime - lastMergeTime) <= comboTimeWindow)
    {
        comboCount++;
        lastMergeTime = currentTime;
        
        if (comboCount >= 2)
        {
            UpdateDisplay();
            
            if (comboLabel)
            {
                comboLabel->setScale(1.5f);

                auto scaleAction = ScaleTo::create(0.3f, 1.0f);
                auto easeAction = EaseBackOut::create(scaleAction);

                comboLabel->runAction(easeAction);
            }
        }
    }

    else
    {
        Reset();
        Update();
    }
}

void ComboSystem::Reset()
{
    comboCount = 0;
    lastMergeTime = 0.0f;
    
    if (comboLabel)
        comboLabel->setVisible(false);
}

void ComboSystem::UpdateDisplay()
{
    if (comboLabel && comboCount >= 2)
    {
        std::string comboText = "COMBO x" + std::to_string(comboCount) + "!";
        comboLabel->setString(comboText);
        comboLabel->setVisible(true);
        
        if (comboCount >= 5)
            comboLabel->setColor(Color3B::RED);

        else if (comboCount >= 3)
            comboLabel->setColor(Color3B::MAGENTA);

        else
            comboLabel->setColor(Color3B::ORANGE);
    }
}

int ComboSystem::CalculateBonus(int baseScore)
{
    if (comboCount < 2) 
        return 0;
    
    float multiplier = (comboCount - 1) * 0.5f;
    int bonus = (int)(baseScore * multiplier);
    
    return bonus;
}

void ComboSystem::AddToParent(Node* parent, int zOrder)
{
    if (comboLabel && parent)
    {
        parentNode = parent;
        parent->addChild(comboLabel, zOrder);
    }
}

void ComboSystem::RemoveFromParent()
{
    if (comboLabel && parentNode)
    {
        comboLabel->removeFromParent();
        parentNode = nullptr;
    }
}

float ComboSystem::getCurrentTime() const
{
    return Director::getInstance()->getTotalFrames() / 60.0f;
}