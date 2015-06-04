//
//  SceneGameMain.cpp
//  pirateTaxtics
//
//  Created by kk on 2015/05/04.
//
//
/*素材などは翔泳社「戦略シミュレーションゲームの作り方」の
 *サンプルを使用しています
 */

#include <UISlider.h>
#include <random>
#include "SceneGameMain.h"
#include "SimpleAudioEngine.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"
#include "Title.h"

USING_NS_CC;

//const float STANDARD_BGM_VOLUME=0.03f;//初期BGM音量
//const float STANDARD_EFFECTS_VOLUME=0.5f;//初期効果音音量

////GameInitialise
GameInitialise::GameInitialise()
{
    
}

GameInitialise::~GameInitialise()
{
    
}

GameInitialise* GameInitialise::create(int stageNumber,bool isPlayer2CPU,bool isVersus)
{
    GameInitialise* ret=new GameInitialise();
    if(ret->init(stageNumber,isPlayer2CPU,isVersus)){
        ret->autorelease();
        return ret;
    }
    
    CC_SAFE_RELEASE_NULL(ret);
    return nullptr;
}

bool GameInitialise::init(int stageNumber,bool isPlayer2CPU,bool isVersus)
{
    if(stageNumber!=0){return false;}
    _stageNumber=stageNumber;
    _isPlayer2CPU=isPlayer2CPU;
    _isVersus=isVersus;
    
    return true;
}
//end/GameInitialise



SceneGameMain::SceneGameMain()
:_turnCounter(0)
,_gameState(GameState::GAMESTART)
,_turnActionPlusCount(0)
,_gameInitialise(nullptr)
,_stage(nullptr)
,_labelLayer(nullptr)
,_turnLabel(nullptr)
,_playerLabel(nullptr)
{
    //乱数の初期化
    std::random_device rdev;
    _engine.seed(rdev());
}

SceneGameMain::~SceneGameMain()
{
    CocosDenshion::SimpleAudioEngine::getInstance()->stopBackgroundMusic();//BGMを停止
    CC_SAFE_RELEASE_NULL(_gameInitialise);
    CC_SAFE_RELEASE_NULL(_stage);
    CC_SAFE_RELEASE_NULL(_labelLayer);
    CC_SAFE_RELEASE_NULL(_turnLabel);
    CC_SAFE_RELEASE_NULL(_playerLabel);
}


//Scene* SceneGameMain::createSceneWithLevel(int level)
Scene* SceneGameMain::createSceneWithGameInitialise(GameInitialise *gameInitialise)
{
    auto scene=Scene::create();
    auto layer=new SceneGameMain();
    if(layer && layer->initWithGameInitialise(gameInitialise)){
        layer->autorelease();
    }else{
        CC_SAFE_DELETE(layer);
    }
    scene->addChild(layer);
    return scene;
}

//bool SceneGameMain::initWithLevel(int level)
bool SceneGameMain::initWithGameInitialise(GameInitialise *gameInitialise)
{
    if(!Layer::init()){
        return false;
    }
    
    this->setGameInitialise(gameInitialise);
    //this->createMap(level);//ステージマップ
    this->createMap();
    this->createLabelLayer();//ラベルレイヤーを作成
    this->createPlayers();//プレイヤーを作成
    
    //CocosDenshion::SimpleAudioEngine::getInstance()->setBackgroundMusicVolume(STANDARD_BGM_VOLUME);//bgm音量設定
    //CocosDenshion::SimpleAudioEngine::getInstance()->setEffectsVolume(STANDARD_EFFECTS_VOLUME);//効果音の音量
    
    auto tileTouchListener=EventListenerTouchOneByOne::create();
    //タッチ開始
    tileTouchListener->onTouchBegan=[this](Touch* touch,Event* event){
        //入力待機中の処理
        if(_gameState==GameState::INPUTWAITING){
            auto stagePosition=touch->getLocation();
            //タイルマップ上でタッチされた場合のみ処理する
            if(_stage->onTiledMapCheck(stagePosition)){
                auto position=_stage->convertToTiledMapSpace(stagePosition);
                auto player=this->getActivePlayer();
                //キャラの移動力に応じてマーカーを配置
                _stage->setMarker(player->getActiveFuneA(),position);
            }
        }
        return true;
    };
    //タッチ中に動かす
    tileTouchListener->onTouchMoved=[this](Touch* touch,Event* event){
        //入力待機中の処理
        if(_gameState==GameState::INPUTWAITING){
            _stage->markerHide();
            auto stagePosition=touch->getLocation();
            if(_stage->onTiledMapCheck(stagePosition)){
                auto position=_stage->convertToTiledMapSpace(stagePosition);
                auto player=this->getActivePlayer();
                //キャラの移動力に応じてマーカーを配置
                _stage->setMarker(player->getActiveFuneA(),position);
            }
        }
    };
    //タッチ終了
    tileTouchListener->onTouchEnded=[this](Touch* touch,Event* event){
        //入力待機中の処理
        if(_gameState==GameState::INPUTWAITING){
            _stage->markerHide();
            auto stagePosition=touch->getLocation();//タッチされた座標
            //タイルマップ上でタッチされた場合のみ処理する
            if(_stage->onTiledMapCheck(stagePosition)){
                auto position=_stage->convertToTiledMapSpace(stagePosition);//タイルマップ座標に変換
                auto player=this->getActivePlayer();
                auto fune=player->getActiveFuneA();
                auto distance=_stage->getDistance(position,fune->getTiledMapPosition());
                
                //クリックした位置に船がある
                auto targetFune=_stage->getOnTiledMapFune(position);
                if(targetFune){
                    if(targetFune==fune){//選択中の船の場合
                        this->createHelpLayer(targetFune);//ヘルプを表示
                    }else if(player->getFuneListContains(targetFune)){//プレイヤーが所持する船の場合
                        player->setActiveFune(targetFune);
                        this->updateTurn();
                    }else{//他のプレイヤーの船の場合
                        if(fune->withinRange(distance)){//攻撃範囲内
                            this->normalAttack(fune,targetFune);
                        }else{//攻撃範囲外
                            _gameState=GameState::ACTION;
                            _stage->damageLabel(targetFune,"攻撃範囲外です",CallFunc::create([this]{
                                _gameState=GameState::INPUTWAITING;
                            }));
                        }
                    }
                }else{//クリックした位置に船は無い
                    _gameState=GameState::ACTION;
                    //if(_stage->tileMoveCheck(position) && distance<=fune->getMovement()){//移動
                    if(distance<=fune->getMovement() &&
                       _stage->getAStar()->checkLine(fune->getTiledMapPosition(),position,fune->getMovement())){//移動可
                        _stage->markerShow(Stage::MapMarkerTypes::REDCROSS,position);
                        this->scheduleOnce(
                                           [this,fune,position](float dt){
                                               _stage->markerHide();
                                               this->moveProcess(fune,position);
                                               /*
                                               _stage->removeAreaRangeLayer();
                                               _stage->moveFuneAnimation(fune,position,
                                                                         CallFunc::create([this]{
                                                   this->endTurn();
                                               }));
                                                */
                                               //_stage->positionFune(fune,position);
                                               //this->endTurn();
                                           },0.2,"CHARACTER_MOVE_EVENT"
                                           );
                    }else{//移動不可
                        _stage->markerShow(Stage::MapMarkerTypes::GRAYCROSS,position);
                        this->scheduleOnce([this](float dt){
                            _stage->markerHide();
                            _gameState=GameState::INPUTWAITING;
                        },0.2,"CHARCTER_MOVE_CANCEL");
                    }
                }
            }
        }
    };
    //タッチキャンセル
    tileTouchListener->onTouchCancelled=[this](Touch* touch,Event* event){
        _stage->markerHide();
    };
    Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(tileTouchListener, _stage);
    
    //this->beginGame();//onEnterTransitionDidFinish()に移動
    return true;
}

void SceneGameMain::onEnterTransitionDidFinish()
{
    Layer::onEnterTransitionDidFinish();
    CocosDenshion::SimpleAudioEngine::getInstance()->playBackgroundMusic("music/highseas.mp3",true);
    //CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sound/se4.wav");
    this->beginGame();
}



////stage関係
void SceneGameMain::createMap()
{
    auto stage=Stage::createWithLevel(_gameInitialise->getStageNumber());
    this->addChild(stage,static_cast<int>(LayerZPositions::STAGE));
    this->setStage(stage);
}
//end/stage関係


////player関係
void SceneGameMain::createPlayers()
{
    //船初期配置 vec2(0,8) vec2(12,0)
    Vec2 stagePosition[2][4]={{Vec2(0,8),Vec2(0,6),Vec2(1,7),Vec2(2,8)},
        {Vec2(12,0),Vec2(10,0),Vec2(11,1),Vec2(12,2)}};
    
    auto player1=Player::create(false);
    this->addPlayerList(player1);
    //プレイヤー側のキャラは0-3番
    for(auto i=0;i<4;i++){
        this->createFune(player1,static_cast<Fune::CharacterTypes>(i),stagePosition[0][i]);
    }
    setCC(player1->getFune(0));
    
    auto player2=Player::create(_gameInitialise->getIsPlayer2CPU());
    this->addPlayerList(player2);
    //相手側のキャラは4-7番
    for(auto i=4;i<8;i++){
        this->createFune(player2,static_cast<Fune::CharacterTypes>(i),stagePosition[1][i-4]);
    }
}

void SceneGameMain::addPlayerList(Player *player)
{
    _playerList.pushBack(player);
}

int SceneGameMain::getPlayerListCount()
{
    return static_cast<int>(_playerList.size());
}

Player* SceneGameMain::getPlayer(int index)
{
    return _playerList.at(index);
}

int SceneGameMain::getPlayerIndex(Player *player)
{
    return static_cast<int>(_playerList.getIndex(player));
}

int SceneGameMain::getActivePlayerIndex()
{
    return _turnCounter%static_cast<int>(_playerList.size());
}

Player* SceneGameMain::getActivePlayer()
{
    return _playerList.at(this->getActivePlayerIndex());
}
//end/player関係



////fune関係
void SceneGameMain::createFune(Player *player,Fune::CharacterTypes characterType,const Vec2& position)
{
    auto fune=Fune::createWithLevel(characterType);
    _stage->addFuneList(fune);
    player->addFuneList(fune);
    _stage->setFuneLayer(fune);
    _stage->positionFune(fune,position);
}
//end/fune関係




////UI関係
void SceneGameMain::createLabelLayer()
{
    auto layer=Layer::create();
    this->setLabelLayer(layer);
    this->addChild(layer,static_cast<int>(LayerZPositions::LABEL));
    
    this->createTurnLabel();
    this->createPlayerLabel();
    this->createSettingButton();
}

void SceneGameMain::createTurnLabel()
{
    auto string=StringUtils::format("ターン：");
    auto turnLabel=Label::createWithSystemFont(string,"Arial",32);
    turnLabel->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
    turnLabel->setPosition(Vec2(320,10));
    this->setTurnLabel(turnLabel);
    _labelLayer->addChild(turnLabel);
}

void SceneGameMain::createPlayerLabel()
{
    auto string=StringUtils::format("プレイヤー");
    auto playerLabel=Label::createWithSystemFont(string,"Arial",32);
    playerLabel->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
    playerLabel->setPosition(Vec2(64,10));
    this->setPlayerLabel(playerLabel);
    _labelLayer->addChild(playerLabel);
}

void SceneGameMain::createSettingButton()
{
    auto winSize=Director::getInstance()->getWinSize();
    //設定画面を開くボタンを作成
    auto button=MenuItemImage::create("images/settings.png","images/settings.png",
                                      [this](Ref* ref){
                                          this->createSettingWindow();
                                      });
    button->runAction(RepeatForever::create(RotateBy::create(10,360)));
    auto menu=Menu::create(button,NULL);
    menu->setPosition(Vec2(winSize.width-40,40));
    _labelLayer->addChild(menu);
    
    //メニューと明記
    auto label=Label::createWithSystemFont("MENU","Arial",24);
    label->setColor(Color3B::ORANGE);
    label->enableOutline(Color4B::BLACK);
    label->enableShadow(Color4B::BLACK,Size(0.5,0.5),3);
    label->setPosition(Vec2(winSize.width-40,25));
    _labelLayer->addChild(label);
}

void SceneGameMain::createSettingWindow()
{
    /*関数定義
     *設定画面の作成
     *BGM設定
     *効果音設定
     *設定画面を閉じるボタン設定
     *タイトルへ戻るボタン設定
     *アニメーションなど一時停止、効果音のみ一時停止
     */
    auto audio=CocosDenshion::SimpleAudioEngine::getInstance();
    
    ////関数
    //音量をスライダーの値に変換
    auto convertToSliderPercent=[](float volume)->int{
        int percent=static_cast<int>(volume*100);
        percent=std::max(0,percent);//0以上
        percent=std::min(100,percent);//100以下
        return percent;
    };
    //スライダーの値を音量に変換
    auto convertToVolume=[](int percent)->float{
        float volume=static_cast<float>(percent)/100.f;
        volume=std::min(1.0f,volume);//1以下
        volume=std::max(0.0f,volume);//0以上
        return volume;
    };
    //音量を設定 volumeType true:BGM false:effect
    auto setVolume=[audio,convertToVolume](bool volumeType,int percent,ui::Text* label){
        auto volume=convertToVolume(percent);
        label->setString(StringUtils::toString(percent));
        if(volumeType){
            audio->setBackgroundMusicVolume(volume);
        }else{
            audio->setEffectsVolume(volume);
        }
    };
    //スライダーの値が変化したときのイベント slider:イベントを設定するスライダー label:スライダーの数値を表示するラベル
    //volumeType true:BGM false:effect
    auto setChangeSliderEvent=[audio,setVolume](ui::Slider* slider,ui::Text* label,bool volumeType){
        slider->addEventListener([audio,setVolume,label,volumeType](Ref* pSlider,ui::Slider::EventType type){
            if(type==ui::Slider::EventType::ON_PERCENTAGE_CHANGED){//スライダーの値が変化したなら
                auto slider=dynamic_cast<ui::Slider*>(pSlider);
                setVolume(volumeType,slider->getPercent(),label);
            }
        });
    };
    //音量を調整するボタンにタッチイベントを設定
    //button:設定するボタン slider:音量を表すスライダー label:音量を表すラベル
    //value:増減する値 volumeType true:BGM false:effect
    auto setVolumeButton=[setVolume](ui::Button* button,ui::Slider* slider,ui::Text* label,int value,bool volumeType){
        button->addTouchEventListener(
                                      [setVolume,slider,label,value,volumeType](Ref* pButton,ui::Widget::TouchEventType type)
                                      {
                                          auto button=dynamic_cast<ui::Button*>(pButton);
                                          if(type==ui::Widget::TouchEventType::BEGAN){//タッチ開始時
                                              button->setColor(Color3B::GRAY);
                                          }
                                          if(type==ui::Widget::TouchEventType::ENDED){//タッチ終了時
                                              slider->setPercent(slider->getPercent()+value);
                                              setVolume(volumeType,slider->getPercent(),label);
                                              button->setColor(Color3B::WHITE);
                                          }
                                          if(type==ui::Widget::TouchEventType::CANCELED){//タッチキャンセル
                                              button->setColor(Color3B::WHITE);
                                          }
                                      });
    };
    //end/関数
    
    
    ////設定画面の作成
    auto settingLayer=Layer::create();
    //レイヤーの下にタッチイベントが伝播しないようにする pause中でもタッチイベントは発生する
    auto listener=EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan=[](Touch* touch,Event* event){return true;};
    Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener,settingLayer);
    
    auto window=CSLoader::getInstance()->createNode("SettingLayer.csb");
    settingLayer->addChild(window);
    //end/設定画面の作成
    
    
    
    ////BGM設定
    auto bgmValueLabel=window->getChildByName<ui::Text*>("BGMValue");//BGMValueラベルの設定
    //値の更新
    bgmValueLabel->setString(StringUtils::toString(convertToSliderPercent(audio->getBackgroundMusicVolume())));
    auto bgmSlider=window->getChildByName<ui::Slider*>("BGMSlider");//bgmスライドバーの設定
    bgmSlider->setPercent(convertToSliderPercent(audio->getBackgroundMusicVolume()));//バーの位置を調整
    setChangeSliderEvent(bgmSlider,bgmValueLabel,true);//バーの値が変わった時のイベントを設定
    auto BGMUpButton=window->getChildByName<ui::Button*>("BGMUpButton");//bgm音量を上げるボタン
    setVolumeButton(BGMUpButton,bgmSlider,bgmValueLabel,1,true);
    auto BGMDownButton=window->getChildByName<ui::Button*>("BGMDownButton");//bgm音量を下げるボタン
    setVolumeButton(BGMDownButton,bgmSlider,bgmValueLabel,-1,true);
    //end/BGM設定
    
    ////効果音設定
    auto effectsValueLabel=window->getChildByName<ui::Text*>("EffectsValue");//EffectsValueラベルの設定
    //値の更新
    effectsValueLabel->setString(StringUtils::toString(convertToSliderPercent(audio->getEffectsVolume())));
    auto effectsSlider=window->getChildByName<ui::Slider*>("EffectsSlider");//効果音スライドバーの設定
    effectsSlider->setPercent(convertToSliderPercent(audio->getEffectsVolume()));//バーの位置を調整
    setChangeSliderEvent(effectsSlider,effectsValueLabel,false);//バーの値が変わった時のイベントを設定
    auto effectsUpButton=window->getChildByName<ui::Button*>("EffectsUpButton");//効果音音量を上げるボタン
    setVolumeButton(effectsUpButton,effectsSlider,effectsValueLabel,1,false);
    auto effectsDownButton=window->getChildByName<ui::Button*>("EffectsDownButton");//効果音音量を下げるボタン
    setVolumeButton(effectsDownButton,effectsSlider,effectsValueLabel,-1,false);
    //end/効果音設定
    
    
    
    //設定画面を閉じるボタンの設定
    auto backButton=window->getChildByName<ui::Button*>("backButton");
    backButton->addTouchEventListener([this,settingLayer,audio](Ref* pButton,ui::Widget::TouchEventType type){
        auto button=dynamic_cast<ui::Button*>(pButton);
        if(type==ui::Widget::TouchEventType::BEGAN){//タッチ開始
            button->setColor(Color3B::GRAY);
        }else if(type==ui::Widget::TouchEventType::ENDED){//タッチ終了
            Director::getInstance()->getEventDispatcher()->removeEventListenersForTarget(settingLayer);//レイヤーのイベント削除
            settingLayer->removeFromParent();//レイヤーを消す
            Director::getInstance()->resume();//再開
            audio->resumeAllEffects();//効果音再開
        }else if(type==ui::Widget::TouchEventType::CANCELED){//タッチキャンセル
            button->setColor(Color3B::WHITE);
        }
    });
    
    //タイトルへ戻るボタンの設定
    auto titleButton=window->getChildByName<ui::Button*>("titleButton");
    titleButton->addTouchEventListener([this](Ref* pButton,ui::Widget::TouchEventType type){
        auto button=dynamic_cast<ui::Button*>(pButton);
        if(type==ui::Widget::TouchEventType::BEGAN){
        }else if(type==ui::Widget::TouchEventType::ENDED){
            Director::getInstance()->getEventDispatcher()->removeAllEventListeners();//全てのイベントを無効化
            Director::getInstance()->getActionManager()->pauseAllRunningActions();//全てのアクションを停止
            Director::getInstance()->getActionManager()->removeAllActions();//全てのアクションを削除
            CocosDenshion::SimpleAudioEngine::getInstance()->stopAllEffects();//全ての効果音を終了
            Director::getInstance()->resume();//一時停止を削除
            
            this->scheduleOnce([](float dt){
                auto scene=Title::createScene();
                auto transition=TransitionPageTurn::create(1,scene,false);
                Director::getInstance()->replaceScene(transition);
            },0.2,"settingLayerTitleBackButtonAction");
        }else if(type==ui::Widget::TouchEventType::CANCELED){
        }
    });
    
    this->addChild(settingLayer,static_cast<int>(LayerZPositions::SETTING));
    Director::getInstance()->pause();//一時停止
    audio->pauseAllEffects();//効果音のみ停止
}

void SceneGameMain::createHelpLayer(Fune *fune)
{
    /*
    if(_helpLayer){
        this->removeHelpLayer();
    }*/
    
    //レイヤー等作成
    //ステータス表示
    //ボタン設定
    //レイヤーのイベント設定
    _gameState=GameState::HELP;
    
    //auto helpLayer=Layer::create();
    auto helpLayer=LayerColor::create();
    helpLayer->setColor(Color3B::BLACK);
    helpLayer->setOpacity(64);
    
    auto winSize=Director::getInstance()->getWinSize();
    
    auto window=CSLoader::getInstance()->createNode("HelpLayer.csb");
    window->setScale(0.7);
    window->runAction(ScaleTo::create(0.2,1));
    helpLayer->addChild(window);
    
    ////ステータス表示
    //船長
    auto captain=window->getChildByName<ui::Text*>("captainValue");
    Sprite* characterSprite=nullptr;//キャラクターの絵
    switch (fune->getType()) {
        case Fune::CharacterTypes::PC_CAPTAIN:
            captain->setString("キャプテン");
            characterSprite=Sprite::create("images/pirate00.png");
            break;
            
        case Fune::CharacterTypes::PC_SPEED:
            captain->setString("はやいちゃん");
            characterSprite=Sprite::create("images/pirate01.png");
            break;
            
        case Fune::CharacterTypes::PC_DEFENSE:
            captain->setString("かたいちゃん");
            characterSprite=Sprite::create("images/pirate02.png");
            break;
            
        case Fune::CharacterTypes::PC_ATTACK:
            captain->setString("攻撃ちゃん");
            characterSprite=Sprite::create("images/pirate03.png");
            break;
            
        case Fune::CharacterTypes::ENEMY_BOSS:
            captain->setString("ボス");
            break;
            
        case Fune::CharacterTypes::ENEMY_SPEED:
            captain->setString("スピードタイプ");
            break;
            
        case Fune::CharacterTypes::ENEMY_DEFENSE:
            captain->setString("ディフェンスタイプ");
            break;
            
        case Fune::CharacterTypes::ENEMY_ATTACK:
            captain->setString("アタックタイプ");
            break;
            
        default:
            break;
    }
    
    if(characterSprite){
        characterSprite->setPosition(Vec2(winSize.width/2+256,winSize.height/2));
        characterSprite->setOpacity(0);
        characterSprite->runAction(FadeIn::create(0.6));
        helpLayer->addChild(characterSprite);
    }
    
    //攻撃力
    auto attack=window->getChildByName<ui::Text*>("attackValue");
    attack->setString(StringUtils::toString(fune->getAttack()));
    
    //防御力
    auto defense=window->getChildByName<ui::Text*>("defenseValue");
    defense->setString(StringUtils::toString(fune->getDefense()));
    
    //移動力
    auto movement=window->getChildByName<ui::Text*>("movementValue");
    movement->setString(StringUtils::toString(fune->getMovement()));
    
    //攻撃の距離
    auto range=window->getChildByName<ui::Text*>("rangeValue");
    range->setString(StringUtils::toString(fune->getRange()));
    
    //hp
    auto hp=window->getChildByName<ui::Text*>("hpValue");
    hp->setString(StringUtils::toString(fune->getHp()));
    
    //maxHp
    auto maxHp=window->getChildByName<ui::Text*>("maxHpValue");
    maxHp->setString(StringUtils::toString(fune->getMaxHP()));
    
    //スキル残り回数
    auto skillCount=fune->getSkillCount();
    window->getChildByName<ui::Text*>("skillCountValue")->setString(StringUtils::toString(skillCount));
    //end/ステータス表示
    
    
    
    ////ボタン設定
    //戻るボタン
    auto backButton=window->getChildByName<ui::Button*>("backButton");
    //スキルメニューボタン
    auto skillButton=window->getChildByName<ui::Button*>("skillButton");
    
    
    if(skillCount>0){//スキルの残り回数があればスキルメニューを設定
        //スキルメニュー
        auto skillWindow=CSLoader::getInstance()->createNode("skillLayer.csb");
        skillWindow->setVisible(false);
        
        //スキル名・説明表示
        auto skillName=skillWindow->getChildByName<ui::Text*>("skillName");
        auto skillExplain=skillWindow->getChildByName<ui::TextField*>("skillExplain");
        std::function<void ()> skill;//使用するスキルを格納したラムダ
        switch(fune->getType()){
            case Fune::CharacterTypes::PC_CAPTAIN:
            case Fune::CharacterTypes::ENEMY_BOSS:
                skillName->setString("セカンドウィンド");
                skillExplain->setString("味方の全ての船のHPを50%回復する");
                skill=[this,fune](){this->skillHeal(fune);};
                break;
                
            case Fune::CharacterTypes::PC_SPEED:
            case Fune::CharacterTypes::ENEMY_SPEED:
                skillName->setString("ハリーアップ");
                skillExplain->setString("プレイヤーは現ターン中2回連続で行動できる");
                skill=[this,fune](){this->skillHurryUp(fune);};
                break;
                
            case Fune::CharacterTypes::PC_DEFENSE:
            case Fune::CharacterTypes::ENEMY_DEFENSE:
                skillName->setString("アイロンシールド");
                skillExplain->setString("一度だけ自船への攻撃を無効化する");
                //skill=[this,fune](){this->skillIronShield(fune);};
                skill=[this,fune](){this->skillDebug(fune);};
                break;
                
            case Fune::CharacterTypes::PC_ATTACK:
            case Fune::CharacterTypes::ENEMY_ATTACK:
                skillName->setString("バレットストーム");
                skillExplain->setString("自船の攻撃範囲内の全ての船を攻撃する");
                skill=[this,fune](){this->skillBulletStorm(fune);};
                break;
            default:
                break;
        }
        
        
        //戻るボタン
        auto skillWindowBackButton=skillWindow->getChildByName<ui::Button*>("backButton");
        skillWindowBackButton->addTouchEventListener([skillWindow,backButton,skillButton]
                                                     (Ref* pButton,ui::Widget::TouchEventType type){
                                                         auto button=dynamic_cast<ui::Button*>(pButton);
                                                         if(type==ui::Widget::TouchEventType::BEGAN){
                                                             button->setColor(Color3B::GRAY);
                                                         }else if(type==ui::Widget::TouchEventType::ENDED){
                                                             backButton->setEnabled(true);
                                                             backButton->setColor(Color3B::WHITE);
                                                             skillButton->setEnabled(true);
                                                             skillButton->setColor(Color3B::WHITE);
                                                             skillWindow->setVisible(false);
                                                         }else if(type==ui::Widget::TouchEventType::CANCELED){
                                                             button->setColor(Color3B::WHITE);
                                                         }
                                                     });
        //スキル使用ボタン
        auto skillWindowSkillButton=skillWindow->getChildByName<ui::Button*>("skillButton");
        skillWindowSkillButton->addTouchEventListener([this,fune,skill,helpLayer,skillWindow,backButton]
                                                      (Ref* pButton,ui::Widget::TouchEventType type){
            auto button=dynamic_cast<ui::Button*>(pButton);
            switch(type){
                case cocos2d::ui::Widget::TouchEventType::BEGAN:
                    button->setColor(Color3B::GRAY);
                    break;
                    
                case cocos2d::ui::Widget::TouchEventType::ENDED:
                    Director::getInstance()->getEventDispatcher()->removeEventListenersForTarget(helpLayer);
                    helpLayer->removeFromParent();
                    skill();
                    /*
                    backButton->setEnabled(true);
                    backButton->setColor(Color3B::WHITE);
                    skillWindow->setVisible(false);
                     */
                    break;
                    
                case cocos2d::ui::Widget::TouchEventType::CANCELED:
                    button->setColor(Color3B::WHITE);
                    break;
                    
                default:
                    break;
            }
        });
        helpLayer->addChild(skillWindow);
        
        //スキルボタンを押した時の処理
        skillButton->addTouchEventListener([skillWindow,backButton,skillButton]
                                           (Ref* pButton,ui::Widget::TouchEventType type){
                                               auto button=dynamic_cast<ui::Button*>(pButton);
                                               if(type==ui::Widget::TouchEventType::BEGAN){
                                                   button->setColor(Color3B::GRAY);
                                               }else if(type==ui::Widget::TouchEventType::ENDED){
                                                   //ヘルプレイヤーのボタンを無効にしておく
                                                   backButton->setEnabled(false);
                                                   backButton->setColor(Color3B::GRAY);
                                                   skillButton->setEnabled(false);
                                                   skillWindow->setVisible(true);
                                               }else if(type==ui::Widget::TouchEventType::CANCELED){
                                                   button->setColor(Color3B::WHITE);
                                               }
                                           });
        
    }else{//スキルの残り回数が無ければskillButtonを無効化しておく
        skillButton->setEnabled(false);
        skillButton->setColor(Color3B::GRAY);
    }
    
    
    //戻るボタンを押した時の処理
    backButton->addTouchEventListener([this,helpLayer,window,characterSprite,skillButton]
                                      (Ref* pButton,ui::Widget::TouchEventType type){
        auto button=dynamic_cast<ui::Button*>(pButton);
        if(type==ui::Widget::TouchEventType::BEGAN){
            button->setColor(Color3B::GRAY);
        }else if(type==ui::Widget::TouchEventType::ENDED){
            button->setEnabled(false);
            skillButton->setEnabled(false);
            skillButton->setColor(Color3B::GRAY);
            if(characterSprite){
                characterSprite->runAction(FadeOut::create(0.2));
            }
            window->runAction(Sequence::create(ScaleTo::create(0.2,0.2),
                                               CallFunc::create([this,helpLayer,characterSprite]{
                if(characterSprite){
                    characterSprite->stopAllActions();
                }
                Director::getInstance()->getEventDispatcher()->removeEventListenersForTarget(helpLayer);
                helpLayer->removeFromParent();
                _gameState=GameState::INPUTWAITING;
            }),
                                               NULL));
        }else if(type==ui::Widget::TouchEventType::CANCELED){
            button->setColor(Color3B::WHITE);
        }
    });
    //end/ボタン設定
    
    
    //他のタッチイベントが発生しないようにする
    auto listener=EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan=[](Touch* touch,Event* event){return true;};
    Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener,helpLayer);
    
    this->addChild(helpLayer,static_cast<int>(LayerZPositions::HELP));
}

void SceneGameMain::updateTurnLabel()
{
    auto string=StringUtils::format("ターン：%d",_turnCounter);
    _turnLabel->setString(string);
}

void SceneGameMain::updatePlayerLabel()
{
    auto string=StringUtils::format("プレイヤー%d",this->getActivePlayerIndex()+1);
    _playerLabel->setString(string);
}

void SceneGameMain::updateUI()
{
    this->updatePlayerLabel();
    this->updateTurnLabel();
}
//end/UI関係



////ターン処理関係
void SceneGameMain::beginGame()
{
    _gameState=GameState::GAMESTART;
    /*
    //船初期配置 vec2(0,8) vec2(12,0)
    Vec2 stagePosition[2][4]={{Vec2(0,8),Vec2(0,6),Vec2(1,7),Vec2(2,8)},
        {Vec2(12,0),Vec2(10,0),Vec2(11,1),Vec2(12,2)}};
    
    for(auto pi=0;pi<this->getPlayerListCount();pi++){
        auto player=this->getPlayer(pi);
        for(auto fi=0;fi<player->getFuneListCount();fi++){
            auto fune=player->getFune(fi);
            _stage->setFuneLayer(fune);
            _stage->positionFune(fune,stagePosition[pi][fi]);
        }
    }*/
    
    auto winSize=Director::getInstance()->getWinSize();
    
    auto inTime=0.5f;//画面中央までの時間
    auto stopTime=1.5f;//画面中央に留まる時間
    auto outTime=0.5f;//画面から消える時間
    //下から左へ 次の処理へのcallbackあり
    auto sprite1=Sprite::create("images/start.png");
    sprite1->setScale(2);
    sprite1->setPosition(Vec2(winSize.width/2,0));
    sprite1->setOpacity(0);
    sprite1->runAction(Sequence::create(Spawn::create(FadeIn::create(inTime),
                                                      MoveTo::create(inTime,Vec2(winSize.width/2,winSize.height/2)),
                                                      NULL),
                                        DelayTime::create(stopTime),
                                        Spawn::create(FadeOut::create(outTime),
                                                      MoveBy::create(outTime,Vec2(-winSize.width,0)),
                                                      NULL),
                                        RemoveSelf::create(),
                                        CallFunc::create([this]{this->startTurn();}),
                                        NULL));
    
    //上から右へ
    auto sprite2=Sprite::create("images/start.png");
    sprite2->setScale(2);
    sprite2->setPosition(Vec2(winSize.width/2,winSize.height));
    sprite2->setOpacity(0);
    sprite2->runAction(Sequence::create(Spawn::create(FadeIn::create(inTime),
                                                      MoveTo::create(inTime,Vec2(winSize.width/2,winSize.height/2)),
                                                      NULL),
                                        DelayTime::create(stopTime),
                                        Spawn::create(FadeOut::create(outTime),
                                                      MoveBy::create(outTime,Vec2(winSize.width,0)),
                                                      NULL),
                                        RemoveSelf::create(),
                                        NULL));
    
    _labelLayer->addChild(sprite1);
    _labelLayer->addChild(sprite2);
    /*
    this->runAction(Sequence::create(DelayTime::create(0.5),
                                     CallFunc::create([this]{this->startTurn();}),
                                     NULL));
     */
}

bool SceneGameMain::endGame()
{
    Vector<Player*> live;//船があるプレイヤー
    Vector<Player*> dead;//船のないプレイヤー
    for(auto& player : _playerList){
        if(player->getFuneListCount()){
            live.pushBack(player);
        }else{
            dead.pushBack(player);
        }
    }
    
    auto liveCount=static_cast<int>(live.size());
    if(liveCount<=1){//ゲーム終了
        _gameState=GameState::GAMEEND;
        this->getEventDispatcher()->removeAllEventListeners();//全てのイベントを無効化
        
        Node* label1;
        Node* label2=nullptr;
        if(liveCount==1){//勝者がいる場合
            auto index=this->getPlayerIndex(live.at(0));
            label1=Sprite::create("images/win.png");
            if(index>=2){//プレイヤーが3人以上いる 想定外
                label2=Label::createWithSystemFont(StringUtils::format("プレイヤー%d",index+1)
                                                   ,"Arial",48);
                label2->setColor(Color3B::RED);
            }else{
                label2=Sprite::create(StringUtils::format("images/playerBanner%d.png",index+1));
            }
        }else{//生き残りがいないので引き分け 想定外
            label1=Label::createWithSystemFont("引き分け","Arial",48);
            label1->setColor(Color3B::ORANGE);
        }
        auto layer=Layer::create();
        auto winSize=Director::getInstance()->getWinSize();
        label1->setPosition(Vec2(winSize.width/2,winSize.height/2+100));
        label1->setScale(0);
        label1->runAction(ScaleTo::create(1,1));
        layer->addChild(label1);
        if(label2){
            label2->setPosition(Vec2(winSize.width/2,winSize.height/2-50));
            label2->setOpacity(0);
            label2->runAction(FadeIn::create(2));
            layer->addChild(label2);
        }
        
        auto label3=Label::createWithSystemFont("タイトルへ","Arial",48);
        label3->setColor(Color3B::RED);
        label3->setPosition(Vec2(winSize.width/2,winSize.height/2-200));
        label3->setOpacity(0);
        layer->addChild(label3);
        scheduleOnce([this,layer,label3](float dt){
            label3->runAction(RepeatForever::create(Sequence::create(
                                                                     FadeIn::create(1),
                                                                     FadeOut::create(1),
                                                                     NULL)));
            
            auto listener=EventListenerTouchOneByOne::create();
            listener->onTouchBegan=[](Touch* touch,Event* event){
                return true;
            };
            listener->onTouchEnded=[this,layer](Touch* touch,Event* event){
                this->getEventDispatcher()->removeAllEventListeners();//イベントを無効化しておく
                this->runAction(Sequence::create(DelayTime::create(0.5),
                                                 CallFunc::create([]{
                    auto scene=Title::createScene();
                    auto transition=TransitionSplitRows::create(1,scene);
                    Director::getInstance()->replaceScene(transition);
                }),NULL));
            };
            Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, layer);
        },2,"RESULTACTION");
        
        this->addChild(layer,static_cast<int>(LayerZPositions::RESULT));
        
        return true;
    }else{//ゲーム続行
        return false;
    }
}

void SceneGameMain::startTurn()
{
    _gameState=GameState::TURNSTART;
    if(!this->getActivePlayer()->getFuneListCount()){//プレイヤーに船が無ければ飛ばす
        this->endTurn();
    }
    
    this->updateUI();
    
    this->startTurnUI(CallFunc::create([this]{this->updateTurn();}));
}

void SceneGameMain::startTurnUI(cocos2d::CallFunc *callfunc)
{
    auto playerIndex=this->getActivePlayerIndex();//現在のプレイヤーのindex
    
    Node* label;
    if(playerIndex<2){
        label=Sprite::create(StringUtils::format("images/playerBanner%d.png",playerIndex+1));
    }else{//プレイヤーが3人以上　想定外
        label=Label::createWithSystemFont(StringUtils::format("player%d",playerIndex+1),"Arial",48);
    }
    auto winSize=Director::getInstance()->getWinSize();
    label->setPosition(Vec2(winSize.width/2,winSize.height/2));
    label->setOpacity(0);
    label->runAction(Sequence::create(FadeIn::create(0.5),
                                      DelayTime::create(0.7),
                                      FadeOut::create(0.4),
                                      RemoveSelf::create(),
                                      callfunc,NULL));
    if(!_labelLayer){
        this->createLabelLayer();
    }
    _labelLayer->addChild(label);
}

void SceneGameMain::updateTurn()
{
    _gameState=GameState::TURNUPDATE;
    
    auto player=this->getActivePlayer();
    auto fune=player->getActiveFuneA();
    _stage->setAStarMap(fune,player->getFuneList());
    _stage->createAreaRangeLayer(fune);
    
    if(player->getIsCPU()){//CPUの場合はsimulatePlayを呼び出す
        this->simulatePlay();
    }else{
        _gameState=GameState::INPUTWAITING;
    }
}

void SceneGameMain::endTurn()
{
    _gameState=GameState::TURNEND;
    _stage->removeAreaRangeLayer();
    
    if(!this->endGame()){
        if(_turnActionPlusCount>=1 && this->getActivePlayer()->getFuneListCount()>=1){
            //行動回数が残っており、かつ、船が残っている場合
            _turnActionPlusCount--;
            updateTurn();
        }else{
            _turnActionPlusCount=0;
            _turnCounter++;
            this->startTurn();
        }
    }
}
//end/ターン処理関係



void SceneGameMain::moveProcess(Fune* fune,const Vec2& position)
{
    _stage->removeAreaRangeLayer();
    _stage->moveFuneAnimation(fune,position,
                              CallFunc::create([this]{
        this->endTurn();
    }));
}

void SceneGameMain::normalAttack(Fune *attackFune,Fune *defenseFune)
{
    _gameState=GameState::ACTION;
    _stage->removeAreaRangeLayer();
    
    std::string string;//ダメージ表記に使用する文字列
    
    //ダメージ処理と船の削除処理はアニメーション前に終わらせる
    string=this->damageProcess(attackFune,defenseFune);
    
    this->deleteFune();
    //
    
    this->damageAnimationProcess(defenseFune,string,
                                 CallFunc::create([this,defenseFune]{
        this->endTurn();
    }));
}

std::string SceneGameMain::damageProcess(const Fune *attackFune,Fune *defenseFune)
{
    int damage;//ダメージ
    auto baseDamage=attackFune->getAttack();//ベースダメージ
    //auto variance=random(-0.5,0.5);//ダメージのぶれ
    auto variance=this->generateRandomFloat(-0.5,0.5);//ダメージのぶれ
    auto varianceDamage=(baseDamage/10)*variance;//ベースダメージ±5%の範囲で変動
    //auto attackRoll=random(1,100);//攻撃の判定
    auto attackRoll=this->generateRandomInt(0,100);//攻撃の判定
    std::string string;//ダメージ表記に使用する文字列
    
    //クリティカルヒット10%
    //ミス10%
    if(attackRoll>90){
        damage=ceilf((baseDamage+varianceDamage)*2);//クリティカルでベースダメージ2倍
    }else if(attackRoll<10){
        damage=0;//ミスならダメージ0;
    }else{
        damage=ceilf(baseDamage+varianceDamage);
    }
    
    if(damage>0){//攻撃が当たった場合
        if(defenseFune->getIsInvincible()){//船が無敵状態なら
            defenseFune->setIsInvincible(false);//無敵を解除
            string="BLOCK!";
        }else{
            auto actualDamage=std::max(damage-defenseFune->getDefense(),1);//最低ダメージは1
            defenseFune->receiveDamage(actualDamage);
            string=StringUtils::format("%d!",actualDamage);
        }
    }else{//回避した場合
        string="MISS!";
    }
    
    return string;
}

void SceneGameMain::damageAnimationProcess(Fune *targetFune,const std::string &string,CallFunc *callFunc)
{
    callFunc->retain();//ActionManagerに登録される前にAutoReleaseで消えるのでretainする
    //爆発表現
    _stage->effectExplosion(targetFune->getTiledMapPosition(),
                            CallFunc::create([this,targetFune,string,callFunc]{
        targetFune->updateHpGauge();//HPゲージ更新
        //ダメージ表示
        //_stage->damageLabel(targetFune,string,callFunc);
        if(targetFune->getHp()<=0){//沈没した時
            _stage->damageLabel(targetFune,string,CallFunc::create([targetFune,callFunc]{
                targetFune->sinkShip(callFunc);
                callFunc->release();//ActionManagerに登録したのでreleaseしておく
            }));
        }else{//沈没しなかったとき
            _stage->damageLabel(targetFune,string,callFunc);
            callFunc->release();//ActionManagerに登録したのでreleaseしておく
        }
        //callFunc->release();//ActionManagerに登録したのでreleaseしておく
    }));
    if(string=="BLOCK!"){//攻撃がブロックされたときはシールドエフェクト
        _stage->effectShield(targetFune);
    }
}



////スキル関係
void SceneGameMain::skillHeal(Fune* fune)
{
    _gameState=GameState::ACTION;
    _stage->removeAreaRangeLayer();
    
    fune->useSkill();
    
    Vector<Fune*> funeList;
    for(auto& player : _playerList){
        if(player->getFuneListContains(fune)){
            funeList=player->getFuneList();
            break;
        }
    }
    
    for(auto& fune : funeList){//回復処理
        fune->receiveDamage(-(fune->getMaxHP()/2));
    }
    
    scheduleOnce([this,funeList](float dt){
        auto flag=true;//endTurnを使用するフラグ
        for(auto& fune : funeList){
            fune->updateHpGauge();
            _stage->effectLabel(fune,
                                StringUtils::toString(fune->getMaxHP()/2),
                                Color3B::GREEN,
                                CallFunc::create([this,flag]{
                if(flag){this->endTurn();}//ループの1回目のみ処理
            })
                                );
            flag=false;//1回目のみtrue
        }
    },0.5,"skillHealAction");
    
}

void SceneGameMain::skillIronShield(Fune *fune)
{
    _gameState=GameState::ACTION;
    _stage->removeAreaRangeLayer();
    
    fune->useSkill();
    fune->setIsInvincible(true);
    
    scheduleOnce([this,fune](float dt){
        _stage->effectShield(fune);
        _stage->effectLabel(fune,"INVINCIBLE",Color3B::BLUE,CallFunc::create([this]{
            this->endTurn();
        }));
    },0.5,"skillIronShieldAction");
}

void SceneGameMain::skillBulletStorm(Fune *fune)
{
    _gameState=GameState::ACTION;
    _stage->removeAreaRangeLayer();
    
    Vector<Fune*> targetFuneList;
    std::vector<std::string> damageStrings;
    
    for(auto& player : _playerList){
        if(!player->getFuneListContains(fune)){//攻撃をした船が所属するプレイヤー以外
            for(auto& targetFune : player->getFuneList()){
                if(fune->withinRange(_stage->getDistance(fune->getTiledMapPosition(),
                                                         targetFune->getTiledMapPosition()))){
                    //攻撃範囲内
                    targetFuneList.pushBack(targetFune);
                    damageStrings.push_back(this->damageProcess(fune,targetFune));
                }
            }
        }
    }
    
    auto sinkFlag=this->deleteFune();//撃沈された船があるか
    
    scheduleOnce([this,targetFuneList,damageStrings,fune,sinkFlag](float dt){
        if(targetFuneList.empty()){//攻撃対象がいない場合はキャンセル
            _stage->effectLabel(fune,"攻撃範囲外です",Color3B::RED,CallFunc::create([this]{
                this->updateTurn();
            }));
        }else{
            fune->useSkill();
            auto flag=sinkFlag;
            for(int i=0;i<targetFuneList.size();i++){
                auto targetFune=targetFuneList.at(i);
                auto string=damageStrings.at(i);
                CallFunc* callFunc;
                if(i==0 && !flag){//撃沈された船がない場合は一番最初の船に
                    callFunc=CallFunc::create([this]{
                        this->endTurn();
                    });
                }else if(flag && targetFune->getHp()<=0){//撃沈された船の一つ目
                    callFunc=CallFunc::create([this,targetFune]{
                        this->endTurn();
                    });
                    flag=false;//２箇所以上設定しないためにfalseにしておく
                }else{
                    callFunc=CallFunc::create([]{});//一つを除いて処理は設定しない
                }
                this->damageAnimationProcess(targetFune,string,callFunc);
            }
        }
    },0.5,"skillBulletStormAction");
}

void SceneGameMain::skillHurryUp(Fune *fune)
{
    _gameState=GameState::ACTION;
    _turnActionPlusCount=2;//2回連続で行動できるようにする
    _stage->removeAreaRangeLayer();
    
    fune->useSkill();
    
    scheduleOnce([this,fune](float dt){
        _stage->effectWheel(fune->getTiledMapPosition(),false);
        _stage->effectLabel(fune,"DOUBLE ACTION",
                            Color3B::GREEN,
                            CallFunc::create([this]{
            this->endTurn();
        }));
    },0.5,"skillHurryUpAction");
}

void SceneGameMain::skillDebug(Fune *fune)
{
    _gameState=GameState::ACTION;
    for(auto& player:_playerList){
        if(player->getFuneListContains(fune)){continue;}
        for(auto& targetFune:player->getFuneList()){
            targetFune->receiveDamage(targetFune->getMaxHP());
            targetFune->removeFromParent();
        }
    }
    this->deleteFune();
    this->endTurn();
}
//end/スキル船



bool SceneGameMain::deleteFune()
{
    Vector<Fune*> sinkFuneList;
    for(auto& fune : _stage->getFuneList()){
        if(fune->getHp()<=0){
            sinkFuneList.pushBack(fune);
        }
    }
    
    if(!sinkFuneList.empty()){
        for(auto& fune : sinkFuneList){
            for(auto& player : _playerList){
                if(player->getFuneListContains(fune)){
                    player->removeFuneList(fune);
                    break;
                }
            }
            _stage->removeFuneList(fune);
        }
        return true;
    }else{
        return false;
    }
}


void SceneGameMain::simulatePlay()
{
    auto player=this->getActivePlayer();
    auto funeList=player->getFuneList();
    
    //攻撃範囲内に敵船がいれば攻撃する
    for(auto& anotherPlayer:_playerList){
        if(anotherPlayer==player){continue;}
        auto targetFuneList=anotherPlayer->getFuneList();
        for(auto& fune:funeList){
            for(auto& targetFune:targetFuneList){
                if(fune->withinRange(_stage->getDistance(fune->getTiledMapPosition(),
                                                         targetFune->getTiledMapPosition()))){
                    this->normalAttack(fune,targetFune);
                    return;
                }
            }
        }
    }
    
    while(true){
        auto index=random(0,static_cast<int>(funeList.size()-1));
        auto fune=funeList.at(index);
        for(int i=-fune->getMovement();i<=fune->getMovement();i++){
            for(int j=-fune->getMovement();j<=fune->getMovement();j++){
                auto position=fune->getTiledMapPosition()+Vec2(i,j);
                if(_stage->getOnTiledMapFune(position)){continue;}
                auto distance=_stage->getDistance(fune->getTiledMapPosition(),position);
                if(distance<=fune->getMovement() && _stage->getAStar()->checkLine(fune->getTiledMapPosition(),
                                                                                  position,fune->getMovement())){
                    this->moveProcess(fune,position);
                    return;
                }
            }
        }
        funeList.eraseObject(fune);
    }
}

int SceneGameMain::generateRandomInt(int min,int max)
{
    std::uniform_int_distribution<int> dest(min,max);
    return dest(_engine);
}

float SceneGameMain::generateRandomFloat(float min,float max)
{
    //maxも範囲に入るように設定
    std::uniform_real_distribution<float> dest(min,std::nextafter(max,std::numeric_limits<float>::max()));
    return dest(_engine);
}