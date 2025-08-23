#include "ShapePool.h"
#include "ShapeSprite.h"

USING_NS_CC;

ShapePool* ShapePool::instance = nullptr;

ShapePool::ShapePool()
    : totalShapes(0)
    , initialPoolSize(5)
{
}

ShapePool::~ShapePool()
{
    Cleanup();
}

ShapePool* ShapePool::GetInstance()
{
    if (!instance)
        instance = new ShapePool();
    
    return instance;
}

void ShapePool::DestroyInstance()
{
    if (instance)
    {
        delete instance;

        instance = nullptr;
    }
}

void ShapePool::Initialize(int initialPoolSize)
{
    initialPoolSize = initialPoolSize;
    
    // 초기 풀 생성 (5개)
    for (int i = 0; i < initialPoolSize; i++)
    {
        ShapeSprite* shape = CreateNewShape();

        if (shape)
            availableShapes.push(shape);
    }
    
    CCLOG("ShapePool initialized with %d shapes", (int)availableShapes.size());
}

void ShapePool::Cleanup()
{
    // 활성 도형들 정리
    for (auto shape : activeShapes)
    {
        if (shape && shape->getParent())
            shape->removeFromParent();

        CC_SAFE_DELETE(shape);
    }

    activeShapes.clear();
    
    // 풀에 있는 도형들 정리
    while (!availableShapes.empty())
    {
        ShapeSprite* shape = availableShapes.front();
        availableShapes.pop();

        CC_SAFE_DELETE(shape);
    }
    
    totalShapes = 0;
    CCLOG("ShapePool cleaned up");
}

ShapeSprite* ShapePool::CreateNewShape()
{
    ShapeSprite* shape = ShapeSprite::Create(3); // 기본 3각형으로 생성

    if (shape)
    {
        shape->retain(); // 풀에서 관리하므로 retain
        shape->setVisible(false); // 풀에 있을 때는 보이지 않게

        totalShapes++;

        CCLOG("Created new shape, total shapes: %d", totalShapes);
    }

    return shape;
}

void ShapePool::ResetShape(ShapeSprite* shape, int level, const cocos2d::Vec2& position)
{
    if (!shape) 
        return;
    
    // 도형의 레벨과 외관 재설정
    shape->SetLevel(level);
    
    // 실제 각형 수도 업데이트 (30각형까지만)
    int sides = (level <= 30) ? level : 30;
    shape->SetSides(sides);
    
    // 크기 설정
    float scale = 1.0f;

        // 31레벨부터는 크기로 구분
    if (level > 30)
        scale = 1.0f + ((level - 30) * 0.1f); // 점진적 크기 증가

    shape->SetShapeScale(scale);
    
    // 텍스처와 물리 바디 재생성
    shape->CreateShapeTexture();
    shape->SetupPhysicsBody();
    
    // 위치 및 기본 속성 설정
    shape->setPosition(position);
    shape->setVisible(true);
    shape->getPhysicsBody()->setVelocity(Vec2::ZERO);
    shape->getPhysicsBody()->setAngularVelocity(0.0f);
}

ShapeSprite* ShapePool::GetShape(int level, const cocos2d::Vec2& position)
{
    ShapeSprite* shape = nullptr;
    
    if (!availableShapes.empty())
    {
        // 풀에서 가져오기
        shape = availableShapes.front();
        availableShapes.pop();

        CCLOG("Retrieved shape from pool, remaining in pool: %d", (int)availableShapes.size());
    }

    else
    {
        // 풀이 비어있으면 새로 생성
        shape = CreateNewShape();
        CCLOG("Pool empty, created new shape. Total shapes: %d", totalShapes);
    }
    
    if (shape)
    {
        // 도형 재설정
        ResetShape(shape, level, position);
        
        // 활성 리스트에 추가
        activeShapes.push_back(shape);
    }
    
    return shape;
}

void ShapePool::ReturnShape(ShapeSprite* shape)
{
    if (shape == nullptr) 
        return;
    
    // 활성 리스트에서 제거
    auto it = std::find(activeShapes.begin(), activeShapes.end(), shape);

    if (it != activeShapes.end())
    {
        activeShapes.erase(it);
        
        // 씬에서 제거
        if (shape->getParent())
            shape->removeFromParent();
        
        // 도형 초기화
        shape->setVisible(false);
        shape->getPhysicsBody()->setVelocity(Vec2::ZERO);
        shape->getPhysicsBody()->setAngularVelocity(0.0f);
        
        // 풀에 반환
        availableShapes.push(shape);
        
        CCLOG("Shape returned to pool, pool size: %d, active shapes: %d", 
              (int)availableShapes.size(), (int)activeShapes.size());
    }
}