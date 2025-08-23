#ifndef __GAME_STATE_MANAGER_H__
#define __GAME_STATE_MANAGER_H__

#include "cocos2d.h"
#include <functional>
#include <vector>

class ShapeSprite;

class GameStateManager
{
public:
    GameStateManager();
    ~GameStateManager();
    
    void Initialize();
    void GameOver(int finalScore, int bestScore, bool isNewRecord);
    void RestartGame();
    
    void SetParentNode(cocos2d::Node* parent) { parentNode = parent; }
    void SetRestartCallback(std::function<void()> callback) { onRestartCallback = callback; }
    void SetShapesContainer(std::vector<ShapeSprite*>* shapesPtr) { shapes = shapesPtr; }
    
    bool IsGameOver() const { return isGameOver; }
    
    bool HandleTouchForRestart(const cocos2d::Vec2& touchLocation);
    
private:
    void ShowGameOverUI(int finalScore, int bestScore, bool isNewRecord);
    void CleanupGameOverUI();
    void CleanupShapes();
    
    bool isGameOver;
    cocos2d::Label* gameOverLabel;
    cocos2d::LayerColor* overlay;
    cocos2d::Node* parentNode;
    
    std::vector<ShapeSprite*>* shapes;
    std::function<void()> onRestartCallback;
};

#endif