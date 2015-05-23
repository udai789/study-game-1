//
//  Stage.cpp
//  pirateTaxtics
//
//  Created by kk on 2015/05/05.
//
//

#include "Stage.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"
#include "SimpleAudioEngine.h"

USING_NS_CC;

const char* MAP_FILE_FORMAT="map/stage%d.tmx";
const char* BACKGROUND_FILE_FORMAT="images/map0%d.png";
const char* MAP_LAYER_NAME="MapTiles";
const char* TILE_CATEGOLY_TYPE="displayData";
const int MAP_POSITION_TOP=10;//マップの画面上からの位置
const int MAP_POSITION_LEFT=64;//マップの画面左からの位置

Stage::Stage()
:_tiledMap(nullptr)
,_makerLayer(nullptr)
,_areaRangeLayer(nullptr)
,_unitLayer(nullptr)
,_helpLayer(nullptr)
,_aStar(nullptr)
,_level(0)
{
    
}

Stage::~Stage()
{
    CC_SAFE_RELEASE_NULL(_tiledMap);
    CC_SAFE_RELEASE_NULL(_makerLayer);
    CC_SAFE_RELEASE_NULL(_areaRangeLayer);
    CC_SAFE_RELEASE_NULL(_unitLayer);
    CC_SAFE_RELEASE_NULL(_helpLayer);
    CC_SAFE_RELEASE_NULL(_aStar);
}

Stage* Stage::createWithLevel(int level)
{
    Stage* ret=new Stage();
    if(ret->initWithLevel(level)){
        ret->autorelease();
        return ret;
    }
    
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool Stage::initWithLevel(int level)
{
    if(!Layer::init()){
        return false;
    }
    
    _level=level;//ステージ番号を格納
    
    this->createMap();//マップ作成
    
    this->createMarkerLayer();//マーカーレイヤー作成
    
    this->createUnitLayer();//ユニットレイヤーの作成
    
    this->createAStar();//試験中 探索マップを作成
    
    return true;
}

void Stage::createMap()
{
    //画面のサイズを取り出す
    auto size=Director::getInstance()->getWinSize();
    
    ////枠と背景の作成
    //枠のスプライトを作成
    auto frame=Sprite::create("images/mapframe.png");
    //枠の表示位置を設定する　画面左上に配置
    frame->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
    frame->setPosition(Vec2(0,size.height));
    this->addChild(frame);
    //背景とマップの位置は画面左上から(64,10)
    //ステージ背景
    auto backgroundFile=StringUtils::format(BACKGROUND_FILE_FORMAT,_level);
    auto background=Sprite::create(backgroundFile);
    background->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
    background->setPosition(Vec2(MAP_POSITION_LEFT,size.height -MAP_POSITION_TOP));
    this->addChild(background);
    //マップファイルからノードを作成する
    auto mapFile=StringUtils::format(MAP_FILE_FORMAT,_level);
    auto map=TMXTiledMap::create(mapFile);
    map->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
    map->setPosition(Vec2(MAP_POSITION_LEFT,size.height-MAP_POSITION_TOP));
    this->addChild(map);
    this->setTiledMap(map);
    ////*/枠と背景の作成
}

void Stage::createMarkerLayer()
{
    //タイルサイズを取り出す
    auto tileSize=_tiledMap->getLayer(MAP_LAYER_NAME)->getMapTileSize();
    auto markerLayer=Layer::create();
    this->setMakerLayer(markerLayer);
    this->addChild(markerLayer,static_cast<int>(LayerZPositions::MARKER));
    for(auto i=0;i<2;i++){
        auto marker=Sprite::create("images/mapui.png",Rect(tileSize.width*i,0,tileSize.width,tileSize.height));
        marker->setVisible(false);
        marker->setTag(i);
        markerLayer->addChild(marker);
    }
}

void Stage::createUnitLayer()
{
    auto unitLayer=Layer::create();
    this->setUnitLayer(unitLayer);
    this->addChild(unitLayer,static_cast<int>(LayerZPositions::UNIT));
}

void Stage::createAStar()
{
    auto mapSize=_tiledMap->getMapSize();//マップの大きさ
    auto aStar=AStar::create(mapSize);//探索マップを作成
    this->setAStar(aStar);
    
    //各タイル(点)を作成
    for(int j=0;j<mapSize.height;j++){
        for(int i=0;i<mapSize.width;i++){
            auto type=this->getTileType(Vec2(i,j));//この座標のタイルタイプ
            int inCost=1;//そのタイルに入るコスト
            int outCost=0;//そのタイルから出るコスト
            //タイルの種類によってコストを調整
            switch (type) {
                case TileTypes::IWA:
                case TileTypes::RIKU:
                    inCost=AStar::getMaxCost();//コストを最大に設定
                    outCost=AStar::getMaxCost();
                    break;
                    
                case TileTypes::ASAI:
                    inCost=2;
                    break;
                    
                case TileTypes::ARAI:
                    outCost=2;
                    break;
                    
                default:
                    break;
            }
            if(!aStar->addPoint(Vec2(i,j),inCost,outCost)){
                log("Stage::createAStar() aStar->addPoint() ERROR");
            }
        }
    }
}

bool Stage::tileMoveCheck(const Vec2 &position)
{
    auto tileType=this->getTileType(position);
    if(tileType==TileTypes::IWA || tileType==TileTypes::RIKU || tileType==TileTypes::NONE){
        return false;
    }
    
    return true;
}

Vec2 Stage::convertToTiledMapSpace(const Vec2 &stagePosition)
{
    auto winSize=Director::getInstance()->getWinSize();//画面サイズ
    auto tileSize=_tiledMap->getLayer(MAP_LAYER_NAME)->getMapTileSize();
    /*マップ上の座標に変換
     *winSize.height-stagePosition.y-MAP_POSITION_TOP マップタイルの座標系は左上が(0,0)なので上側を基準にした座標に変換
     */
    auto positon=Vec2(stagePosition.x-MAP_POSITION_LEFT,
                      winSize.height-stagePosition.y-MAP_POSITION_TOP);
    auto x=floor(positon.x/tileSize.width);
    auto y=floor(positon.y/tileSize.height);
    
    return std::move(Vec2(x,y));
}

Vec2 Stage::convertToStageSpace(const cocos2d::Vec2 &position)
{
    auto winSize=Director::getInstance()->getWinSize();//画面サイズ
    auto tileSize=_tiledMap->getLayer(MAP_LAYER_NAME)->getMapTileSize();//タイルのサイズ
    auto mapSize=_tiledMap->getMapSize();//タイル数
    //ステージ上の座標に変換
    auto x=tileSize.width*(position.x+0.5)+MAP_POSITION_LEFT;
    auto y=winSize.height-tileSize.height*(position.y+0.5)-MAP_POSITION_TOP;
    
    return std::move(Vec2(x,y));
}

Stage::TileTypes Stage::getTileType(const Vec2 &position)
{
    auto gid=_tiledMap->getLayer(MAP_LAYER_NAME)->getTileGIDAt(position);//positionにあるタイルのidを取得
    auto properties=_tiledMap->getPropertiesForGID(gid).asValueMap();//タイルのプロパティを取り出す
    if(properties.count(TILE_CATEGOLY_TYPE)>0){//TILE_CATEGOLY_TYPEが定義されている場合
        return static_cast<TileTypes>(properties.at(TILE_CATEGOLY_TYPE).asInt());
    }
    
    return TileTypes::NONE;
}

bool Stage::onTiledMapCheck(const Vec2 &stagePosition)
{
    auto position=this->convertToTiledMapSpace(stagePosition);
    auto mapSize=_tiledMap->getMapSize();//タイル数
    
    if(position.x<0 || position.x>=mapSize.width ||
       position.y<0 || position.y>=mapSize.height){
        return false;
    }
    
    return true;
}

void Stage::positionObject(Sprite* sprite,const Vec2 &position)
{
    auto stagePosition=this->convertToStageSpace(position);
    sprite->setPosition(stagePosition);
}

void Stage::positionFune(Fune *fune,const Vec2 &position)
{
    this->positionObject(fune,position);
    fune->setTiledMapPosition(position);
}

void Stage::moveFuneAnimation(Fune *fune,const cocos2d::Vec2 &position,cocos2d::CallFunc *callfunc)
{
    /*
    auto distance=this->getDistance(fune->getTiledMapPosition(),position);
    auto duration=std::min(1.2,distance*0.3);
    fune->setLocalZOrder(100);
    fune->runAction(Sequence::create(EaseInOut::create(
                                                    MoveTo::create(duration,
                                                                   this->convertToStageSpace(position)),
                                                    duration),
                                     CallFunc::create([this,fune,position]{
        fune->setLocalZOrder(0);
        this->positionFune(fune,position);}
                                                      ),
                                     callfunc,NULL));
     */
    auto duration=0.4f;
    auto line=this->getAStar()->search(fune->getTiledMapPosition(),position,fune->getMovement());
    Vector<FiniteTimeAction*> move;
    for(int i=1;i<line.size();i++){//先頭は船の現在地のため1から
        auto movePosition=line.at(i);//移動先
        auto anotherFune=this->getOnTiledMapFune(movePosition);//移動先に他の船がいるか
        if(anotherFune){
            auto ve=movePosition-line.at(i-1);
            if(ve.y<0){//下からマスに入る場合
                //移動する船を下にし、移動終了後上にする
                move.pushBack(CallFunc::create([fune,anotherFune]{
                    fune->setLocalZOrder(1);//2箇所連続の場合に前の船の下に回り込むのを回避
                    anotherFune->setLocalZOrder(100);
                }));
                move.pushBack(MoveTo::create(duration,this->convertToStageSpace(movePosition)));
                move.pushBack(CallFunc::create([fune,anotherFune]{
                    fune->setLocalZOrder(100);
                    anotherFune->setLocalZOrder(0);//zを0に戻しておく
                }));
            }else{//下から入らない場合 移動する船を上にする
                move.pushBack(CallFunc::create([fune]{
                    fune->setLocalZOrder(100);
                }));
                move.pushBack(MoveTo::create(duration,this->convertToStageSpace(movePosition)));
            }
        }else{
            move.pushBack(MoveTo::create(duration,this->convertToStageSpace(movePosition)));
        }
    }
    
    fune->runAction(Sequence::create(Sequence::create(move),
                                     CallFunc::create([this,fune,position]{
        fune->setLocalZOrder(0);//zを0に戻しておく
        this->positionFune(fune,position);
    }
                                                      ),
                                     callfunc,NULL));
}

int Stage::getDistance(const Vec2 &position1,const Vec2 &position2)
{
    auto distance=std::abs(position1.x-position2.x)+std::abs(position1.y-position2.y);
    return distance;
}

void Stage::markerHide()
{
    for(auto i=0;i<2;i++){
        _makerLayer->getChildByTag(i)->setVisible(false);
    }
}

void Stage::markerShow(Stage::MapMarkerTypes markerType,const Vec2 &position)
{
    //ばつマーカーでない場合は終了
    if(!(markerType==MapMarkerTypes::REDCROSS || markerType==MapMarkerTypes::GRAYCROSS)){
        return;
    }
    this->markerHide();
    auto marker=dynamic_cast<Sprite*>(_makerLayer->getChildByTag(static_cast<int>(markerType)));
    marker->setVisible(true);
    this->positionObject(marker,position);
}

void Stage::setMarker(Fune *fune,const cocos2d::Vec2 &position)
{
    auto distance=this->getDistance(position,fune->getTiledMapPosition());
    //if(this->tileMoveCheck(position) && distance<=fune->getMovement()){
    if(distance<=fune->getMovement() &&
       this->getAStar()->checkLine(fune->getTiledMapPosition(),position,fune->getMovement())){
        this->markerShow(Stage::MapMarkerTypes::REDCROSS,position);
    }else{
        this->markerShow(Stage::MapMarkerTypes::GRAYCROSS,position);
    }
}

void Stage::createAreaRangeLayer(Fune *fune)
{
    if(_areaRangeLayer !=nullptr){
        this->removeAreaRangeLayer();
    }
    auto areaRangeLayer=Layer::create();
    auto tileSize=_tiledMap->getLayer(MAP_LAYER_NAME)->getMapTileSize();
    //移動可能かを表すスプライトを配置
    for(auto j=-fune->getMovement();j<=fune->getMovement();j++){
        for(auto i=-fune->getMovement();i<=fune->getMovement();i++){
            auto position=fune->getTiledMapPosition()+Vec2(i,j);//調べる先の座標
            auto stagePosition=this->convertToStageSpace(position);
            auto distance=this->getDistance(position,fune->getTiledMapPosition());
            //移動先がタイルマップ上にないか、船の移動距離が足りなければcontinue
            if(!this->onTiledMapCheck(stagePosition) || distance>fune->getMovement()){
                continue;
            }
            //移動可能なタイルなら青、不可なら赤
            //auto cover=this->tileMoveCheck(position) ? Stage::MapMarkerTypes::BLUECOVER : Stage::MapMarkerTypes::REDCOVER;
            auto cover=this->getAStar()->checkLine(fune->getTiledMapPosition(),position,fune->getMovement()) ?
                Stage::MapMarkerTypes::BLUECOVER : Stage::MapMarkerTypes::REDCOVER;
            auto sprite=Sprite::create("images/mapui.png",Rect(
                                                           tileSize.width*static_cast<int>(cover),0,
                                                           tileSize.width,tileSize.height
                                                           ));
            this->positionObject(sprite,position);
            areaRangeLayer->addChild(sprite);
        }
    }
    
    this->addChild(areaRangeLayer,static_cast<int>(LayerZPositions::AREARANGE));
    this->setAreaRangeLayer(areaRangeLayer);
}

void Stage::removeAreaRangeLayer()
{
    if(_areaRangeLayer){
        _areaRangeLayer->removeFromParent();
        this->setAreaRangeLayer(nullptr);
    }
}

void Stage::addFuneList(Fune *fune)
{
    _funeList.pushBack(fune);
}

void Stage::removeFuneList(Fune *fune)
{
    _funeList.eraseObject(fune);
}

void Stage::setFuneLayer(Fune *fune)
{
    _unitLayer->addChild(fune);
}

Fune* Stage::getOnTiledMapFune(cocos2d::Vec2 &position)
{
    for(auto& fune : _funeList){
        if(position==fune->getTiledMapPosition()){
            return fune;
        }
    }
    return nullptr;
}

void Stage::createHelpLayer(Fune *fune)
{
    if(_helpLayer){
        this->removeHelpLayer();
    }
    
    auto helpLayer=Layer::create();
    helpLayer->setColor(Color3B::GRAY);
    helpLayer->setOpacity(0.7);
    
    auto winSize=Director::getInstance()->getWinSize();
    
    /*
    //背景
    auto windowSprite=Sprite::create("images/window.png");
    windowSprite->setPosition(Vec2(winSize.width/2,winSize.height/2));
    helpLayer->addChild(windowSprite);
    
    //ステータス
    auto captainLabel=Label::createWithSystemFont(
                                                  StringUtils::format("船長%d",static_cast<int>(fune->getType())),
                                                  "Arial",48);
    captainLabel->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
    captainLabel->setPosition(Vec2(winSize.width/2-162,winSize.height/2+220));
    helpLayer->addChild(captainLabel);
     */
    
    auto window=CSLoader::getInstance()->createNode("MainScene.csb");
    window->setScale(0.7);
    window->runAction(ScaleTo::create(0.2,1));
    helpLayer->addChild(window);
    
    //ステータス
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
    
    //戻るボタン
    auto backButton=MenuItemImage::create("images/btnCancel.png",
                                          "images/btnCancel.png",
                                          [this](Ref *ref){
                                              //戻るボタンを押した時の処理
                                              auto button=dynamic_cast<MenuItemImage*>(ref);
                                              this->getEventDispatcher()->removeEventListenersForTarget(button);
                                              button->runAction(Sequence::create(ScaleTo::create(0.2,1.3),
                                                                           CallFunc::create(
                                                                                            [this]{this->removeHelpLayer();}
                                                                                            ),NULL));
                                          });
    //メニューを作成
    auto menu=Menu::create(backButton,NULL);
    menu->setPosition(Vec2(340,140));
    menu->setOpacity(0);
    menu->runAction(FadeIn::create(0.2));
    helpLayer->addChild(menu);
    
    //他のタッチイベントが発生しないようにする
    auto listener=EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan=[](Touch* touch,Event* event){return true;};
    Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener,helpLayer);
    
    this->addChild(helpLayer,static_cast<int>(LayerZPositions::HELP));
    this->setHelpLayer(helpLayer);
}

void Stage::removeHelpLayer()
{
    if(_helpLayer){
        Director::getInstance()->getEventDispatcher()->removeEventListenersForTarget(_helpLayer);
        _helpLayer->removeFromParent();
        this->setHelpLayer(nullptr);
    }
}

void Stage::effectExplosion(const Vec2 &position,CallFunc* callfunc)
{
    //アニメーションを作成
    Vector<SpriteFrame*> explosionFrames;
    Vector<SpriteFrame*> frames1;
    Vector<SpriteFrame*> frames2;
    const int animationFrameCount=5;//アニメーションのフレーム数
    auto explosionSize=Size(32,32);
    
    for(auto i=0;i<animationFrameCount;i++){
        auto rect=Rect(explosionSize.width*i,0,explosionSize.width,explosionSize.height);
        auto frame=SpriteFrame::create("images/explosion.png",rect);
        explosionFrames.pushBack(frame);
    }
    
    //フレーム順序
    for(auto i=0;i<3;i++){
        frames1.pushBack(explosionFrames.at(i));
    }
    frames2.pushBack(explosionFrames.at(3));
    frames2.pushBack(explosionFrames.at(4));
    
    //表示するスプライトを作成
    auto sprite=Sprite::create("images/explosion.png",Rect(0,0,explosionSize.width,explosionSize.height));
    sprite->setPosition(this->convertToStageSpace(position));
    _unitLayer->addChild(sprite);
    
    
    auto animation1=Animation::createWithSpriteFrames(frames1,5.0/60.0);
    //爆発エフェクトと効果音を同時に
    auto spawn=Spawn::create(
                         CallFunc::create([]{
        CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sound/bomb1.wav");
    }),
                         Animate::create(animation1),NULL);
    auto repeat=Repeat::create(spawn,3);//3回繰り返す
    auto animation2=Animation::createWithSpriteFrames(frames2,5.0/60.0);
    sprite->runAction(Sequence::create(repeat,
                                       Animate::create(animation2),
                                       RemoveSelf::create(),
                                       callfunc,NULL));
    /*
    auto animation1=Animation::createWithSpriteFrames(frames1,5.0/60.0);
    animation1->setLoops(3);//3回繰り返し
    auto animation2=Animation::createWithSpriteFrames(frames2,5.0/60.0);
    sprite->runAction(Sequence::create(Animate::create(animation1),
                                       Animate::create(animation2),
                                       RemoveSelf::create(),
                                       callfunc,NULL));
     */
}

void Stage::damageLabel(Fune *fune,std::string string,cocos2d::CallFunc *callfunc)
{
    auto label=Label::createWithSystemFont(string,"Arial",36);
    label->setColor(Color3B::RED);
    if(fune->getPosition().x>Director::getInstance()->getWinSize().width*2/3){//画面の2/3から右側
        label->setAnchorPoint(Vec2::ANCHOR_BOTTOM_RIGHT);
        label->setPosition(fune->getPosition()-Vec2(32,0));
    }else{
        label->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
        label->setPosition(fune->getPosition()+Vec2(32,0));
    }
    _unitLayer->addChild(label);
    
    label->runAction(Sequence::create(MoveBy::create(0.5f,Vec2(0,32)),
                                      RemoveSelf::create(),
                                      callfunc,NULL));
}