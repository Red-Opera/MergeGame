#ifndef __SHAPE_SPRITE_H__
#define __SHAPE_SPRITE_H__

#include "cocos2d.h"

class ShapeSprite : public cocos2d::Sprite
{
public:
    static ShapeSprite* Create(int level);
    static ShapeSprite* Create(int level, float scale);
    
    bool InitWithLevel(int level);
    bool InitWithLevel(int level, float scale);
    void SetupPhysicsBody();
    
    int GetSides() const { return sides; }
    void SetSides(int sides) { this->sides = sides; }
    float GetShapeScale() const { return shapeScale; }
    void SetShapeScale(float scale) { shapeScale = scale; }
    int GetLevel() const { return level; }
    void SetLevel(int level) { this->level = level; }
    
    void CreateShapeTexture();
    static cocos2d::Color3B GetColorBySides(int sides);
    
private:
    int sides;
    float shapeScale;
    int level; // 실제 도형 레벨 (3=삼각형, 11=11각형 등)
    
    cocos2d::Texture2D* GeneratePolygonTexture(int sides, float radius, const cocos2d::Color3B& color);
};

#endif