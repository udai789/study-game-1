//
//  Fune.cpp
//  pirateTaxtics
//
//  Created by kk on 2015/05/06.
//
//

#include "Fune.h"
#include "json/rapidjson.h"
#include "json/document.h"

USING_NS_CC;

//アニメーションが何フレームあるか
const int FRAME_COUNT=8;
//通常状態のフレーム数
const int NORMAL_FRAME_COUNT=4;

Fune::Fune()
:_isInvincible(false)
,_hpGauge(nullptr)
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
    auto typeString=Fune::convertString(type);
    auto jsonData=FileUtils::getInstance()->getStringFromFile("json/UnitData.json");
    rapidjson::Document docJson;
    docJson.Parse<rapidjson::kParseDefaultFlags>(jsonData.data());
    if(docJson.HasParseError()){return false;}//parseに失敗
    
    if(!Sprite::initWithFile("images/ships.png")){
        return false;
    }
    
    //キャラクター番号を格納
    _type=type;
    
    ///ステータスを設定
    _movement=docJson[typeString.data()]["movement"].GetInt();
    _range=docJson[typeString.data()]["range"].GetInt();
    _attack=docJson[typeString.data()]["attack"].GetInt();
    _defense=docJson[typeString.data()]["defense"].GetInt();
    _maxHp=docJson[typeString.data()]["maxHP"].GetInt();
    _hp=_maxHp;
    _skillCount=docJson[typeString.data()]["skillCount"].GetInt();
    //*/ステータスを設定
    
    ////キャラクターの基本アニメーション作成
    //1フレームの画面サイズを取得する
    auto frameSize=Size(this->getContentSize().width/FRAME_COUNT,
                        this->getContentSize().height/static_cast<float>(Fune::CharacterTypes::COUNT));
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

std::string Fune::convertString(Fune::CharacterTypes type)
{
    std::string string;
    switch(type){
        case CharacterTypes::PC_CAPTAIN:
            string="PC_CAPTAIN";
            break;
            
        case CharacterTypes::PC_DEFENSE:
            string="PC_DEFENSE";
            break;
            
        case CharacterTypes::PC_SPEED:
            string="PC_SPEED";
            break;
            
        case CharacterTypes::PC_ATTACK:
            string="PC_ATTACK";
            break;
            
        case CharacterTypes::ENEMY_BOSS:
            string="ENEMY_BOSS";
            break;
            
        case CharacterTypes::ENEMY_DEFENSE:
            string="ENEMY_DEFENSE";
            break;
            
        case CharacterTypes::ENEMY_SPEED:
            string="ENEMY_SPEED";
            break;
            
        case CharacterTypes::ENEMY_ATTACK:
            string="ENEMY_ATTACK";
            break;
            
        default:
            string="ERROR NOT SETTING TYPE";
            break;
    }
    
    return string;
}

Fune::CharacterTypes Fune::convertType(const std::string string)
{
    Fune::CharacterTypes type;
    if(string=="PC_CAPTAIN"){
        type=CharacterTypes::PC_CAPTAIN;
    }else if(string=="PC_DEFENSE"){
        type=CharacterTypes::PC_DEFENSE;
    }else if(string=="PC_SPEED"){
        type=CharacterTypes::PC_SPEED;
    }else if(string=="PC_ATTACK"){
        type=CharacterTypes::PC_ATTACK;
    }else if(string=="ENEMY_BOSS"){
        type=CharacterTypes::ENEMY_BOSS;
    }else if(string=="ENEMY_DEFENSE"){
        type=CharacterTypes::ENEMY_DEFENSE;
    }else if(string=="ENEMY_SPEED"){
        type=CharacterTypes::ENEMY_SPEED;
    }else if(string=="ENEMY_ATTACK"){
        type=CharacterTypes::ENEMY_ATTACK;
    }else{
        log("Fune.cpp Fune::convertString Error string is not CharacterTypes");
        type=CharacterTypes::COUNT;
    }
    
    return type;
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

void Fune::receiveDamage(int damage)
{
    auto hp=_hp-damage;
    hp=std::max(hp,0);
    hp=std::min(hp,_maxHp);
    _hp=hp;
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
                                     RemoveSelf::create(),
                                     callfunc,
                                     NULL));
}