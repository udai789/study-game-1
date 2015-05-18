//
//  Player.h
//  pirateTaxtics
//
//  Created by kk on 2015/05/13.
//
//

#ifndef __pirateTaxtics__Player__
#define __pirateTaxtics__Player__

#include <stdio.h>
#include "cocos2d.h"
#include "Fune.h"

class Player :public cocos2d::Ref
{
protected:
    Player();
    virtual ~Player();
    bool init();
    
public:
    CC_SYNTHESIZE(cocos2d::Vector<Fune*>,_funeList,FuneList);//プレイヤーに属する船
    CC_SYNTHESIZE_RETAIN(Fune*,_activeFune,ActiveFune);//プレイヤーが選択中の船
    CREATE_FUNC(Player);
    
    /*船をプレイヤーに所属させる
     *@param fune このプレイヤーに持たせる船
     */
    void addFuneList(Fune* fune);
    
    /*リストから船を削除
     *@param fune _funelistから削除する船
     */
    void removeFuneList(Fune* fune);
    
    /*指定された番号の船を返す
     *@param index list番号
     *@return 番号の船
     */
    Fune* getFune(int index);
    
    /*現在選択中の船を返す。選択していなければ_funeListの最初の船を返す
     *@return 洗濯中の船
     */
    Fune* getActiveFuneA();
    
    /*所属する船の数を返す
     *@return 船の数
     */
    int getFuneListCount();
    
    /*プレイヤーが所持する船か
     *@param 調べる船
     *@return true:プレイヤーの船
     */
    bool getFuneListContains(Fune* fune);
};

#endif /* defined(__pirateTaxtics__Player__) */
