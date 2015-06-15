//
//  Title.h
//  pirateTaxtics
//
//  Created by kk on 2015/06/02.
//
//

#ifndef __pirateTaxtics__Title__
#define __pirateTaxtics__Title__

#include <stdio.h>
#include "cocos2d.h"
#include "SceneGameMain.h"

class Title :public cocos2d::Layer
{
protected:
    Title();
    virtual ~Title();
    bool init() override;
    
    void createTitleLayer();//タイトル画面を作成する
    void createVSLayer();//対戦選択画面を作成
    void createStoryLayer();//ストーリーモード選択画面を作成
    
    /*SceneGameMainにシーン遷移する
     *@param gameInitialise ゲームの初期化設定
     */
    void turnScene(GameInitialise* gameInitialise);
    
public:
    static cocos2d::Scene* createScene();
    void onEnterTransitionDidFinish() override;
    CREATE_FUNC(Title);
};

#endif /* defined(__pirateTaxtics__Title__) */
