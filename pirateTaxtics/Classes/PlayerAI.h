//
//  PlayerAI.h
//  pirateTaxtics
//
//  Created by kk on 2015/06/04.
//
//

//プレイヤーの操作(船の移動、攻撃、スキル使用を代行するクラス)
#ifndef __pirateTaxtics__PlayerAI__
#define __pirateTaxtics__PlayerAI__

#include <stdio.h>
#include "cocos2d.h"
#include "SceneGameMain.h"
#include "Player.h"

class PlayerAI :public cocos2d::Ref
{
protected:
    PlayerAI();
    virtual ~PlayerAI();
    bool init();
    
public:
    //PlayerAIを作成する
    //@param player 行動を代行するプレイヤー
    PlayerAI* create(SceneGameMain*,Player* player);
};
#endif /* defined(__pirateTaxtics__PlayerAI__) */
