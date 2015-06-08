//
//  Title.cpp
//  pirateTaxtics
//
//  Created by kk on 2015/06/02.
//
//

#include "Title.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"
#include "SimpleAudioEngine.h"

USING_NS_CC;


Title::Title(){
    
}

Title::~Title(){
}

Scene* Title::createScene(){
    auto scene=Scene::create();
    auto layer=Title::create();
    scene->addChild(layer);
    return scene;
}

bool Title::init()
{
    if(!Layer::init()){
        return false;
    }
    
    auto winSize=Director::getInstance()->getWinSize();
    
    auto frame=Sprite::create("images/mapframe.png");
    frame->setPosition(Vec2(winSize.width/2,winSize.height/2));
    this->addChild(frame);
    
    auto background=Sprite::create("images/map00.png");
    background->setPosition(Vec2(winSize.width/2,winSize.height/2));
    this->addChild(background);
    
    auto layer=LayerColor::create();
    layer->setColor(Color3B::BLACK);
    layer->setOpacity(64);
    this->addChild(layer);
    
    //アプリケーション終了ボタン プラットフォームがウィンドウズ系の場合は終了ボタンを設置しない
#if !(CC_TARGET_PLATFORM == CC_PLATFORM_WP8) || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
    auto endButton=MenuItemImage::create("images/settings.png","images/settings.png",
                                         [this](Ref* ref){
                                             this->getEventDispatcher()->removeAllEventListeners();//全イベントの無効化
                                             //全アクションの停止
                                             Director::getInstance()->getActionManager()->pauseAllRunningActions();
                                             Director::getInstance()->getActionManager()->removeAllActions();
                                             //全BGM・効果音の停止
                                             CocosDenshion::SimpleAudioEngine::getInstance()->stopBackgroundMusic();
                                             CocosDenshion::SimpleAudioEngine::getInstance()->stopAllEffects();
                                             
                                             this->scheduleOnce([this](float dt){
                                                 this->runAction(Sequence::create(ScaleTo::create(0.5,0),
                                                                                  CallFunc::create([this](){
                                                     Director::getInstance()->end();//アプリケーションを終了する
                                                     if(CC_TARGET_PLATFORM==CC_PLATFORM_IOS){//プレットフォームがiosの場合の処理
                                                         exit(0);
                                                     }
                                                 }),NULL));
                                             },0.5,"ENDACTION");
                                             
                                         });
    
    endButton->runAction(RepeatForever::create(RotateBy::create(10,360)));
    endButton->setScale(1.5);
    auto menu=Menu::create(endButton,NULL);
    menu->setPosition(Vec2(winSize.width-100,100));
    
    auto label=Label::createWithSystemFont("GAME END","Arial",28);
    label->setColor(Color3B::ORANGE);
    label->enableOutline(Color4B::BLACK);
    label->enableShadow(Color4B::BLACK,Size(0.5,0.5),3);
    label->setPosition(Vec2(winSize.width-100,100));
    
    layer->addChild(menu);
    layer->addChild(label);
#endif
    //アプリーケーション終了ボタン
    
    this->createTitleLayer();
    
    return true;
}

void Title::onEnterTransitionDidFinish(){
    Layer::onEnterTransitionDidFinish();
}

void Title::createTitleLayer()
{
    auto window=CSLoader::getInstance()->createNode("TItleLayer.csb");
    
    auto vsButton=window->getChildByName<ui::Button*>("vsButton");
    vsButton->addTouchEventListener([this,window](Ref* pButton,ui::Widget::TouchEventType type){
        auto button=dynamic_cast<ui::Button*>(pButton);
        switch(type){
            case cocos2d::ui::Widget::TouchEventType::BEGAN:
                button->setColor(Color3B::GRAY);
                break;
                
            case cocos2d::ui::Widget::TouchEventType::ENDED:
                window->removeFromParent();
                CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sound/se2.wav");
                this->createVSLayer();
                break;
                
            case cocos2d::ui::Widget::TouchEventType::CANCELED:
                button->setColor(Color3B::WHITE);
                break;
                
            default:
                break;
        }
    });
    
    auto storyButton=window->getChildByName<ui::Button*>("storyButton");
    storyButton->setVisible(false);//現状消しておく
    
    this->addChild(window);
}

void Title::createVSLayer()
{
    auto window=CSLoader::getInstance()->createNode("VsLayer.csb");
    
    auto humanButton=window->getChildByName<ui::Button*>("humanButton");
    humanButton->addTouchEventListener([this](Ref* pButton,ui::Widget::TouchEventType type){
        auto button=dynamic_cast<ui::Button*>(pButton);
        
        if(type==ui::Widget::TouchEventType::BEGAN){
            button->setColor(Color3B::GRAY);
        }else if(type==ui::Widget::TouchEventType::CANCELED){
            button->setColor(Color3B::WHITE);
        }else if(type==ui::Widget::TouchEventType::ENDED){
            CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sound/se2.wav");
            this->turnScene(GameInitialise::create(0,false,true));
        }
    });
    
    auto CPUButton=window->getChildByName<ui::Button*>("CPUButton");
    CPUButton->addTouchEventListener([this](Ref* pButton,ui::Widget::TouchEventType type){
        auto button=dynamic_cast<ui::Button*>(pButton);
        
        if(type==ui::Widget::TouchEventType::BEGAN){
            button->setColor(Color3B::GRAY);
        }else if(type==ui::Widget::TouchEventType::CANCELED){
            button->setColor(Color3B::WHITE);
        }else if(type==ui::Widget::TouchEventType::ENDED){
            CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sound/se2.wav");
            this->turnScene(GameInitialise::create(0,true,true));
        }
    });
    
    this->addChild(window);
}

void Title::turnScene(GameInitialise *gameInitialise)
{
    gameInitialise->retain();//retainしておく
    this->getEventDispatcher()->removeAllEventListeners();//イベントを全て無効化
    this->runAction(Sequence::create(DelayTime::create(0.3),
                                     CallFunc::create([this,gameInitialise]{
        this->scheduleOnce([](float dt){
            CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sound/se4.wav");
        },0.3,"TurnScene");
        auto scene=SceneGameMain::createSceneWithGameInitialise(gameInitialise);
        gameInitialise->release();//retainした分release;
        auto transition=TransitionSplitCols::create(2,scene);
        Director::getInstance()->replaceScene(transition);
    }),NULL));
}