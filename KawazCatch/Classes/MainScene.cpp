//
//  MainScene.cpp
//  KawazCatch
//
//  Created by kk on 2015/04/14.
//
//

#include "SimpleAudioEngine.h"
#include "MainScene.h"
#include "TitleScene.h"

USING_NS_CC;

//フルーツの画面上端からのマージン(px)
const int FRUIT_TOP_MARGIN=40;
//フルーツの出現率
const int FRUIT_SPAWN_RATE=20;
//制限時間
const float TIME_LIMIT_SECOND=20;
//黄金のフルーツを取ったときの点数
const int GOLDEN_FRUIT_SCORE=5;
//爆弾を取ったときのマイナス点
const int BOMB_PENALTY_SCORE=4;
//黄金のフルーツが出る確率の初期値
const float GOLDEN_FRUIT_PROBABILITY_BASE=0.02;
//爆弾が出る確率の初期値
const float BOMB_PROBABILITY_BASE=0.05;
//黄金のフルーツが出る確率の増え幅
const float GOLDEN_FRUIT_PROBABILITY_RATE=0.001;
//爆弾が出る確率の増え幅
const float BOMB_PROBABILITY_RATE=0.003;
//普通のフルーツの数
const int NORMAL_FRUIT_COUNT=5;

//フルーツの出現頻度の初期値
const float FRUIT_SPAWN_INCREASE_BASE=0.02;
//フルーツ出現頻度の増加率
const float FRUIT_SPAWN_INCREASE_RATE=1.05f;
//フルーツ出現頻度の最大値
const float MAXIMUM_SPAWN_PROBABILITY=0.5;

//ハイスコア保存用のキー
const char* HIGHSCORE_KEY="highscoreKey";

MainScene::MainScene()
:_score(0),
_highScore(0),
_second(TIME_LIMIT_SECOND),
_isCrash(false),
_state(GameState::READY),
_player(NULL),
_scoreLabel(NULL),
_highScoreLabel(NULL),
_secondLabel(NULL),
_fruitsBatchNode(NULL)
{
    //乱数の初期化
    std::random_device rdev;
    _engine.seed(rdev());
}

MainScene::~MainScene(){
    //デストラクタ
    CC_SAFE_RELEASE_NULL(_player);
    CC_SAFE_RELEASE_NULL(_scoreLabel);
    CC_SAFE_RELEASE_NULL(_highScoreLabel);
    CC_SAFE_RELEASE_NULL(_secondLabel);
    CC_SAFE_RELEASE_NULL(_fruitsBatchNode);
}

Scene* MainScene::createScene(){
    auto scene=Scene::create();
    auto layer=MainScene::create();
    scene->addChild(layer);
    return scene;
}

float MainScene::generateRandom(float min,float max){
    std::uniform_real_distribution<float> dest(min,max);
    return dest(_engine);
}

Sprite* MainScene::addFruit(){
    //画面サイズを取り出す
    auto winSize=Director::getInstance()->getWinSize();
    //フルーツの種類を選択する
    //int fruitType=rand()%static_cast<int>(FruitType::COUNT);
    int fruitType=0;
    float r=this->generateRandom(0,1);
    int pastSecond=TIME_LIMIT_SECOND-_second;//経過時間
    float goldenFruitProbability=GOLDEN_FRUIT_PROBABILITY_BASE+GOLDEN_FRUIT_PROBABILITY_RATE*pastSecond;
    float bombProbability=BOMB_PROBABILITY_BASE+BOMB_PROBABILITY_RATE*pastSecond;
    
    if(r<=goldenFruitProbability){
        fruitType=static_cast<int>(FruitType::GOLDEN);
    }else if(r<=goldenFruitProbability+bombProbability){
        fruitType=static_cast<int>(FruitType::BOMB);
    }else{
        fruitType=round(this->generateRandom(0, NORMAL_FRUIT_COUNT-1));
    }
    
    
    
    //フルーツを作成する
    //std::string filename=StringUtils::format("fruit%d.png",fruitType);
    //auto fruit=Sprite::create(filename);
    //テクスチャのサイズを取り出す
    auto textureSize=_fruitsBatchNode->getTextureAtlas()->getTexture()->getContentSize();
    //テクスチャの横幅を個数で割ったものがフルーツ1個の幅になる
    auto fruitWidth=textureSize.width/static_cast<int>(FruitType::COUNT);
    auto fruit=Sprite::create("fruits.png",Rect(
                                                fruitWidth*fruitType,
                                                0,
                                                fruitWidth,
                                                textureSize.height
                              ));
    fruit->setTag(fruitType);//フルーツの種類をタグとして指定する。
    
    auto fruitSize=fruit->getContentSize();//フルーツのサイズを取り出す
    //float fruitXPos=rand()%static_cast<int>(winSize.width);//x軸のランダムな位置を選択する
    float fruitXPos=generateRandom(fruitWidth/2.0+5,winSize.width-fruitWidth/2.0-5);//x軸のランダムな位置を選択する
    
    fruit->setPosition(Vec2(fruitXPos,winSize.height-FRUIT_TOP_MARGIN-fruitSize.height/2.0));
    //this->addChild(fruit);
    //BatchNodeにフルーツを追加する
    _fruitsBatchNode->addChild(fruit);
    _fruits.pushBack(fruit);//_fruitsベクターにフルーツを追加する
    
    //フルーツに動きをつける
    
    //地面の座標
    auto ground=Vec2(fruitXPos, 0);
    //3秒かけてgroundの位置まで落下させるアクション
    auto fall=MoveTo::create(3, ground);
    
    //removeFruitを即座に呼び出すアクション
    auto remove=CallFuncN::create([this](Node* node){
        //NodeをSpriteにダウンキャストする
        auto sprite=dynamic_cast<Sprite*>(node);
        
        //removeFruitを呼び出す
        this->removeFruit(sprite);
    });
    
    //fallとremoveを連続して実行させるアクション
    //auto sequence=Sequence::create(fall,remove, NULL);
    //fruit->runAction(sequence);
    auto swing=Repeat::create(Sequence::create(RotateTo::create(0.25,-30),RotateTo::create(0.25,30), NULL),2);
    fruit->setScale(0);
    fruit->runAction(Sequence::create(ScaleTo::create(0.25,1),swing,RotateTo::create(0,0.125),fall,remove,NULL));
    
    return fruit;
}

bool MainScene::removeFruit(cocos2d::Sprite* fruit){
    //_fruitsにfruitmが含まれているかを確認する
    if(_fruits.contains(fruit)){
        //親ノードから削除する
        fruit->removeFromParent();
        //_fruits配列から削除する
        _fruits.eraseObject(fruit);
        
        return true;
    }
    
    return false;
}

void MainScene::catchFruit(cocos2d::Sprite *fruit){
    //もしクラッシュしてたら、フルーツを取得できない
    if(this->getIsCrash()){
        return;
    }
    
    auto audioEngine=CocosDenshion::SimpleAudioEngine::getInstance();
    
    //フルーツタイプの取得
    FruitType fruitType=static_cast<FruitType>(fruit->getTag());
    switch (fruitType) {
        case MainScene::FruitType::GOLDEN:
            //黄金のフルーツのとき
            _score+=GOLDEN_FRUIT_SCORE;
            audioEngine->playEffect("catch_golden.mp3");
            break;
            
        case MainScene::FruitType::BOMB:
            //爆弾のとき
            this->onCatchBomb();
            audioEngine->playEffect("catch_bomb.mp3");
            break;
            
        default:
            //その他のとき
            _score+=1;
            audioEngine->playEffect("catch_fruit.mp3");
            break;
    }
    
    //フルーツを削除する
    this->removeFruit(fruit);
    
    _scoreLabel->setString(StringUtils::toString(_score));//スコア用のラベルの表示を更新している
}

void MainScene::onCatchBomb(){
    //クラッシュ状態にする
    _isCrash=true;
    
    //アニメーションの作成
    Vector<SpriteFrame *> frames;
    auto playerSize=_player->getContentSize();
    const int animationFrameCount=3;//アニメーションのフレーム数
    //アニメ用のフレームを読み込む
    for(int i=0; i<animationFrameCount; ++i){
        auto rect=Rect(playerSize.width*i,0,playerSize.width,playerSize.height);
        auto frame=SpriteFrame::create("player_crash.png", rect);
        frames.pushBack(frame);
    }
    //アニメーションを作成する
    auto animation=Animation::createWithSpriteFrames(frames,10.0/60.0);
    animation->setLoops(3);//3回繰り返して再生する
    animation->setRestoreOriginalFrame(true);
    _player->runAction(Sequence::create(Animate::create(animation),
                                        CallFunc::create([this]{_isCrash=false;}),
                                        NULL));
    
    _score=MAX(0,_score-BOMB_PENALTY_SCORE);//4点引いて0未満になったら0点にする
    CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("crash.mp3");
}

void MainScene::addReadyLabel(){
    auto winSize=Director::getInstance()->getWinSize();
    auto center=Vec2(winSize.width/2.0,winSize.height/2.0);
    
    //Readyの文字を定義する
    auto ready=Sprite::create("ready.png");
    ready->setScale(0);//最初に大きさを0%にしておく
    ready->setPosition(center);
    this->addChild(ready);
    
    //STARTの文字を定義する
    auto start=Sprite::create("start.png");
    start->runAction(
                     Sequence::create(
                                      Spawn::create(
                                                    EaseIn::create(
                                                                   ScaleTo::create(0.5,5.0),
                                                                   0.5
                                                                   ),
                                                    FadeOut::create(0.5),//0.5秒かけて拡大とフェードアウトを同時に行う
                                                    NULL
                                                    ),
                                      RemoveSelf::create(),//自分を削除する
                                      NULL
                                      )
                     );
    
    start->setPosition(center);
    
    //READYにアニメーションを追加する
    ready->runAction(
                     Sequence::create(
                                      ScaleTo::create(0.25,1),//0.25秒かけて等倍に拡大される
                                      DelayTime::create(1.0),//1.0秒待つ
                                      CallFunc::create(
                                                       [this,start]{//ラムダの中でthisとstart変数を使っているのでキャプチャに加える
                                                           this->addChild(start);//スタートのラベルを追加する(この時点でスタートのアニメーションがスタート)
                                                           _state=GameState::PLAYING;
                                                           CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("start.mp3");
                                                       }
                                                       ),
                                      RemoveSelf::create(),//自分を削除する
                                      NULL
                                      )
                     );
}

void MainScene::onResult(){
    _state=GameState::RESULT;
    auto winSize=Director::getInstance()->getWinSize();
    
    //ハイスコアの保存
    auto userDefault=UserDefault::getInstance();
    int highscore=userDefault->getIntegerForKey(HIGHSCORE_KEY);
    auto score=static_cast<int>(_score);
    if(score>highscore){
        UserDefault::getInstance()->setIntegerForKey(HIGHSCORE_KEY,score);
        _highScore=score;
        _highScoreLabel->setString(StringUtils::toString(static_cast<int>(_highScore)));
    }
    
    //もう一度遊ぶボタン
    auto replayButton=MenuItemImage::create("replay_button.png",
                                            "replay_button_pressed.png",
                                            [](Ref *ref){
                                                //もう一度遊ぶボタンを押した時の処理、新しくMainSceneを作成して置き換える
                                                //効果音を鳴らす
                                                CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("decide.mp3");
                                                
                                                auto scene=MainScene::createScene();
                                                auto transition=TransitionFade::create(0.5, scene);
                                                Director::getInstance()->replaceScene(transition);
                                            });
    
    //タイトルへ戻るボタン
    auto titleButton=MenuItemImage::create("title_button.png",
                                           "title_button_pressed.png",
                                           [](Ref *ref){
                                               //タイトルへ戻るボタンを押した時の処理
                                               //効果音を鳴らす
                                               CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("decide.mp3");
                                               
                                               auto scene=TitleScene::createScene();
                                               auto transition=TransitionCrossFade::create(1.0, scene);
                                               Director::getInstance()->replaceScene(transition);
                                           });
    
    //二つのボタンからメニューを作成する
    auto menu=Menu::create(replayButton,titleButton, NULL);
    //ボタンを縦に並べる
    menu->alignItemsVerticallyWithPadding(15);//ボタンを縦に並べる
    menu->setPosition(Vec2(winSize.width/2.0, winSize.height/2.0));
    this->addChild(menu);
}

void MainScene::update(float dt){
    if(_state==GameState::PLAYING) {
        //毎フレーム実行される
        
        //フルーツの出現を判定する
        float pastTime=TIME_LIMIT_SECOND-_second;
        float p=FRUIT_SPAWN_INCREASE_BASE*(1+powf(FRUIT_SPAWN_INCREASE_RATE,pastTime));
        p=MIN(p,MAXIMUM_SPAWN_PROBABILITY);//pが最大値以上なら丸める
        float random=this->generateRandom(0,1);
        //int random=rand()%FRUIT_SPAWN_RATE;
        if(random<p){//適当な乱数が0のとき
            this->addFruit();
        }
    
        for(auto& fruit : _fruits){
            Vec2 busketPosition=_player->getPosition()-Vec2(0,10);
            Rect boundingBox=fruit->getBoundingBox();//フルーツの矩形を取り出す
            bool isHit=boundingBox.containsPoint(busketPosition);
            if(isHit){
                this->catchFruit(fruit);
            }
        }
    
        //残り秒数を減らす
        _second-=dt;
        //残り秒数の表示を更新する
        int second=static_cast<int>(_second);//int型にキャストする
        _secondLabel->setString(StringUtils::toString(second));
        
        if(_second<0){//制限時間が0になったら
            //リザルト状態に移行
            //_state=GameState::RESULT;
            _state=GameState::ENDING;//ゲーム状態をENDINGに移行
            //終了時の効果音を鳴らす
            //CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("finish.mp3");
            //this->onResult();
            
            //終了文字の表示
            auto finish=Sprite::create("finish.png");
            auto winSize=Director::getInstance()->getWinSize();
            finish->setPosition(Vec2(winSize.width/2.0,winSize.height/2.0));
            finish->setScale(0);
            CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("finish.mp3");
            
            //アクションの作成
            auto appear=EaseExponentialIn::create(ScaleTo::create(0.25,1.0));
            auto disappear=EaseExponentialIn::create(ScaleTo::create(0.25,0));
            
            finish->runAction(Sequence::create(
                                               appear,
                                               DelayTime::create(2.0),
                                               disappear,
                                               DelayTime::create(1.0),
                                               CallFunc::create(
                                                                [this]{
                                                                    _state=GameState::RESULT;//ゲーム状態をリザルトに移行
                                                                    this->onResult();//メニュー画面を出現させる
                                                                }
                                                                ),
                                               NULL
                                               )
                              );
            
            this->addChild(finish);
        }
    }
}

void MainScene::onEnterTransitionDidFinish(){
    Layer::onEnterTransitionDidFinish();
    //BGMを再生
    CocosDenshion::SimpleAudioEngine::getInstance()->playBackgroundMusic("main.mp3",true);
    //開始時の効果音を再生
    //CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("start.mp3");
    
    //READY演出を行う
    this->addReadyLabel();
}

bool MainScene::init(){
    if(!Layer::init()){
        return  false;
    }
    
    //Directorを取り出す
    auto director=Director::getInstance();
    //画面サイズを取り出す
    auto size=director->getWinSize();
    //背景のスプライトを生成する
    auto background=Sprite::create("background.png");
    //スプライトの表示位置を設定する
    background->setPosition(Vec2(size.width/2.0,size.height/2.0));
    //親ノードにスプライトを追加する
    this->addChild(background);
    
    
    //Spriteを生成して_playerに格納
    this->setPlayer(Sprite::create("player.png"));
    //_playerの位置を設定
    _player->setPosition(Vec2(size.width/2.0,size.height-445));
    //シーンに_playerを配置
    this->addChild(_player);
    
    
    auto listener=EventListenerTouchOneByOne::create();
    listener->onTouchBegan=[](Touch* touch,Event* event){
        //タッチされたときの処理
        //log("touch at (%f,%f)",touch->getLocation().x,touch->getLocation().y);
        return true;//イベントを実行する
        
    };
    listener->onTouchMoved=[this](Touch* touch,Event* event){
        //タッチ中に動いた時の処理
        if(!this->getIsCrash()){//クラッシュしてないとき
            //前回とのタッチ位置の差をベクトルで取得する
            Vec2 delta=touch->getDelta();
            
            //現在のかわずたんの座標を取得する
            Vec2 position=_player->getPosition();
            
            //現在座標＋移動量を新たな座標にする
            Vec2 newPosition=position+delta;
            
            //プレイヤーの横幅
            auto playerWidth=_player->getContentSize().width;
            
            auto winSize=Director::getInstance()->getWinSize();
            newPosition=newPosition.getClampPoint(Vec2(playerWidth/3.0, position.y), Vec2(winSize.width-playerWidth/3.0,position.y));
            _player->setPosition(newPosition);
        }
    };
    director->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this);
    
    
    //スコアラベルの追加
    auto scoreLabel=Label::createWithSystemFont(StringUtils::toString(_score), "Marker Felt", 16);
    scoreLabel->setPosition(Vec2(size.width/2.0*1.5,size.height-40));
    scoreLabel->enableShadow(Color4B::BLACK,Size(0.5, 0.5),3);
    scoreLabel->enableOutline(Color4B::BLACK,1.5);
    this->setScoreLabel(scoreLabel);
    this->addChild(_scoreLabel);
    
    //スコアヘッダーの追加
    auto scoreLabelHeader=Label::createWithSystemFont("SCORE", "Marker Felt", 16);
    scoreLabelHeader->enableShadow(Color4B::BLACK,Size(0.5, 0.5),3);
    scoreLabelHeader->enableOutline(Color4B::BLACK,1.5);
    scoreLabelHeader->setPosition(Vec2(size.width/2.0*1.5,size.height-20));
    this->addChild(scoreLabelHeader);
    
    //ハイスコアの追加
    int saveHighScore=UserDefault::getInstance()->getIntegerForKey(HIGHSCORE_KEY);//記録されたハイスコア
    if(saveHighScore){_highScore=saveHighScore;}
    auto highScoreLabel=Label::createWithSystemFont(StringUtils::toString(_highScore),"Marker Felt",16);
    highScoreLabel->setPosition(Vec2(size.width/2.0*0.5,size.height-40));
    highScoreLabel->enableShadow(Color4B::BLACK,Size(0.5,0.5),3);
    highScoreLabel->enableOutline(Color4B::BLACK,1.5);
    this->setHighScoreLabel(highScoreLabel);
    this->addChild(_highScoreLabel);
    
    //ハイスコアヘッダーの追加
    auto highScoreLabelHeader=Label::createWithSystemFont("HIGHSCORE","Marker Felt",16);
    highScoreLabelHeader->setPosition(Vec2(size.width/2.0*0.5,size.height-20));
    highScoreLabelHeader->enableShadow(Color4B::BLACK,Size(0.5, 0.5),3);
    highScoreLabelHeader->enableOutline(Color4B::BLACK,1.5);
    this->addChild(highScoreLabelHeader);
    
    
    //タイマーラベルの追加
    int second=static_cast<int>(_second);//int型にキャストする
    auto secondLabel=Label::createWithSystemFont(StringUtils::toString(second), "Marker Felt", 16);
    this->setSecondLabel(secondLabel);
    secondLabel->enableShadow(Color4B::BLACK,Size(0.5, 0.5),3);
    secondLabel->enableOutline(Color4B::BLACK,1.5);
    secondLabel->setPosition(Vec2(size.width/2.0,size.height-40));
    this->addChild(secondLabel);
    
    //タイマーヘッダーの追加
    auto secondLabelHeader=Label::createWithSystemFont("TIME", "Marker Felt", 16);
    secondLabelHeader->enableShadow(Color4B::BLACK,Size(0.5, 0.5),3);
    secondLabelHeader->enableOutline(Color4B::BLACK,1.5);
    secondLabelHeader->setPosition(Vec2(size.width/2.0,size.height-20));
    this->addChild(secondLabelHeader);
    
    
    //BatchNodeの初期化
    auto fruitsBatchNode=SpriteBatchNode::create("fruits.png");
    this->addChild(fruitsBatchNode);
    this->setFruitsBatchNode(fruitsBatchNode);
    
    
    //updateを毎フレーム実行するように登録する
    this->scheduleUpdate();
    
    return  true;
}