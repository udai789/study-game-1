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
#include "SceneGameMain.h"
#include "SimpleAudioEngine.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"

USING_NS_CC;

const float STANDARD_BGM_VOLUME=0.03f;//初期BGM音量
const float STANDARD_EFFECTS_VOLUME=0.5f;//初期効果音音量

SceneGameMain::SceneGameMain()
:_turnCounter(0)
,_gameState(GameState::GAMESTART)
,_stage(nullptr)
,_labelLayer(nullptr)
,_turnLabel(nullptr)
,_playerLabel(nullptr)
{
    
}

SceneGameMain::~SceneGameMain()
{
    CC_SAFE_RELEASE_NULL(_stage);
    CC_SAFE_RELEASE_NULL(_labelLayer);
    CC_SAFE_RELEASE_NULL(_turnLabel);
    CC_SAFE_RELEASE_NULL(_playerLabel);
}


Scene* SceneGameMain::createSceneWithLevel(int level)
{
    auto scene=Scene::create();
    auto layer=new SceneGameMain();
    if(layer && layer->initWithLevel(level)){
        layer->autorelease();
    }else{
        CC_SAFE_DELETE(layer);
    }
    scene->addChild(layer);
    return scene;
}

bool SceneGameMain::initWithLevel(int level)
{
    if(!Layer::init()){
        return false;
    }
    
    this->createMap(level);//ステージマップ
    this->createLabelLayer();//ラベルレイヤーを作成
    this->createPlayers();//プレイヤーを作成
    
    CocosDenshion::SimpleAudioEngine::getInstance()->setBackgroundMusicVolume(STANDARD_BGM_VOLUME);//bgm音量設定
    CocosDenshion::SimpleAudioEngine::getInstance()->setEffectsVolume(STANDARD_EFFECTS_VOLUME);//効果音の音量
    
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
                        _stage->createHelpLayer(targetFune);//ヘルプを表示
                    }else if(player->getFuneListContains(targetFune)){//プレイヤーが所持する船の場合
                        player->setActiveFune(targetFune);
                        this->updateTurn();
                    }else{//他のプレイヤーの船の場合
                        if(fune->withinRange(distance)){//攻撃範囲内
                            this->attack(fune,targetFune);
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
                                               _stage->removeAreaRangeLayer();
                                               _stage->moveFuneAnimation(fune,position,
                                                                         CallFunc::create([this]{
                                                   this->endTurn();
                                               }));
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
    
    this->beginGame();
    return true;
}

void SceneGameMain::onEnterTransitionDidFinish()
{
    Layer::onEnterTransitionDidFinish();
    CocosDenshion::SimpleAudioEngine::getInstance()->playBackgroundMusic("music/highseas.mp3",true);
}

void SceneGameMain::createMap(int level)
{
    auto stage=Stage::createWithLevel(level);
    this->addChild(stage,static_cast<int>(LayerZPositions::STAGE));
    this->setStage(stage);
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

void SceneGameMain::createFune(Player *player,Fune::CharacterTypes characterType)
{
    auto fune=Fune::createWithLevel(characterType);
    _stage->addFuneList(fune);
    player->addFuneList(fune);
}

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

void SceneGameMain::createPlayers()
{
    auto player1=Player::create();
    this->addPlayerList(player1);
    //プレイヤー側のキャラは0-3番
    for(auto i=0;i<4;i++){
        this->createFune(player1,static_cast<Fune::CharacterTypes>(i));
    }
    
    auto player2=Player::create();
    this->addPlayerList(player2);
    //相手側のキャラは4-7番
    for(auto i=4;i<8;i++){
        this->createFune(player2,static_cast<Fune::CharacterTypes>(i));
    }
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

void SceneGameMain::updateUi()
{
    this->updatePlayerLabel();
    this->updateTurnLabel();
}

void SceneGameMain::beginGame()
{
    _gameState=GameState::GAMESTART;
    //船初期配置 vec2(0,8) vec2(12,0)
    Vec2 stagePosition[2][4]={{Vec2(5,4),Vec2(0,6),Vec2(1,7),Vec2(2,8)},
        {Vec2(6,4),Vec2(10,0),Vec2(11,1),Vec2(12,2)}};
    
    for(auto pi=0;pi<this->getPlayerListCount();pi++){
        auto player=this->getPlayer(pi);
        for(auto fi=0;fi<player->getFuneListCount();fi++){
            auto fune=player->getFune(fi);
            _stage->setFuneLayer(fune);
            _stage->positionFune(fune,stagePosition[pi][fi]);
        }
    }
    
    this->startTurn();
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
        this->addChild(layer,static_cast<int>(LayerZPositions::RESULT));
        
        _gameState=GameState::GAMEEND;
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
    
    this->updateUi();
    
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
    
    _gameState=GameState::INPUTWAITING;
}

void SceneGameMain::endTurn()
{
    _gameState=GameState::TURNEND;
    _stage->removeAreaRangeLayer();
    
    if(!this->endGame()){
        _turnCounter++;
        this->startTurn();
    }
}

void SceneGameMain::attack(Fune *attackFune,Fune *defenseFune)
{
    _gameState=GameState::ACTION;
    int damage;//ダメージ
    auto baseDamage=attackFune->getAttack();//ベースダメージ
    auto variance=random(-0.5,0.5);//ダメージのぶれ
    auto varianceDamage=(baseDamage/10)*variance;//ベースダメージ±5%の範囲で変動
    auto attackRoll=random(1,100);//攻撃の判定
    std::string string;//ダメージ表記に使用する文字列
    
    //クリティカルヒット10%
    //ミス10%
    if(attackRoll>=90){
        damage=ceilf((baseDamage+varianceDamage)*2);//クリティカルでベースダメージ2倍
    }else if(attackRoll<=10){
        damage=0;//ミスならダメージ0;
    }else{
        damage=ceilf(baseDamage+varianceDamage);
    }
    
    if(damage>0){//攻撃が当たった場合
        auto actualDamage=std::max(damage-defenseFune->getDefense(),1);//最低ダメージは1
        defenseFune->setHp(defenseFune->getHp()-actualDamage);
        string=StringUtils::format("%d!",actualDamage);
    }else{//回避した場合
        string=StringUtils::format("MISS!");
    }
    
    //爆発表現
    _stage->effectExplosion(defenseFune->getTiledMapPosition(),
                            CallFunc::create([this,defenseFune,string]{
        defenseFune->updateHpGauge();//HPゲージ更新
        //ダメージ表示
        _stage->damageLabel(defenseFune,string,
                            CallFunc::create([this,defenseFune]{
            //沈没判定
            if(defenseFune->getHp()<=0){//撃沈されたなら
                defenseFune->sinkShip(CallFunc::create([this,defenseFune]{
                    //プレイヤーから船を削除
                    for(auto& player : _playerList){
                        if(player->getFuneListContains(defenseFune)){
                            player->removeFuneList(defenseFune);
                            break;
                        }
                    }
                    //stageから船を削除
                    _stage->removeFuneList(defenseFune);
                    //stageレイヤーから船を削除
                    defenseFune->removeFromParent();
                    
                    this->endTurn();
                }));
            }else{//撃沈されなければ
                this->endTurn();
            }
        }));
    }));
}

void SceneGameMain::createSettingWindow()
{
    /*関数定義
     *設定画面のレイヤー作成
     *設定画面の画像作成
     *BGM設定
     *効果音設定
     *設定画面を閉じるボタン設定
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
        }
        if(type==ui::Widget::TouchEventType::ENDED){//タッチ終了
            Director::getInstance()->getEventDispatcher()->removeEventListenersForTarget(settingLayer);//レイヤーのイベント削除
            settingLayer->removeFromParent();//レイヤーを消す
            Director::getInstance()->resume();//再開
            audio->resumeAllEffects();//効果音再開
        }
        if(type==ui::Widget::TouchEventType::CANCELED){//タッチキャンセル
            button->setColor(Color3B::WHITE);
        }
    });
    
    this->addChild(settingLayer,static_cast<int>(LayerZPositions::SETTING));
    Director::getInstance()->pause();//一時停止
    audio->pauseAllEffects();//効果音のみ停止
}