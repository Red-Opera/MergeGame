#ifndef __SHAPE_GAME_SCENE_H__
#define __SHAPE_GAME_SCENE_H__

#include "cocos2d.h"
#include "physics/CCPhysicsWorld.h"
#include <vector>
#include <map>
#include <algorithm>

class ShapeSprite;

class ShapeGameScene : public cocos2d::Layer
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    
    void onAcceleration(cocos2d::Acceleration* acc, cocos2d::Event* event);
    bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);
    
    CREATE_FUNC(ShapeGameScene);
    
    cocos2d::PhysicsWorld* m_world;
    
private:
    void setupPhysicsWorld();
    void createGameBox();
    void createInitialShapes();
    void addRandomShape();
    void addShapeAtPosition(const cocos2d::Vec2& position);
    
    bool onContactBegin(cocos2d::PhysicsContact& contact);
    void mergeShapes(ShapeSprite* shape1, ShapeSprite* shape2);
    void addScore(int points);
    void updateScoreDisplay();
    void createScoreUI();
    void gameOver();
    bool isSpaceAvailable(const cocos2d::Vec2& position);
    void restartGame();
    void saveBestScore();
    void loadBestScore();
    void updateBestScoreDisplay();
    void updateCombo();
    void resetCombo();
    void updateComboDisplay();
    int calculateComboBonus(int baseScore, int comboCount);
    
    cocos2d::DrawNode* m_boxDrawNode;
    std::vector<ShapeSprite*> m_shapes;
    
    static const float BOX_WIDTH;
    static const float BOX_HEIGHT;
    static const float BOX_WALL_THICKNESS;
    
    cocos2d::Vec2 m_boxCenter;
    cocos2d::Size m_boxSize;
    
    // 점수 시스템
    int m_score;
    int m_bestScore;
    cocos2d::Label* m_scoreLabel;
    cocos2d::Label* m_bestScoreLabel;
    
    // 게임 상태
    bool m_isGameOver;
    cocos2d::Label* m_gameOverLabel;
    
    // 콤보 시스템
    int m_comboCount;
    float m_lastMergeTime;
    float m_comboTimeWindow;
    cocos2d::Label* m_comboLabel;
};

class ShapeSprite : public cocos2d::Sprite
{
public:
    static ShapeSprite* create(int level);
    static ShapeSprite* create(int level, float scale);
    
    bool initWithLevel(int level);
    bool initWithLevel(int level, float scale);
    void setupPhysicsBody();
    
    int getSides() const { return m_sides; }
    void setSides(int sides) { m_sides = sides; }
    float getShapeScale() const { return m_shapeScale; }
    void setShapeScale(float scale) { m_shapeScale = scale; }
    int getLevel() const { return m_level; }
    void setLevel(int level) { m_level = level; }
    
    static cocos2d::Color3B getColorBySides(int sides);
    
private:
    int m_sides;
    float m_shapeScale;
    int m_level; // 실제 도형 레벨 (3=삼각형, 11=11각형 등)
    
    void createShapeTexture();
    cocos2d::Texture2D* generatePolygonTexture(int sides, float radius, const cocos2d::Color3B& color);
};

#endif