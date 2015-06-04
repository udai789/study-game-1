//
//  Player.cpp
//  pirateTaxtics
//
//  Created by kk on 2015/05/13.
//
//

#include "Player.h"

USING_NS_CC;

Player::Player()
:_activeFune(nullptr)
{
    
}

Player::~Player()
{
    CC_SAFE_RELEASE_NULL(_activeFune);
}

Player* Player::create(bool isCPU)
{
    Player* ret=new Player();
    if(ret->init(isCPU)){
        ret->autorelease();
        return ret;
    }
    
    CC_SAFE_RELEASE_NULL(ret);
    return nullptr;
}

//bool Player::init()
bool Player::init(bool isCPU)
{
    _isCPU=isCPU;
    return true;
}

void Player::addFuneList(Fune *fune)
{
    _funeList.pushBack(fune);
}

void Player::removeFuneList(Fune *fune)
{
    _funeList.eraseObject(fune);
    if(fune==_activeFune){
        this->setActiveFune(nullptr);
    }
}

Fune* Player::getFune(int index)
{
    return _funeList.at(index);
}

Fune* Player::getActiveFuneA()
{
    auto fune=_activeFune;
    if(!fune){
        this->setActiveFune(this->getFune(0));
        fune=_activeFune;
    }
    
    return fune;
}

int Player::getFuneListCount()
{
    return static_cast<int>(_funeList.size());
}

bool Player::getFuneListContains(Fune *fune){
    return _funeList.contains(fune);
}