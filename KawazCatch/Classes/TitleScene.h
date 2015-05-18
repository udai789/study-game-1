//
//  TitleScene.h
//  KawazCatch
//
//  Created by kk on 2015/04/16.
//
//

#ifndef __KawazCatch__TitleScene__
#define __KawazCatch__TitleScene__

#include <stdio.h>
#include "cocos2d.h"

class TitleScene :public cocos2d::Layer{
protected:
    TitleScene();
    virtual ~TitleScene();
    bool init() override;
    
public:
    static cocos2d::Scene* createScene();
    void onEnterTransitionDidFinish() override;
    CREATE_FUNC(TitleScene);
};

#endif /* defined(__KawazCatch__TitleScene__) */
