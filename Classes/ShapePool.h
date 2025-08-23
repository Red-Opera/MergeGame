#ifndef __SHAPE_POOL_H__
#define __SHAPE_POOL_H__

#include "cocos2d.h"
#include "ShapeGameScene.h"
#include <vector>
#include <queue>

class ShapeSprite;

class ShapePool
{
public:
    static ShapePool* GetInstance();
    static void DestroyInstance();
    
    void Initialize(int initialPoolSize = 5);
    void Cleanup();
    
    ShapeSprite* GetShape(int level, const cocos2d::Vec2& position);
    void ReturnShape(ShapeSprite* shape);
    
    int GetActiveCount() const { return activeShapes.size(); }
    int GetPooledCount() const { return availableShapes.size(); }
    int GetTotalCount() const { return totalShapes; }

private:
    ShapePool();
    ~ShapePool();
    
    ShapeSprite* CreateNewShape();
    void ResetShape(ShapeSprite* shape, int level, const cocos2d::Vec2& position);
    
    static ShapePool* instance;
    
    std::queue<ShapeSprite*> availableShapes;
    std::vector<ShapeSprite*> activeShapes;
    int totalShapes;
    int initialPoolSize;
    
    ShapePool(const ShapePool&) = delete;
    ShapePool& operator=(const ShapePool&) = delete;
};

#endif // __SHAPE_POOL_H__