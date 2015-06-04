//
//  Fune.h
//  pirateTaxtics
//
//  Created by kk on 2015/05/06.
//
//

#ifndef __pirateTaxtics__Fune__
#define __pirateTaxtics__Fune__

#include <stdio.h>
#include "cocos2d.h"

class Fune :public cocos2d::Sprite
{
public:
    //キャラクター識別　キャラクター番号と同じ
    enum class CharacterTypes{
        PC_CAPTAIN,//キャプテン 0
        PC_SPEED,//味方スピードタイプ 1
        PC_DEFENSE,//味方防御タイプ 2
        PC_ATTACK,//味方攻撃タイプ 3
        ENEMY_BOSS,//敵ボスタイプ 4
        ENEMY_SPEED,//敵スピードタイプ 5
        ENEMY_DEFENSE,//敵防御タイプ 6
        ENEMY_ATTACK,//敵攻撃タイプ 7
        COUNT//キャラ数
    };
    
    //キャラクター番号
    CC_SYNTHESIZE_READONLY(CharacterTypes,_type,Type);
    
    CC_SYNTHESIZE_READONLY(int,_movement,Movement);//船の移動力
    CC_SYNTHESIZE_READONLY(int,_range,Range);//攻撃距離
    CC_SYNTHESIZE_READONLY(int,_attack,Attack);//攻撃力
    CC_SYNTHESIZE_READONLY(int,_defense,Defense);//防御力
    CC_SYNTHESIZE_READONLY(int,_maxHp,MaxHP);//最大HP
    CC_SYNTHESIZE_READONLY(int,_hp,Hp);//現在のHP
    CC_SYNTHESIZE_READONLY(int,_skillCount,SkillCount);//スキル使用可能回数 1で初期化
    
    CC_SYNTHESIZE(bool,_isInvincible,IsInvincible);//無敵状態かどうか
    
    CC_SYNTHESIZE_RETAIN(cocos2d::Sprite*,_hpGauge,HpGauge);//現在のHPを表すゲージ
    
    //船のタイルマップ上の位置
    CC_SYNTHESIZE(cocos2d::Vec2,_tiledMapPosition,TiledMapPosition);
    
    /*キャラクタータイプから船を生成します
     *@param type キャラクター識別 pngで一番上が0 下が7
     *@return 船
     */
    static Fune* createWithLevel(CharacterTypes type);
    
    /*相手の船が攻撃範囲内か
     *@param distance 対象との距離
     *@return true:攻撃可
     */
    bool withinRange(int distance);
    
    void updateHpGauge();//hpゲージを更新する
    
    /*hpを変化させる
     *@param damage hpの変化値 回復する場合は値を負の値にする
     */
    void receiveDamage(int damage);
    
    void useSkill(){_skillCount-=1;};//スキルを使用した時に呼び出す スキル使用回数を1減らす
    
    /*沈没表現
     *@param アニメーション終了時の処理
     */
    void sinkShip(cocos2d::CallFunc* callfunc);//沈没表現
    
protected:
    Fune();
    virtual ~Fune();
    bool initWithLevel(CharacterTypes type);
};
#endif /* defined(__pirateTaxtics__Fune__) */
