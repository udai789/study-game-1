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
,_unitLayer(nullptr)
,_effectLayer(nullptr)
,_messageLayer(nullptr)
,_aStar(nullptr)
,_level(0)
{
    
}

Stage::~Stage()
{
    CC_SAFE_RELEASE_NULL(_tiledMap);
    CC_SAFE_RELEASE_NULL(_makerLayer);
    CC_SAFE_RELEASE_NULL(_unitLayer);
    CC_SAFE_RELEASE_NULL(_effectLayer);
    CC_SAFE_RELEASE_NULL(_messageLayer);
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
    
    this->createEffectLayer();//エフェクトレイヤーの作成
    this->createMessageLayer();//メッセージレイヤーの作成
    
    this->createAStar();//探索マップを作成
    
    return true;
}



////初期化処理
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
    map->setVisible(false);//隠しておく
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

void Stage::createEffectLayer()
{
    auto effectLayer=Layer::create();
    this->setEffectLayer(effectLayer);
    this->addChild(effectLayer,static_cast<int>(LayerZPositions::EFFECT));
}

void Stage::createMessageLayer()
{
    auto messageLayer=Layer::create();
    this->setMessageLayer(messageLayer);
    this->addChild(messageLayer,static_cast<int>(LayerZPositions::MESSAGE));
}

void Stage::createAStar()
{
    auto mapSize=_tiledMap->getMapSize();//マップの大きさ
    auto aStar=AStar::create();//探索マップを作成
    Map<int,MPoint*> map;
    this->setAStar(aStar);
    //各_orisinalAStarMap用のマップを作成
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
            if(!aStar->createMPoint(map,Vec2(i,j),inCost,outCost)){
                log("Stage::createAStar() aStar->addPoint() ERROR");
            }
        }
    }
    _orisinalAStarMap=map;
}
//end/初期化処理



////座標関係
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

int Stage::getDistance(const Vec2 &position1,const Vec2 &position2)
{
    auto distance=std::abs(position1.x-position2.x)+std::abs(position1.y-position2.y);
    return distance;
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

bool Stage::tileMoveCheck(const Vec2 &position)
{
    auto tileType=this->getTileType(position);
    if(tileType==TileTypes::IWA || tileType==TileTypes::RIKU || tileType==TileTypes::NONE){
        return false;
    }
    
    return true;
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
//end/座標関係



////marker関係
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
//end/marker関係



////areaRange関係
void Stage::createAreaRangeLayer(Fune *fune)
{
    this->removeAreaRangeLayer();
    
    auto areaRangeLayer=Layer::create();
    areaRangeLayer->setTag(static_cast<int>(LayerZPositions::AREARANGE));
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
}

void Stage::removeAreaRangeLayer()
{
    auto layer=this->getChildByTag(static_cast<int>(LayerZPositions::AREARANGE));
    if(layer){
        layer->removeFromParent();
    }
}
//end/areaRange関係



////fune関係
void Stage::addFuneList(Fune *fune)
{
    _funeList.pushBack(fune);
}

void Stage::removeFuneList(Fune *fune)
{
    _funeList.eraseObject(fune);
}

void Stage::positionFune(Fune *fune,const Vec2 &position)
{
    this->positionObject(fune,position);
    fune->setTiledMapPosition(position);
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

void Stage::moveFuneAnimation(Fune *fune,const cocos2d::Vec2 &position,cocos2d::CallFunc *callfunc)
{
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
//end/fune関係



////エフェクト関係
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
    
    _effectLayer->addChild(sprite);
}

void Stage::effectLabel(Fune *fune,std::string string,Color3B color,CallFunc *callFunc){
    auto label=Label::createWithSystemFont(string,"Arial",36);
    label->setColor(color);
    if(fune->getPosition().x>Director::getInstance()->getWinSize().width*2/3){//画面の2/3から右側
        label->setAnchorPoint(Vec2::ANCHOR_BOTTOM_RIGHT);
        label->setPosition(fune->getPosition()-Vec2(32,0));
    }else{
        label->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
        label->setPosition(fune->getPosition()+Vec2(32,0));
    }
    
    label->runAction(Sequence::create(MoveBy::create(0.5f,Vec2(0,32)),
                                      RemoveSelf::create(),
                                      callFunc,NULL));
    _messageLayer->addChild(label);
}

void Stage::damageLabel(Fune *fune,std::string string,CallFunc *callfunc)
{
    this->effectLabel(fune,string,Color3B::RED,callfunc);
    /*
    auto label=Label::createWithSystemFont(string,"Arial",36);
    label->setColor(Color3B::RED);
    if(fune->getPosition().x>Director::getInstance()->getWinSize().width*2/3){//画面の2/3から右側
        label->setAnchorPoint(Vec2::ANCHOR_BOTTOM_RIGHT);
        label->setPosition(fune->getPosition()-Vec2(32,0));
    }else{
        label->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
        label->setPosition(fune->getPosition()+Vec2(32,0));
    }
    
    label->runAction(Sequence::create(MoveBy::create(0.5f,Vec2(0,32)),
                                      RemoveSelf::create(),
                                      callfunc,NULL));
    
    _messageLayer->addChild(label);
     */
}

void Stage::effectShield(Fune *fune)
{
    auto particle=ParticleSystemQuad::create("particle/shieldEffect.plist");
    particle->setPosition(fune->getPosition());
    _effectLayer->addChild(particle);
}

void Stage::effectWheel(const Vec2 &position,bool direction)
{
    auto sprite=Sprite::create("images/settings.png");
    sprite->setPosition(this->convertToStageSpace(position));
    
    int d;
    if(direction){
        d=1;
    }else{
        d=-1;
    }
    
    sprite->runAction(Sequence::create(
                                       Spawn::create(RotateBy::create(1.0,d*360),
                                                     CallFunc::create([]{
        CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sound/se4.wav");
    }),
                                                     NULL),
                                       RemoveSelf::create(),
                                       NULL));
    
    _effectLayer->addChild(sprite);
}
//end/エフェクト関係




////探索関係
void Stage::setAStarMap(const Fune *activeFune,const Vector<Fune *>& activePlayerFuneList)
{
    Map<int,MPoint*> map;//作成するマップ
    map=AStar::copyMap(_orisinalAStarMap);//基本のマップをコピー
    
    std::vector<Vec2> diffVec;
    diffVec.push_back(Vec2(0,-1));//上
    diffVec.push_back(Vec2(1,-1));//右上
    diffVec.push_back(Vec2(1,0));//右
    diffVec.push_back(Vec2(1,1));//右下
    diffVec.push_back(Vec2(0,1));//下
    diffVec.push_back(Vec2(-1,1));//左下
    diffVec.push_back(Vec2(-1,0));//左
    diffVec.push_back(Vec2(-1,-1));//左上
    //zocエリアの作成 船の周囲8マスに作成 zocエリアからは出られない
    for(auto& fune : _funeList){
        if(activePlayerFuneList.contains(fune)){continue;}//自軍側の船は無視する
        for(int i=0;i<diffVec.size();i++){
            AStar::setOutCost(map,fune->getTiledMapPosition()+diffVec.at(i),AStar::getMaxCost());
        }
    }
    _aStar->setMap(map);
}
//end/探索関係