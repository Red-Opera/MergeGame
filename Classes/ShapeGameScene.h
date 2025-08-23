#ifndef __SHAPE_GAME_SCENE_H__
#define __SHAPE_GAME_SCENE_H__

#include "cocos2d.h"
#include "physics/CCPhysicsWorld.h"
#include <vector>
#include <map>
#include <algorithm>

class ShapeSprite;
class ShapePool;
class ComboSystem;
class GameStateManager;

class ShapeGameScene : public cocos2d::Layer
{
public:
    static cocos2d::Scene* CreateScene();
    virtual bool init() override;
    virtual ~ShapeGameScene();
    
    void OnAcceleration(cocos2d::Acceleration* acc, cocos2d::Event* event);
    bool OnTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);
    
    CREATE_FUNC(ShapeGameScene);
    
    cocos2d::PhysicsWorld* world;
    
private:
    void SetupPhysicsWorld();
    void CreateGameBox();
    void CreateInitialShapes();
    void AddRandomShape();
    void AddShapeAtPosition(const cocos2d::Vec2& position);
    
    bool OnContactBegin(cocos2d::PhysicsContact& contact);
    void MergeShapes(ShapeSprite* shape1, ShapeSprite* shape2);
    void AddScore(int points);
    void UpdateScoreDisplay();
    void CreateScoreUI();
    bool IsSpaceAvailable(const cocos2d::Vec2& position);
    void SaveBestScore();
    void LoadBestScore();
    void UpdateBestScoreDisplay();
    void OnGameRestart();
    
    cocos2d::DrawNode* boxDrawNode;
    std::vector<ShapeSprite*> shapes;
    
    static const float BOX_WIDTH;
    static const float BOX_HEIGHT;
    static const float BOX_WALL_THICKNESS;
    
    cocos2d::Vec2 boxCenter;
    cocos2d::Size boxSize;
    
    // 점수 시스템
    int score;
    int bestScore;
    cocos2d::Label* scoreLabel;
    cocos2d::Label* bestScoreLabel;
    
    // 콤보 시스템
    ComboSystem* comboSystem;
    
    // 게임 상태 관리
    GameStateManager* gameStateManager;
};
#endif