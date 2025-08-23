#include "ShapeGameScene.h"
#include "ShapePool.h"
#include "ShapeSprite.h"
#include "ComboSystem.h"
#include "GameStateManager.h"

USING_NS_CC;

const float ShapeGameScene::BOX_WIDTH = 600.0f;
const float ShapeGameScene::BOX_HEIGHT = 400.0f;
const float ShapeGameScene::BOX_WALL_THICKNESS = 10.0f;

ShapeGameScene::~ShapeGameScene()
{
    if (comboSystem)
    {
        delete comboSystem;
        comboSystem = nullptr;
    }
    
    if (gameStateManager)
    {
        delete gameStateManager;
        gameStateManager = nullptr;
    }
}

Scene* ShapeGameScene::CreateScene()
{
    auto scene = Scene::createWithPhysics();
    auto gameLayer = ShapeGameScene::create();
    
    // Set physics world reference before adding to scene
    gameLayer->world = scene->getPhysicsWorld();
    gameLayer->world->setGravity(Vec2(0, -300));
    
    scene->addChild(gameLayer);
    
    return scene;
}

bool ShapeGameScene::init()
{
    if (!Layer::init())
        return false;
    
    // 점수 초기화
    score = 0;
    bestScore = 0;
    bestScoreLabel = nullptr;
    
    // 콤보 시스템 초기화
    comboSystem = new ComboSystem();
    
    // 게임 상태 관리자 초기화
    gameStateManager = new GameStateManager();
    
    // 베스트 스코어 로드
    LoadBestScore();
    
    // ShapePool 초기화
    ShapePool::GetInstance()->Initialize(5);
    
    SetupPhysicsWorld();
    CreateGameBox();
    CreateInitialShapes();
    CreateScoreUI();
    
    // 콤보 시스템 초기화
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    Vec2 comboPosition(origin.x + visibleSize.width - 120, origin.y + visibleSize.height - 60);
    comboSystem->Initialize(comboPosition);
    comboSystem->AddToParent(this, 100);
    
    // 게임 상태 관리자 초기화
    gameStateManager->Initialize();
    gameStateManager->SetParentNode(this);
    gameStateManager->SetShapesContainer(&shapes);
    gameStateManager->SetRestartCallback([this]() { OnGameRestart(); });
    
    auto contactListener = EventListenerPhysicsContact::create();
    contactListener->onContactBegin = CC_CALLBACK_1(ShapeGameScene::OnContactBegin, this);
    this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(contactListener, this);
    
    auto accelerationListener = EventListenerAcceleration::create(CC_CALLBACK_2(ShapeGameScene::OnAcceleration, this));
    this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(accelerationListener, this);
    Device::setAccelerometerEnabled(true);
    
    auto touchListener = EventListenerTouchOneByOne::create();
    touchListener->onTouchBegan = CC_CALLBACK_2(ShapeGameScene::OnTouchBegan, this);
    this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(touchListener, this);
    
    scheduleUpdate();
    
    return true;
}

void ShapeGameScene::SetupPhysicsWorld()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    
    boxCenter = Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2);
    
    // 가로 모드에 맞춰 박스 크기 조정
    float boxWidth = MIN(visibleSize.width * 0.5f, 400.0f);  // 화면 너비의 50%, 최대 400px
    float boxHeight = MIN(visibleSize.height * 0.7f, 450.0f); // 화면 높이의 70%, 최대 450px
    
    // 최소 크기 보장
    boxWidth = MAX(boxWidth, 200.0f);
    boxHeight = MAX(boxHeight, 300.0f);
    
    boxSize = Size(boxWidth, boxHeight);
}

void ShapeGameScene::CreateGameBox()
{
    boxDrawNode = DrawNode::create();
    this->addChild(boxDrawNode);
    
    Vec2 boxRect[4];
    boxRect[0] = Vec2(boxCenter.x - boxSize.width/2, boxCenter.y - boxSize.height/2);
    boxRect[1] = Vec2(boxCenter.x + boxSize.width/2, boxCenter.y - boxSize.height/2);
    boxRect[2] = Vec2(boxCenter.x + boxSize.width/2, boxCenter.y + boxSize.height/2);
    boxRect[3] = Vec2(boxCenter.x - boxSize.width/2, boxCenter.y + boxSize.height/2);
    
    boxDrawNode->drawPoly(boxRect, 4, false, Color4F::WHITE);
    
    auto leftWall = Node::create();
    auto leftWallBody = PhysicsBody::createBox(Size(BOX_WALL_THICKNESS, boxSize.height));
    leftWallBody->setDynamic(false);
    leftWallBody->setContactTestBitmask(0xFFFFFFFF);
    leftWall->setPhysicsBody(leftWallBody);
    leftWall->setPosition(boxCenter.x - boxSize.width/2, boxCenter.y);
    this->addChild(leftWall);
    
    auto rightWall = Node::create();
    auto rightWallBody = PhysicsBody::createBox(Size(BOX_WALL_THICKNESS, boxSize.height));
    rightWallBody->setDynamic(false);
    rightWallBody->setContactTestBitmask(0xFFFFFFFF);
    rightWall->setPhysicsBody(rightWallBody);
    rightWall->setPosition(boxCenter.x + boxSize.width/2, boxCenter.y);
    this->addChild(rightWall);
    
    auto bottomWall = Node::create();
    auto bottomWallBody = PhysicsBody::createBox(Size(boxSize.width, BOX_WALL_THICKNESS));
    bottomWallBody->setDynamic(false);
    bottomWallBody->setContactTestBitmask(0xFFFFFFFF);
    bottomWall->setPhysicsBody(bottomWallBody);
    bottomWall->setPosition(boxCenter.x, boxCenter.y - boxSize.height/2);
    this->addChild(bottomWall);
    
    auto topWall = Node::create();
    auto topWallBody = PhysicsBody::createBox(Size(boxSize.width, BOX_WALL_THICKNESS));
    topWallBody->setDynamic(false);
    topWallBody->setContactTestBitmask(0xFFFFFFFF);
    topWall->setPhysicsBody(topWallBody);
    topWall->setPosition(boxCenter.x, boxCenter.y + boxSize.height/2);
    this->addChild(topWall);
}

void ShapeGameScene::CreateScoreUI()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    
    // 현재 점수 라벨 생성
    scoreLabel = Label::createWithSystemFont("Score: 0", "Arial", 24);
    scoreLabel->setColor(Color3B::WHITE);
    scoreLabel->setPosition(Vec2(origin.x + 80, origin.y + visibleSize.height - 50));
    this->addChild(scoreLabel, 100);
    
    // 베스트 스코어 라벨 생성
    bestScoreLabel = Label::createWithSystemFont("Best: " + std::to_string(bestScore), "Arial", 20);
    bestScoreLabel->setColor(Color3B::YELLOW);
    bestScoreLabel->setPosition(Vec2(origin.x + 80, origin.y + visibleSize.height - 80));
    this->addChild(bestScoreLabel, 100);
    
}

void ShapeGameScene::AddScore(int points)
{
    score += points;
    UpdateScoreDisplay();
    
    CCLOG("Score added: %d, Total score: %d", points, score);
}

void ShapeGameScene::UpdateScoreDisplay()
{
    if (scoreLabel)
    {
        std::string scoreText = "Score: " + std::to_string(score);
        scoreLabel->setString(scoreText);
    }
}

void ShapeGameScene::LoadBestScore()
{
    // UserDefault에서 베스트 스코어 로드
    bestScore = UserDefault::getInstance()->getIntegerForKey("BestScore", 0);
}

void ShapeGameScene::SaveBestScore()
{
    UserDefault::getInstance()->setIntegerForKey("BestScore", bestScore);
    UserDefault::getInstance()->flush(); // 즉시 저장
}

void ShapeGameScene::UpdateBestScoreDisplay()
{
    if (bestScoreLabel)
    {
        std::string bestScoreText = "Best: " + std::to_string(bestScore);
        bestScoreLabel->setString(bestScoreText);
    }
}

bool ShapeGameScene::IsSpaceAvailable(const Vec2& position)
{
    // 새로운 삼각형의 반지름 (ShapeSprite::createShapeTexture에서 사용하는 공식)
    float shapeRadius = 20.0f; // 삼각형(3-3=0)의 기본 반지름
    
    // 위치 주변에 다른 도형이 있는지 확인
    for (auto shape : shapes)
    {
        if (shape && shape->getParent())
        {
            Vec2 shapePos = shape->getPosition();
            float distance = position.distance(shapePos);
            
            // 다른 도형과의 최소 거리 계산 (두 도형의 반지름 + 여유 공간)
            float otherShapeRadius = 20.0f + (shape->GetSides() - 3) * 3.0f;
            float minDistance = (shapeRadius + otherShapeRadius) * shape->GetShapeScale() + 10.0f; // 10px 여유
            
            // 공간 부족
            if (distance < minDistance)
                return false; 
        }
    }
    
    return true; // 공간 있음
}

void ShapeGameScene::OnGameRestart()
{
    // 게임 상태 초기화
    score = 0;
    
    // 콤보 초기화
    comboSystem->Reset();
    
    // UI 업데이트
    UpdateScoreDisplay();
    
    // 초기 도형들 다시 생성
    CreateInitialShapes();
}

void ShapeGameScene::CreateInitialShapes()
{
    // Start with fewer initial shapes since player can add more by touching
    for (int i = 0; i < 2; i++)
        AddRandomShape();
}

void ShapeGameScene::AddRandomShape()
{
    int level = 3 + (rand() % 3); // 3~5레벨 랜덤
    
    float margin = 60.0f;
    float x = boxCenter.x + (rand() % (int)(boxSize.width - margin)) - (boxSize.width - margin)/2;
    float y = boxCenter.y + boxSize.height/2 - 50;
    Vec2 position(x, y);
    
    auto shape = ShapePool::GetInstance()->GetShape(level, position);

    if (shape)
    {
        this->addChild(shape);
        shapes.push_back(shape);
    }
}

bool ShapeGameScene::OnContactBegin(cocos2d::PhysicsContact& contact)
{
    auto nodeA = contact.getShapeA()->getBody()->getNode();
    auto nodeB = contact.getShapeB()->getBody()->getNode();
    
    auto shapeA = dynamic_cast<ShapeSprite*>(nodeA);
    auto shapeB = dynamic_cast<ShapeSprite*>(nodeB);
    
    if (shapeA && shapeB && shapeA->GetLevel() == shapeB->GetLevel())
    {
        // Check if shapes are still valid and not already being processed
        auto it1 = std::find(shapes.begin(), shapes.end(), shapeA);
        auto it2 = std::find(shapes.begin(), shapes.end(), shapeB);
        
        if (it1 != shapes.end() && it2 != shapes.end())
        {
            // Retain the shapes to prevent deletion during physics processing
            shapeA->retain();
            shapeB->retain();
            
            // Schedule merge to happen on next frame to avoid physics issues
            this->runAction(CallFunc::create([this, shapeA, shapeB](){
                // Check if shapes are still valid before merging
                if (shapeA->getParent() && shapeB->getParent())
                {
                    MergeShapes(shapeA, shapeB);
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

void ShapeGameScene::MergeShapes(ShapeSprite* shape1, ShapeSprite* shape2)
{
    if (!shape1 || !shape2) 
        return;

    if (!shape1->getParent() || !shape2->getParent())
        return;

    if (!shape1->getPhysicsBody() || !shape2->getPhysicsBody()) 
        return;
    
    // Double check that shapes are still in our container
    auto it1 = std::find(shapes.begin(), shapes.end(), shape1);
    auto it2 = std::find(shapes.begin(), shapes.end(), shape2);
    
    if (it1 == shapes.end() || it2 == shapes.end()) 
        return;
    
    int newLevel = shape1->GetLevel() + 1; // 레벨 기반 계산
    float newScale = 1.0f;
    
    // 30레벨을 넘어서면 크기로 구분 (무한 확장)
    if (newLevel > 30) 
    {
        // 31레벨부터는 크기를 증가시켜서 구분
        float scale1 = shape1->GetShapeScale();
        float scale2 = shape2->GetShapeScale();

        newScale = (scale1 + scale2) * 0.55f; // 점진적 크기 증가
    }
    
    // Calculate merge position as the midpoint between the two shapes
    Vec2 mergePos = (shape1->getPosition() + shape2->getPosition()) * 0.5f;
    
    // Store the velocity of the shapes to apply to the new merged shape
    Vec2 velocity1 = shape1->getPhysicsBody()->getVelocity();
    Vec2 velocity2 = shape2->getPhysicsBody()->getVelocity();
    Vec2 averageVelocity = (velocity1 + velocity2) * 0.5f;
    
    // Remove from vector using erase-remove idiom
    shapes.erase(std::remove(shapes.begin(), shapes.end(), shape1), shapes.end());
    shapes.erase(std::remove(shapes.begin(), shapes.end(), shape2), shapes.end());
    
    // 기존 도형들을 풀로 반환
    ShapePool::GetInstance()->ReturnShape(shape1);
    ShapePool::GetInstance()->ReturnShape(shape2);
    
    // Create new shape at the merge position using pool
    auto newShape = ShapePool::GetInstance()->GetShape(newLevel, mergePos);

    if (newShape)
    {
        // 30레벨 이후 스케일 적용
        if (newLevel > 30)
        {
            newShape->SetShapeScale(newScale);
            newShape->CreateShapeTexture();
            newShape->SetupPhysicsBody();
        }
        
        this->addChild(newShape);
        
        // Apply the average velocity to the new shape for more natural physics
        newShape->getPhysicsBody()->setVelocity(averageVelocity * 0.8f); // Slight dampening
        
        shapes.push_back(newShape);
    }
    
    // 콤보 업데이트
    comboSystem->Update();
    
    // 레벨에 따른 점수 획득 (무한 확장 가능)
    int baseScore = newLevel;
    int bonusScore = comboSystem->CalculateBonus(baseScore);

    AddScore(baseScore + bonusScore);
    
    CCLOG("Base score: %d, Combo: %d, Bonus: %d, Total: %d", baseScore, comboSystem->GetComboCount(), bonusScore, baseScore + bonusScore);
}

void ShapeGameScene::OnAcceleration(cocos2d::Acceleration* acc, cocos2d::Event* event)
{
    // 디버깅을 위한 가속도계 값 출력
    CCLOG("Acceleration: x=%.3f, y=%.3f, z=%.3f", acc->x, acc->y, acc->z);
    
    float gravityStrength = 500.0f;
    
    // 좌우와 앞뒤 방향 모두 반전 적용
    float gravityX = acc->x * gravityStrength;  // 좌우 기울임 = X축 (방향 반전)
    float gravityY = acc->y * gravityStrength;  // 앞뒤 기울임 = Y축 (방향 반전)
    
    Vec2 gravity = Vec2(gravityX, gravityY);
    
    if (world)
    {
        world->setGravity(gravity);
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

void ShapeGameScene::AddShapeAtPosition(const Vec2& position)
{
    // 게임 오버 상태면 아무것도 하지 않음
    if (gameStateManager->IsGameOver())
        return; 
    
    // Check if position is within the box bounds
    float margin = 30.0f;

    if (position.x < boxCenter.x - boxSize.width/2 + margin || 
        position.x > boxCenter.x + boxSize.width/2 - margin ||
        position.y < boxCenter.y - boxSize.height/2 + margin || 
        position.y > boxCenter.y + boxSize.height/2 - margin)
    {
        return; // Don't create shape outside the box
    }
    
    // 공간이 있는지 확인
    if (!IsSpaceAvailable(position))
    {
        // 공간이 없으면 게임 오버
        bool isNewRecord = false;

        if (score > bestScore)
        {
            bestScore = score;
            SaveBestScore();
            UpdateBestScoreDisplay();
            isNewRecord = true;
        }

        gameStateManager->GameOver(score, bestScore, isNewRecord);

        return;
    }
    
    // Always create a triangle (level 3) when player touches
    auto shape = ShapePool::GetInstance()->GetShape(3, position);

    if (shape)
    {
        this->addChild(shape);
        shapes.push_back(shape);
    }
    
    // 터치로 도형 생성 시에는 콤보를 리셋 (터치로는 콤보 불가)
    comboSystem->Reset();
    
    // 3각형 생성 시 3점 획득 (콤보 보너스 없음)
    AddScore(3);
}

bool ShapeGameScene::OnTouchBegan(Touch* touch, Event* event)
{
    Vec2 touchLocation = touch->getLocation();
    
    // 게임 상태 관리자에서 터치 처리
    if (gameStateManager->HandleTouchForRestart(touchLocation))
        return true;
    
    AddShapeAtPosition(touchLocation);

    return true;
}