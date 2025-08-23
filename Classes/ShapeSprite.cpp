#include "ShapeSprite.h"

USING_NS_CC;

ShapeSprite* ShapeSprite::Create(int level)
{
    return Create(level, 1.0f);
}

ShapeSprite* ShapeSprite::Create(int level, float scale)
{
    ShapeSprite* sprite = new (std::nothrow) ShapeSprite();

    if (sprite && sprite->InitWithLevel(level, scale))
    {
        sprite->autorelease();
        return sprite;
    }

    CC_SAFE_DELETE(sprite);

    return nullptr;
}

bool ShapeSprite::InitWithLevel(int level)
{
    return InitWithLevel(level, 1.0f);
}

bool ShapeSprite::InitWithLevel(int level, float scale)
{
    // 최소 3각형만 제한
    if (level < 3) 
        return false; 
    
    this->level = level;
    sides = (level <= 30) ? level : 30; // 실제 각형 수도 증가시키되, 성능을 위해 최대 30각형까지만
    shapeScale = scale;

    CreateShapeTexture();
    SetupPhysicsBody();
    
    return true;
}

void ShapeSprite::CreateShapeTexture()
{
    float radius = (20.0f + (sides - 3) * 3.0f) * shapeScale;
    Color3B color = GetColorBySides(level); // 레벨 기반 색상
    
    auto texture = GeneratePolygonTexture(sides, radius, color);

    if (texture)
        this->initWithTexture(texture);
}

Texture2D* ShapeSprite::GeneratePolygonTexture(int sides, float radius, const Color3B& color)
{
    int textureSize = (int)(radius * 2.5f);
    
    RenderTexture* renderTexture = RenderTexture::create(textureSize, textureSize);

    if (!renderTexture) 
        return nullptr;
    
    renderTexture->begin();
    
    auto drawNode = DrawNode::create();
    
    std::vector<Vec2> vertices;
    float angleStep = 2.0f * M_PI / sides;
    
    for (int i = 0; i < sides; i++)
    {
        float angle = i * angleStep - M_PI / 2;
        float x = radius * cos(angle) + textureSize / 2;
        float y = radius * sin(angle) + textureSize / 2;

        vertices.push_back(Vec2(x, y));
    }
    
    drawNode->drawSolidPoly(vertices.data(), sides, Color4F(color.r/255.0f, color.g/255.0f, color.b/255.0f, 1.0f));
    drawNode->drawPoly(vertices.data(), sides, true, Color4F::WHITE);
    
    auto renderer = Director::getInstance()->getRenderer();
    auto& parentTransform = Director::getInstance()->getMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);

    drawNode->visit(renderer, parentTransform, true);
    renderTexture->end();
    
    return renderTexture->getSprite()->getTexture();
}

void ShapeSprite::SetupPhysicsBody()
{
    float radius = (20.0f + (sides - 3) * 3.0f) * shapeScale;
    
    std::vector<Vec2> vertices;
    float angleStep = 2.0f * M_PI / sides;
    
    for (int i = 0; i < sides; i++)
    {
        float angle = i * angleStep - M_PI / 2;
        float x = radius * cos(angle);
        float y = radius * sin(angle);

        vertices.push_back(Vec2(x, y));
    }
    
    auto physicsBody = PhysicsBody::createPolygon(vertices.data(), sides);

    physicsBody->setDynamic(true);
    physicsBody->setContactTestBitmask(0xFFFFFFFF);
    physicsBody->setCollisionBitmask(0xFFFFFFFF);
    physicsBody->getFirstShape()->setRestitution(0.3f);
    physicsBody->getFirstShape()->setFriction(0.5f);
    physicsBody->setMass(1.0f);
    
    this->setPhysicsBody(physicsBody);
}

Color3B ShapeSprite::GetColorBySides(int sides)
{
    static const Color3B baseColors[] = {
        Color3B(255, 100, 100),  // 3 - Red
        Color3B(100, 255, 100),  // 4 - Green  
        Color3B(100, 100, 255),  // 5 - Blue
        Color3B(255, 255, 100),  // 6 - Yellow
        Color3B(255, 100, 255),  // 7 - Magenta
        Color3B(100, 255, 255),  // 8 - Cyan
        Color3B(255, 150, 100),  // 9 - Orange
        Color3B(150, 100, 255)   // 10 - Purple
    };
    
    if (sides >= 3 && sides <= 10)
        return baseColors[sides - 3];
    
    else if (sides > 10 && sides <= 30)
    {
        // 11~30각형: 무지개 색상으로 순환
        int colorIndex = (sides - 11) % 7;
        static const Color3B rainbowColors[] = {
            Color3B(255, 0, 0),    // 빨강
            Color3B(255, 127, 0),  // 주황
            Color3B(255, 255, 0),  // 노랑
            Color3B(0, 255, 0),    // 초록
            Color3B(0, 0, 255),    // 파랑
            Color3B(75, 0, 130),   // 남색
            Color3B(148, 0, 211)   // 보라
        };

        return rainbowColors[colorIndex];
    }

    else if (sides > 30)
    {
        // 30각형 이후: 금색/은색 계열로 특별 표시
        int colorIndex = (sides - 31) % 3;
        static const Color3B specialColors[] = {
            Color3B(255, 215, 0),  // 금색
            Color3B(192, 192, 192), // 은색
            Color3B(255, 255, 255)  // 백금색
        };

        return specialColors[colorIndex];
    }
    
    return Color3B::WHITE;
}