//
//  MainScene.h
//  KawazCatch
//
//  Created by kk on 2015/04/14.
//
//

#ifndef __KawazCatch__MainScene__
#define __KawazCatch__MainScene__

#include <stdio.h>
#include <random>
#include "cocos2d.h"

class MainScene :public cocos2d::Layer
{
private:
    //フルーツの種類を表します
    enum class FruitType{
        APPLE,
        GRAPE,
        ORANGE,
        BANANA,
        CHERRY,
        GOLDEN,
        BOMB,
        COUNT//最大値
    };
    
    /*ゲームの状態を表します
     READY:開始前
     PLAYING:プレイ中
     ENDING:終了演出中
     RESULT:スコア表示
     */
    enum class GameState{
        READY,//開始前
        PLAYING,//ゲーム中
        ENDING,//終了演出中
        RESULT//スコア表示
    };
    
    /*画面にフルーツを新たに配置して、それを返します
     @return 新たに作成されたフルーツ
     */
    cocos2d::Sprite* addFruit();
    /*マップからフルーツを取り除きます
     @param fruit 削除するフルーツ
     @return 正しく削除されたか
     */
    bool removeFruit(cocos2d::Sprite* fruit);
    
    /*フルーツを取得します
     @param Sprite* 取得するフルーツ
     */
    void catchFruit(cocos2d::Sprite* fruit);
    
    /*ゲーム開始の文字を追加します
     */
    void addReadyLabel();
    
    
    /*ゲームが終了したときに呼び出されます
     */
    void onResult();
    
    /*爆弾を取ってしまったときの呼び出されます
     */
    void onCatchBomb();
    
    /*min ~ maxの乱数をfloatで返します
     @param min 最小値
     @param max 最大値
     @return min <= n < maxの整数値
     */
    float generateRandom(float min,float max);
    
protected:
    MainScene();
    virtual ~MainScene();
    bool init() override;
    
public:
    static cocos2d::Scene* createScene();
    void update(float dt);
    void onEnterTransitionDidFinish() override;
    CREATE_FUNC(MainScene);
    //_player変数と、getPlayer()メソッド、setPlayer(Sprite *)メソッドが自動的に実装される
    CC_SYNTHESIZE(cocos2d::Vector<cocos2d::Sprite*>, _fruits, Fruits);
    CC_SYNTHESIZE(int, _score, Score);
    CC_SYNTHESIZE(int, _highScore, HighScore);
    CC_SYNTHESIZE(float, _second, Second);
    CC_SYNTHESIZE(bool, _isCrash, IsCrash);
    CC_SYNTHESIZE(GameState, _state, State);
    CC_SYNTHESIZE(std::mt19937,_engine,Engine);
    CC_SYNTHESIZE_RETAIN(cocos2d::Sprite*, _player, Player);
    CC_SYNTHESIZE_RETAIN(cocos2d::Label*, _scoreLabel, ScoreLabel);
    CC_SYNTHESIZE_RETAIN(cocos2d::Label*,_highScoreLabel,HighScoreLabel);
    CC_SYNTHESIZE_RETAIN(cocos2d::Label*, _secondLabel, SecondLabel);
    CC_SYNTHESIZE_RETAIN(cocos2d::SpriteBatchNode*,_fruitsBatchNode,FruitsBatchNode);
};

#endif /* defined(__KawazCatch__MainScene__) */
