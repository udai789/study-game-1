//
//  Fune.cpp
//  pirateTaxtics
//
//  Created by kk on 2015/05/06.
//
//

#include "Fune.h"

USING_NS_CC;

//アニメーションが何フレームあるか
const int FRAME_COUNT=8;
//通常状態のフレーム数
const int NORMAL_FRAME_COUNT=4;
//キャラ数
const int CHARACTER_COUNT=8;

Fune::Fune()
:_hpGauge(nullptr)
{
    
}

Fune::~Fune()
{
    CC_SAFE_RELEASE_NULL(_hpGauge);
}

Fune* Fune::createWithLevel(Fune::CharacterTypes type)
{
    Fune* ret=new Fune();
    if(ret->initWithLevel(type)){
        ret->autorelease();
        return ret;
    }
    
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool Fune::initWithLevel(Fune::CharacterTypes type)
{
    if(!Sprite::initWithFile("images/ships.png")){
        return false;
    }
    
    //キャラクター番号を格納
    _type=type;
    
    ///ステータスを設定
    switch (type) {
        case CharacterTypes::PC_CAPTAIN:
        case CharacterTypes::ENEMY_BOSS:
            _movement=4;
            _range=3;
            _attack=100;
            _defense=50;
            _maxHp=120;
            _hp=_maxHp;
            break;
            
        case CharacterTypes::PC_SPEED:
        case CharacterTypes::ENEMY_SPEED:
            _movement=5;
            _range=3;
            _attack=80;
            _defense=60;
            _maxHp=80;
            _hp=_maxHp;
            break;
            
        case CharacterTypes::PC_DEFENSE:
        case CharacterTypes::ENEMY_DEFENSE:
            _movement=3;
            _range=3;
            _attack=80;
            _defense=60;
            _maxHp=240;
            _hp=_maxHp;
            break;
            
        case CharacterTypes::PC_ATTACK:
        case CharacterTypes::ENEMY_ATTACK:
            _movement=3;
            _range=3;
            _attack=120;
            _defense=40;
            _maxHp=150;
            _hp=_maxHp;
            break;
            
        default:
            break;
    }
    //*/ステータスを設定
    
    ////キャラクターの基本アニメーション作成
    //1フレームの画面サイズを取得する
    auto frameSize=Size(this->getContentSize().width/FRAME_COUNT,this->getContentSize().height/CHARACTER_COUNT);
    //テクスチャの大きさを1フレーム分にする
    this->setTextureRect(Rect(0,0,frameSize.width,frameSize.height));
    
    Vector<SpriteFrame*> charcterFrames;//キャラクターのフレームを格納
    Vector<SpriteFrame*> frames;//キャラクターの基本アニメーションをフレーム順に格納
    
    auto level=static_cast<int>(type);
    for(int i=0;i<NORMAL_FRAME_COUNT;i++){
        auto frame=SpriteFrame::create("images/ships.png",Rect(frameSize.width*i,frameSize.height*level,
                                                               frameSize.width,frameSize.height));
        charcterFrames.pushBack(frame);
    }
    
    frames.pushBack(charcterFrames.at(0));
    frames.pushBack(charcterFrames.at(1));
    frames.pushBack(charcterFrames.at(2));
    frames.pushBack(charcterFrames.at(1));
    frames.pushBack(charcterFrames.at(0));
    frames.pushBack(charcterFrames.at(3));
    
    auto animation=Animation::createWithSpriteFrames(frames);
    animation->setDelayPerUnit(0.3);
    this->runAction(RepeatForever::create(Animate::create(animation)));
    
    /*
    frames.pushBack(SpriteFrame::create("images/ships.png",Rect(frameSize.width,frameSize.height*level,
                                                                frameSize.width,frameSize.height)));
    frames.pushBack(SpriteFrame::create("images/ships.png",Rect(frameSize.width*2,frameSize.height*level,
                                                                frameSize.width,frameSize.height)));
    frames.pushBack(SpriteFrame::create("images/ships.png",Rect(frameSize.width,frameSize.height*level,
                                                                frameSize.width,frameSize.height)));
    frames.pushBack(SpriteFrame::create("images/ships.png",Rect(0,frameSize.height*level,
                                                                frameSize.width,frameSize.height)));
    frames.pushBack(SpriteFrame::create("images/ships.png",Rect(frameSize.width*3,frameSize.height*level,
                                                                frameSize.width,frameSize.height)));
     */
    
    /*
     for(int i=0;i<NORMAL_FRAME_COUNT;i++){
     //1コマずつアニメーションを作成する
     auto frame=SpriteFrame::create("images/ships.png",Rect(frameSize.width*i,frameSize.height*level,
     frameSize.width,frameSize.height));
     frames.pushBack(frame);
     }*/
    /*
    Vector<AnimationFrame*> frames;
    ValueMap userInfo;
    userInfo["size"]=Value("0");
    auto spriteFrame0=SpriteFrame::create("images/ships.png",Rect(0,frameSize.height*level,
                                                     frameSize.width,frameSize.height));
    auto animationFrame0=AnimationFrame::create(spriteFrame0,4,userInfo);
    frames.pushBack(animationFrame0);
    auto spriteFrame1=SpriteFrame::create("images/ships.png",Rect(frameSize.width,frameSize.height*level,
                                                                  frameSize.width,frameSize.height));
    auto animationFrame1=AnimationFrame::create(spriteFrame1,3,userInfo);
    frames.pushBack(animationFrame1);
    auto spriteFrame2=SpriteFrame::create("images/ships.png",Rect(frameSize.width*2,frameSize.height*level,
                                                                  frameSize.width,frameSize.height));
    auto animationFrame2=AnimationFrame::create(spriteFrame2,2,userInfo);
    frames.pushBack(animationFrame2);
    auto animationFrame3=AnimationFrame::create(spriteFrame1,2,userInfo);
    frames.pushBack(animationFrame3);
    auto animationFrame4=AnimationFrame::create(spriteFrame0,4,userInfo);
    frames.pushBack(animationFrame4);
    auto spriteFrame3=SpriteFrame::create("images/ships.png",Rect(frameSize.width*3,frameSize.height*level,
                                                                  frameSize.width,frameSize.height));
    auto animationFrame5=AnimationFrame::create(spriteFrame3,3,userInfo);
    frames.pushBack(animationFrame5);
    
    auto animation=Animation::create(frames,1);*/
    ////*/キャラクターの基本アニメーション作成
    
    ///HPゲージの作成
    auto hpBack=Sprite::create("images/healthBack.png");
    hpBack->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
    this->addChild(hpBack);
    
    auto hpRed=Sprite::create("images/healthRed.png");
    hpRed->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
    this->addChild(hpRed);
    
    auto hpGreen=Sprite::create("images/healthGreen.png");
    hpGreen->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
    this->addChild(hpGreen);
    this->setHpGauge(hpGreen);
    //*/HPゲージの作成
    
    return true;
}

bool Fune::withinRange(int distance)
{
    return distance<=_range;
}

void Fune::updateHpGauge()
{
    auto hpGagueWidthRate=static_cast<float>(std::max(_hp,0))/static_cast<float>(_maxHp);//hpゲージの長さ 最低値は0
    _hpGauge->setScale(hpGagueWidthRate,1);
}


void Fune::sinkShip(CallFunc* callfunc)
{
    Vector<SpriteFrame*> sinkFrames;
    auto frameSize=this->getContentSize();
    
    auto level=static_cast<int>(_type);
    for(int i=NORMAL_FRAME_COUNT;i<FRAME_COUNT;i++){
        auto frame=SpriteFrame::create("images/ships.png",Rect(frameSize.width*i,frameSize.height*level,
                                                               frameSize.width,frameSize.height));
        sinkFrames.pushBack(frame);
    }
    
    auto animation=Animation::createWithSpriteFrames(sinkFrames,15.0/60.0);
    this->stopAllActions();
    this->runAction(Sequence::create(Animate::create(animation),
                                     callfunc,
                                     NULL));
}