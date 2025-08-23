#include "GameStateManager.h"
#include "ShapeSprite.h"
#include "ShapePool.h"

USING_NS_CC;

GameStateManager::GameStateManager()
    : isGameOver(false)
    , gameOverLabel(nullptr)
    , overlay(nullptr)
    , parentNode(nullptr)
    , shapes(nullptr)
    , onRestartCallback(nullptr)
{
}

GameStateManager::~GameStateManager()
{
    CleanupGameOverUI();
}

void GameStateManager::Initialize()
{
    isGameOver = false;

    CleanupGameOverUI();
}

void GameStateManager::GameOver(int finalScore, int bestScore, bool isNewRecord)
{
    if (isGameOver)
        return;
    
    isGameOver = true;
    ShowGameOverUI(finalScore, bestScore, isNewRecord);
    
    CCLOG("Game Over! Final Score: %d, Best Score: %d", finalScore, bestScore);
}

void GameStateManager::RestartGame()
{
    CleanupShapes();
    CleanupGameOverUI();
    
    isGameOver = false;
    
    if (onRestartCallback)
        onRestartCallback();
    
    CCLOG("Game Restarted!");
}

bool GameStateManager::HandleTouchForRestart(const Vec2& touchLocation)
{
    if (isGameOver)
    {
        RestartGame();

        return true;
    }

    return false;
}

void GameStateManager::ShowGameOverUI(int finalScore, int bestScore, bool isNewRecord)
{
    if (!parentNode)
        return;
    
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    
    // 게임 오버 메시지 생성
    std::string gameOverText = "GAME OVER\nFinal Score: " + std::to_string(finalScore);

    if (isNewRecord)
        gameOverText += "\nNEW RECORD!";

    gameOverText += "\n\nTap to restart";
    
    gameOverLabel = Label::createWithSystemFont(gameOverText, "Arial", 28);
    gameOverLabel->setColor(isNewRecord ? Color3B::YELLOW : Color3B::RED);
    gameOverLabel->setPosition(Vec2(origin.x + visibleSize.width/2, origin.y + visibleSize.height/2));
    gameOverLabel->setAlignment(cocos2d::TextHAlignment::CENTER);
    parentNode->addChild(gameOverLabel, 200);
    
    // 반투명 배경 추가
    overlay = LayerColor::create(Color4B(0, 0, 0, 150));
    parentNode->addChild(overlay, 150);
}

void GameStateManager::CleanupGameOverUI()
{
    if (gameOverLabel && parentNode)
    {
        gameOverLabel->removeFromParent();
        gameOverLabel = nullptr;
    }
    
    if (overlay && parentNode)
    {
        overlay->removeFromParent();
        overlay = nullptr;
    }
    
    // Remove all children with z-order 150 (overlays)
    if (parentNode)
    {
        auto children = parentNode->getChildren();

        for (auto child : children)
        {
            if (child->getLocalZOrder() == 150)
                child->removeFromParent();
        }
    }
}

void GameStateManager::CleanupShapes()
{
    if (shapes)
    {
        for (auto shape : *shapes)
        {
            if (shape)
                ShapePool::GetInstance()->ReturnShape(shape);
        }

        shapes->clear();
    }
}