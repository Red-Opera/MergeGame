#include "ShapeGameScene.h"

USING_NS_CC;

const float ShapeGameScene::BOX_WIDTH = 600.0f;
const float ShapeGameScene::BOX_HEIGHT = 400.0f;
const float ShapeGameScene::BOX_WALL_THICKNESS = 10.0f;

Scene* ShapeGameScene::createScene()
{
    auto scene = Scene::createWithPhysics();
    auto gameLayer = ShapeGameScene::create();
    
    // Set physics world reference before adding to scene
    gameLayer->m_world = scene->getPhysicsWorld();
    gameLayer->m_world->setGravity(Vec2(0, -300));
    
    scene->addChild(gameLayer);
    
    return scene;
}

bool ShapeGameScene::init()
{
    if (!Layer::init())
    {
        return false;
    }
    
    // m_world will be set in createScene() before this init is called
    
    // 점수 초기화
    m_score = 0;
    m_bestScore = 0;
    m_isGameOver = false;
    m_gameOverLabel = nullptr;
    m_bestScoreLabel = nullptr;
    
    // 콤보 시스템 초기화
    m_comboCount = 0;
    m_lastMergeTime = 0.0f;
    m_comboTimeWindow = 1.5f; // 1.5초 내에 연속 합치기
    m_comboLabel = nullptr;
    
    // 베스트 스코어 로드
    loadBestScore();
    
    setupPhysicsWorld();
    createGameBox();
    createInitialShapes();
    createScoreUI();
    
    auto contactListener = EventListenerPhysicsContact::create();
    contactListener->onContactBegin = CC_CALLBACK_1(ShapeGameScene::onContactBegin, this);
    this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(contactListener, this);
    
    auto accelerationListener = EventListenerAcceleration::create(CC_CALLBACK_2(ShapeGameScene::onAcceleration, this));
    this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(accelerationListener, this);
    Device::setAccelerometerEnabled(true);
    
    auto touchListener = EventListenerTouchOneByOne::create();
    touchListener->onTouchBegan = CC_CALLBACK_2(ShapeGameScene::onTouchBegan, this);
    this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(touchListener, this);
    
    scheduleUpdate();
    
    return true;
}

void ShapeGameScene::setupPhysicsWorld()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    
    m_boxCenter = Vec2(origin.x + visibleSize.width/2, origin.y + visibleSize.height/2);
    
    // 가로 모드에 맞춰 박스 크기 조정
    float boxWidth = MIN(visibleSize.width * 0.5f, 400.0f);  // 화면 너비의 50%, 최대 400px
    float boxHeight = MIN(visibleSize.height * 0.7f, 450.0f); // 화면 높이의 70%, 최대 450px
    
    // 최소 크기 보장
    boxWidth = MAX(boxWidth, 200.0f);
    boxHeight = MAX(boxHeight, 300.0f);
    
    m_boxSize = Size(boxWidth, boxHeight);
    
    // 디버깅을 위한 로그 출력
    CCLOG("Screen size: %.0f x %.0f", visibleSize.width, visibleSize.height);
    CCLOG("Box size: %.0f x %.0f", boxWidth, boxHeight);
}

void ShapeGameScene::createGameBox()
{
    m_boxDrawNode = DrawNode::create();
    this->addChild(m_boxDrawNode);
    
    Vec2 boxRect[4];
    boxRect[0] = Vec2(m_boxCenter.x - m_boxSize.width/2, m_boxCenter.y - m_boxSize.height/2);
    boxRect[1] = Vec2(m_boxCenter.x + m_boxSize.width/2, m_boxCenter.y - m_boxSize.height/2);
    boxRect[2] = Vec2(m_boxCenter.x + m_boxSize.width/2, m_boxCenter.y + m_boxSize.height/2);
    boxRect[3] = Vec2(m_boxCenter.x - m_boxSize.width/2, m_boxCenter.y + m_boxSize.height/2);
    
    m_boxDrawNode->drawPoly(boxRect, 4, false, Color4F::WHITE);
    
    auto leftWall = Node::create();
    auto leftWallBody = PhysicsBody::createBox(Size(BOX_WALL_THICKNESS, m_boxSize.height));
    leftWallBody->setDynamic(false);
    leftWallBody->setContactTestBitmask(0xFFFFFFFF);
    leftWall->setPhysicsBody(leftWallBody);
    leftWall->setPosition(m_boxCenter.x - m_boxSize.width/2, m_boxCenter.y);
    this->addChild(leftWall);
    
    auto rightWall = Node::create();
    auto rightWallBody = PhysicsBody::createBox(Size(BOX_WALL_THICKNESS, m_boxSize.height));
    rightWallBody->setDynamic(false);
    rightWallBody->setContactTestBitmask(0xFFFFFFFF);
    rightWall->setPhysicsBody(rightWallBody);
    rightWall->setPosition(m_boxCenter.x + m_boxSize.width/2, m_boxCenter.y);
    this->addChild(rightWall);
    
    auto bottomWall = Node::create();
    auto bottomWallBody = PhysicsBody::createBox(Size(m_boxSize.width, BOX_WALL_THICKNESS));
    bottomWallBody->setDynamic(false);
    bottomWallBody->setContactTestBitmask(0xFFFFFFFF);
    bottomWall->setPhysicsBody(bottomWallBody);
    bottomWall->setPosition(m_boxCenter.x, m_boxCenter.y - m_boxSize.height/2);
    this->addChild(bottomWall);
    
    auto topWall = Node::create();
    auto topWallBody = PhysicsBody::createBox(Size(m_boxSize.width, BOX_WALL_THICKNESS));
    topWallBody->setDynamic(false);
    topWallBody->setContactTestBitmask(0xFFFFFFFF);
    topWall->setPhysicsBody(topWallBody);
    topWall->setPosition(m_boxCenter.x, m_boxCenter.y + m_boxSize.height/2);
    this->addChild(topWall);
}

void ShapeGameScene::createScoreUI()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    
    // 현재 점수 라벨 생성
    m_scoreLabel = Label::createWithSystemFont("Score: 0", "Arial", 24);
    m_scoreLabel->setColor(Color3B::WHITE);
    m_scoreLabel->setPosition(Vec2(origin.x + 80, origin.y + visibleSize.height - 50));
    this->addChild(m_scoreLabel, 100);
    
    // 베스트 스코어 라벨 생성
    m_bestScoreLabel = Label::createWithSystemFont("Best: " + std::to_string(m_bestScore), "Arial", 20);
    m_bestScoreLabel->setColor(Color3B::YELLOW);
    m_bestScoreLabel->setPosition(Vec2(origin.x + 80, origin.y + visibleSize.height - 80));
    this->addChild(m_bestScoreLabel, 100);
    
    // 콤보 라벨 생성 (처음에는 숨김)
    m_comboLabel = Label::createWithSystemFont("", "Arial", 32);
    m_comboLabel->setColor(Color3B::ORANGE);
    m_comboLabel->setPosition(Vec2(origin.x + visibleSize.width - 120, origin.y + visibleSize.height - 60));
    m_comboLabel->setVisible(false);
    this->addChild(m_comboLabel, 100);
}

void ShapeGameScene::addScore(int points)
{
    m_score += points;
    updateScoreDisplay();
    
    CCLOG("Score added: %d, Total score: %d", points, m_score);
}

void ShapeGameScene::updateScoreDisplay()
{
    if (m_scoreLabel)
    {
        std::string scoreText = "Score: " + std::to_string(m_score);
        m_scoreLabel->setString(scoreText);
    }
}

void ShapeGameScene::loadBestScore()
{
    // UserDefault에서 베스트 스코어 로드
    m_bestScore = UserDefault::getInstance()->getIntegerForKey("BestScore", 0);
}

void ShapeGameScene::saveBestScore()
{
    UserDefault::getInstance()->setIntegerForKey("BestScore", m_bestScore);
    UserDefault::getInstance()->flush(); // 즉시 저장
}

void ShapeGameScene::updateBestScoreDisplay()
{
    if (m_bestScoreLabel)
    {
        std::string bestScoreText = "Best: " + std::to_string(m_bestScore);
        m_bestScoreLabel->setString(bestScoreText);
    }
}

bool ShapeGameScene::isSpaceAvailable(const Vec2& position)
{
    // 새로운 삼각형의 반지름 (ShapeSprite::createShapeTexture에서 사용하는 공식)
    float shapeRadius = 20.0f; // 삼각형(3-3=0)의 기본 반지름
    
    // 위치 주변에 다른 도형이 있는지 확인
    for (auto shape : m_shapes)
    {
        if (shape && shape->getParent())
        {
            Vec2 shapePos = shape->getPosition();
            float distance = position.distance(shapePos);
            
            // 다른 도형과의 최소 거리 계산 (두 도형의 반지름 + 여유 공간)
            float otherShapeRadius = 20.0f + (shape->getSides() - 3) * 3.0f;
            float minDistance = (shapeRadius + otherShapeRadius) * shape->getShapeScale() + 10.0f; // 10px 여유
            
            if (distance < minDistance)
            {
                return false; // 공간 부족
            }
        }
    }
    
    return true; // 공간 있음
}

void ShapeGameScene::gameOver()
{
    if (m_isGameOver) return; // 이미 게임 오버 상태면 리턴
    
    m_isGameOver = true;
    
    // 베스트 스코어 체크 및 저장
    bool isNewRecord = false;
    if (m_score > m_bestScore)
    {
        m_bestScore = m_score;
        saveBestScore();
        updateBestScoreDisplay();
        isNewRecord = true;
    }
    
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    
    // 게임 오버 메시지 생성
    std::string gameOverText = "GAME OVER\nFinal Score: " + std::to_string(m_score);
    if (isNewRecord)
    {
        gameOverText += "\nNEW RECORD!";
    }
    gameOverText += "\n\nTap to restart";
    
    m_gameOverLabel = Label::createWithSystemFont(gameOverText, "Arial", 28);
    m_gameOverLabel->setColor(isNewRecord ? Color3B::YELLOW : Color3B::RED);
    m_gameOverLabel->setPosition(Vec2(origin.x + visibleSize.width/2, origin.y + visibleSize.height/2));
    m_gameOverLabel->setAlignment(cocos2d::TextHAlignment::CENTER);
    this->addChild(m_gameOverLabel, 200); // 최상위에 표시
    
    // 반투명 배경 추가
    auto overlay = LayerColor::create(Color4B(0, 0, 0, 150)); // 반투명 검정
    this->addChild(overlay, 150);
    
    CCLOG("Game Over! Final Score: %d, Best Score: %d", m_score, m_bestScore);
}

void ShapeGameScene::restartGame()
{
    // 모든 도형 제거
    for (auto shape : m_shapes)
    {
        if (shape && shape->getParent())
        {
            shape->removeFromParent();
        }
    }
    m_shapes.clear();
    
    // 게임 오버 UI 제거
    if (m_gameOverLabel)
    {
        m_gameOverLabel->removeFromParent();
        m_gameOverLabel = nullptr;
    }
    
    // 오버레이 제거 (z-order 150인 모든 자식들 제거)
    auto children = this->getChildren();
    for (auto child : children)
    {
        if (child->getLocalZOrder() == 150)
        {
            child->removeFromParent();
        }
    }
    
    // 게임 상태 초기화
    m_score = 0;
    m_isGameOver = false;
    
    // 콤보 초기화
    resetCombo();
    
    // UI 업데이트
    updateScoreDisplay();
    
    // 초기 도형들 다시 생성
    createInitialShapes();
    
    CCLOG("Game Restarted!");
}

void ShapeGameScene::createInitialShapes()
{
    // Start with fewer initial shapes since player can add more by touching
    for (int i = 0; i < 2; i++)
    {
        addRandomShape();
    }
}

void ShapeGameScene::addRandomShape()
{
    int level = 3 + (rand() % 3); // 3~5레벨 랜덤
    auto shape = ShapeSprite::create(level);
    
    float margin = 60.0f;
    float x = m_boxCenter.x + (rand() % (int)(m_boxSize.width - margin)) - (m_boxSize.width - margin)/2;
    float y = m_boxCenter.y + m_boxSize.height/2 - 50;
    shape->setPosition(x, y);
    
    this->addChild(shape);
    m_shapes.push_back(shape);
}

bool ShapeGameScene::onContactBegin(cocos2d::PhysicsContact& contact)
{
    auto nodeA = contact.getShapeA()->getBody()->getNode();
    auto nodeB = contact.getShapeB()->getBody()->getNode();
    
    auto shapeA = dynamic_cast<ShapeSprite*>(nodeA);
    auto shapeB = dynamic_cast<ShapeSprite*>(nodeB);
    
    if (shapeA && shapeB && shapeA->getLevel() == shapeB->getLevel())
    {
        // Check if shapes are still valid and not already being processed
        auto it1 = std::find(m_shapes.begin(), m_shapes.end(), shapeA);
        auto it2 = std::find(m_shapes.begin(), m_shapes.end(), shapeB);
        
        if (it1 != m_shapes.end() && it2 != m_shapes.end())
        {
            // Retain the shapes to prevent deletion during physics processing
            shapeA->retain();
            shapeB->retain();
            
            // Schedule merge to happen on next frame to avoid physics issues
            this->runAction(CallFunc::create([this, shapeA, shapeB](){
                // Check if shapes are still valid before merging
                if (shapeA->getParent() && shapeB->getParent())
                {
                    mergeShapes(shapeA, shapeB);
                }
                // Release the retained references
                shapeA->release();
                shapeB->release();
            }));
        }
        return false;
    }
    
    return true;
}

void ShapeGameScene::mergeShapes(ShapeSprite* shape1, ShapeSprite* shape2)
{
    if (!shape1 || !shape2) return;
    if (!shape1->getParent() || !shape2->getParent()) return;
    if (!shape1->getPhysicsBody() || !shape2->getPhysicsBody()) return;
    
    // Double check that shapes are still in our container
    auto it1 = std::find(m_shapes.begin(), m_shapes.end(), shape1);
    auto it2 = std::find(m_shapes.begin(), m_shapes.end(), shape2);
    
    if (it1 == m_shapes.end() || it2 == m_shapes.end()) return;
    
    int newLevel = shape1->getLevel() + 1; // 레벨 기반 계산
    float newScale = 1.0f;
    
    // 30레벨을 넘어서면 크기로 구분 (무한 확장)
    if (newLevel > 30) 
    {
        // 31레벨부터는 크기를 증가시켜서 구분
        float scale1 = shape1->getShapeScale();
        float scale2 = shape2->getShapeScale();
        newScale = (scale1 + scale2) * 0.55f; // 점진적 크기 증가
    }
    
    // Calculate merge position as the midpoint between the two shapes
    Vec2 mergePos = (shape1->getPosition() + shape2->getPosition()) * 0.5f;
    
    // Store the velocity of the shapes to apply to the new merged shape
    Vec2 velocity1 = shape1->getPhysicsBody()->getVelocity();
    Vec2 velocity2 = shape2->getPhysicsBody()->getVelocity();
    Vec2 averageVelocity = (velocity1 + velocity2) * 0.5f;
    
    // Remove from vector using erase-remove idiom
    m_shapes.erase(std::remove(m_shapes.begin(), m_shapes.end(), shape1), m_shapes.end());
    m_shapes.erase(std::remove(m_shapes.begin(), m_shapes.end(), shape2), m_shapes.end());
    
    shape1->removeFromParent();
    shape2->removeFromParent();
    
    // Create new shape at the merge position
    auto newShape = ShapeSprite::create(newLevel, newScale);
    newShape->setPosition(mergePos);
    this->addChild(newShape);
    
    // Apply the average velocity to the new shape for more natural physics
    newShape->getPhysicsBody()->setVelocity(averageVelocity * 0.8f); // Slight dampening
    
    m_shapes.push_back(newShape);
    
    // 콤보 업데이트
    updateCombo();
    
    // 레벨에 따른 점수 획득 (무한 확장 가능)
    int baseScore = newLevel;
    int bonusScore = calculateComboBonus(baseScore, m_comboCount);
    addScore(baseScore + bonusScore);
    
    CCLOG("Base score: %d, Combo: %d, Bonus: %d, Total: %d", baseScore, m_comboCount, bonusScore, baseScore + bonusScore);
}

void ShapeGameScene::onAcceleration(cocos2d::Acceleration* acc, cocos2d::Event* event)
{
    // 디버깅을 위한 가속도계 값 출력
    CCLOG("Acceleration: x=%.3f, y=%.3f, z=%.3f", acc->x, acc->y, acc->z);
    
    float gravityStrength = 500.0f;
    
    // 좌우와 앞뒤 방향 모두 반전 적용
    float gravityX = acc->x * gravityStrength;  // 좌우 기울임 = X축 (방향 반전)
    float gravityY = acc->y * gravityStrength;  // 앞뒤 기울임 = Y축 (방향 반전)
    
    Vec2 gravity = Vec2(gravityX, gravityY);
    
    if (m_world)
    {
        m_world->setGravity(gravity);
    }
    
    CCLOG("Applied gravity: x=%.1f, y=%.1f", gravity.x, gravity.y);
    
    // Optional: Add shake detection for extra effects (주석 처리)
    /*
    float shakeThreshold = 1.5f; // Increased threshold to avoid constant triggering
    
    if (abs(acc->x) > shakeThreshold || abs(acc->y) > shakeThreshold || abs(acc->z) > shakeThreshold)
    {
        // Apply additional impulse for shake effect
        for (auto shape : m_shapes)
        {
            if (shape && shape->getPhysicsBody())
            {
                Vec2 shakeImpulse = Vec2(-(acc->x * 30000), (acc->z * 30000));
                shape->getPhysicsBody()->applyImpulse(shakeImpulse);
            }
        }
    }
    */
}

void ShapeGameScene::addShapeAtPosition(const Vec2& position)
{
    if (m_isGameOver) return; // 게임 오버 상태면 아무것도 하지 않음
    
    // Check if position is within the box bounds
    float margin = 30.0f;
    if (position.x < m_boxCenter.x - m_boxSize.width/2 + margin || 
        position.x > m_boxCenter.x + m_boxSize.width/2 - margin ||
        position.y < m_boxCenter.y - m_boxSize.height/2 + margin || 
        position.y > m_boxCenter.y + m_boxSize.height/2 - margin)
    {
        return; // Don't create shape outside the box
    }
    
    // 공간이 있는지 확인
    if (!isSpaceAvailable(position))
    {
        gameOver(); // 공간이 없으면 게임 오버
        return;
    }
    
    // Always create a triangle (level 3) when player touches
    auto shape = ShapeSprite::create(3);
    shape->setPosition(position);
    
    this->addChild(shape);
    m_shapes.push_back(shape);
    
    // 터치로 도형 생성 시에는 콤보를 리셋 (터치로는 콤보 불가)
    resetCombo();
    
    // 3각형 생성 시 3점 획득 (콤보 보너스 없음)
    addScore(3);
}

bool ShapeGameScene::onTouchBegan(Touch* touch, Event* event)
{
    if (m_isGameOver)
    {
        // 게임 오버 상태에서 터치하면 재시작
        restartGame();
        return true;
    }
    
    Vec2 touchLocation = touch->getLocation();
    addShapeAtPosition(touchLocation);
    return true;
}

ShapeSprite* ShapeSprite::create(int level)
{
    return create(level, 1.0f);
}

ShapeSprite* ShapeSprite::create(int level, float scale)
{
    ShapeSprite* sprite = new (std::nothrow) ShapeSprite();
    if (sprite && sprite->initWithLevel(level, scale))
    {
        sprite->autorelease();
        return sprite;
    }
    CC_SAFE_DELETE(sprite);
    return nullptr;
}

bool ShapeSprite::initWithLevel(int level)
{
    return initWithLevel(level, 1.0f);
}

bool ShapeSprite::initWithLevel(int level, float scale)
{
    if (level < 3) return false; // 최소 3각형만 제한
    
    m_level = level;
    // 실제 각형 수도 증가시키되, 성능을 위해 최대 30각형까지만
    m_sides = (level <= 30) ? level : 30;
    m_shapeScale = scale;
    createShapeTexture();
    setupPhysicsBody();
    
    return true;
}

void ShapeSprite::createShapeTexture()
{
    float radius = (20.0f + (m_sides - 3) * 3.0f) * m_shapeScale;
    Color3B color = getColorBySides(m_level); // 레벨 기반 색상
    
    auto texture = generatePolygonTexture(m_sides, radius, color);
    if (texture)
    {
        this->initWithTexture(texture);
    }
}

Texture2D* ShapeSprite::generatePolygonTexture(int sides, float radius, const Color3B& color)
{
    int textureSize = (int)(radius * 2.5f);
    
    RenderTexture* renderTexture = RenderTexture::create(textureSize, textureSize);
    if (!renderTexture) return nullptr;
    
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

void ShapeSprite::setupPhysicsBody()
{
    float radius = (20.0f + (m_sides - 3) * 3.0f) * m_shapeScale;
    
    std::vector<Vec2> vertices;
    float angleStep = 2.0f * M_PI / m_sides;
    
    for (int i = 0; i < m_sides; i++)
    {
        float angle = i * angleStep - M_PI / 2;
        float x = radius * cos(angle);
        float y = radius * sin(angle);
        vertices.push_back(Vec2(x, y));
    }
    
    auto physicsBody = PhysicsBody::createPolygon(vertices.data(), m_sides);
    physicsBody->setDynamic(true);
    physicsBody->setContactTestBitmask(0xFFFFFFFF);
    physicsBody->setCollisionBitmask(0xFFFFFFFF);
    physicsBody->getFirstShape()->setRestitution(0.3f);
    physicsBody->getFirstShape()->setFriction(0.5f);
    physicsBody->setMass(1.0f);
    
    this->setPhysicsBody(physicsBody);
}

Color3B ShapeSprite::getColorBySides(int sides)
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
    {
        return baseColors[sides - 3];
    }
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

void ShapeGameScene::updateCombo()
{
    float currentTime = Director::getInstance()->getTotalFrames() / 60.0f; // 프레임 기반 시간
    
    // 첫 번째 합치기거나 시간 창 내에서 합쳐진 경우
    if (m_comboCount == 0 || (currentTime - m_lastMergeTime) <= m_comboTimeWindow)
    {
        m_comboCount++;
        m_lastMergeTime = currentTime;
        
        // 콤보가 2 이상일 때만 UI 표시
        if (m_comboCount >= 2)
        {
            updateComboDisplay();
            
            // 콤보 효과 애니메이션
            if (m_comboLabel)
            {
                m_comboLabel->setScale(1.5f);
                auto scaleAction = ScaleTo::create(0.3f, 1.0f);
                auto easeAction = EaseBackOut::create(scaleAction);
                m_comboLabel->runAction(easeAction);
            }
        }
        
        // 콤보 타이머 스케줄 (기존 타이머 취소 후 새로 설정)
        unschedule("combo_timer");
        schedule([this](float dt) {
            float currentTime = Director::getInstance()->getTotalFrames() / 60.0f;
            if ((currentTime - m_lastMergeTime) > m_comboTimeWindow)
            {
                resetCombo();
                unschedule("combo_timer");
            }
        }, 0.1f, "combo_timer");
    }
    else
    {
        // 시간 창을 벗어났으므로 콤보 리셋
        resetCombo();
        updateCombo(); // 새로운 콤보 시작
    }
}

void ShapeGameScene::resetCombo()
{
    m_comboCount = 0;
    m_lastMergeTime = 0.0f;
    
    if (m_comboLabel)
    {
        m_comboLabel->setVisible(false);
    }
}

void ShapeGameScene::updateComboDisplay()
{
    if (m_comboLabel && m_comboCount >= 2)
    {
        std::string comboText = "COMBO x" + std::to_string(m_comboCount) + "!";
        m_comboLabel->setString(comboText);
        m_comboLabel->setVisible(true);
        
        // 콤보 수에 따른 색상 변경
        if (m_comboCount >= 5)
        {
            m_comboLabel->setColor(Color3B::RED); // 빨간색 (5콤보 이상)
        }
        else if (m_comboCount >= 3)
        {
            m_comboLabel->setColor(Color3B::MAGENTA); // 자주색 (3-4콤보)
        }
        else
        {
            m_comboLabel->setColor(Color3B::ORANGE); // 주황색 (2콤보)
        }
    }
}

int ShapeGameScene::calculateComboBonus(int baseScore, int comboCount)
{
    if (comboCount < 2) return 0; // 콤보가 2 미만이면 보너스 없음
    
    // 콤보 보너스 계산: (콤보 수 - 1) * 기본 점수 * 0.5
    // 예: 3콤보시 기본점수의 100% 보너스, 4콤보시 150% 보너스
    float multiplier = (comboCount - 1) * 0.5f;
    int bonus = (int)(baseScore * multiplier);
    
    return bonus;
}