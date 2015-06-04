//
//  Stage.h
//  pirateTaxtics
//
//  Created by kk on 2015/05/05.
//
//

#ifndef __pirateTaxtics__Stage__
#define __pirateTaxtics__Stage__

#include <stdio.h>
#include "cocos2d.h"
#include "Fune.h"
#include "AStar.h"

class Stage :public cocos2d::Layer
{
protected:
    Stage();//コンストラクタ
    virtual ~Stage();//デストラクタ
    bool initWithLevel(int level);
    
    ////初期化処理
    //フレーム、マップ、タイルマップを作成しStageに追加
    void createMap();
    //マーカーレイヤーの作成
    void createMarkerLayer();
    //ユニットレイヤーの作成
    void createUnitLayer();
    void createEffectLayer();//エフェクトレイヤーを作成
    void createMessageLayer();//メッセージレイヤーを作成
    //探索マップを作成
    void createAStar();
    //end/初期化処理
    
public:
    //レイヤーの重なり tagにも使用
    enum class LayerZPositions{
        NORMAL,//0 標準
        UNIT,//1 船を置くレイヤー
        AREARANGE,//2 移動範囲を表示するレイヤー
        MARKER,//3 マーカーを表示するレイヤー
        EFFECT,//4 エフェクトを表示するレイヤー
        MESSAGE//5 メッセージを表示するレイヤー
    };
    
    //タイルの種類 カスタムプロパティのdisplayDataの値に対応
    enum class TileTypes{
        UMI,//displayData=0 海
        ARAI,//=1 荒い海
        ASAI,//=2 浅瀬
        RIKU,//=3 陸
        IWA,//=4 岩
        NONE//定義されていない場合
    };
    
    //マップマーカーの種類
    enum class MapMarkerTypes{
        REDCROSS,//赤い×マーク 変更不可 createMarkerLayerの変更が必要
        GRAYCROSS,//灰色の×マーク 変更不可 createMarkerLayerの変更が必要
        BLUECOVER,//青いカバー
        REDCOVER,//赤いカバー
        COUNT//マーカーの数
    };
    
    cocos2d::Map<int,MPoint*> _orisinalAStarMap;//_tiledMapの情報のみで作成した探索用マップ 状況に合わせたマップはこれをコピーして作成
    
    //マップの番号
    CC_SYNTHESIZE_READONLY(int,_level,Level);
    
    //船を保持するベクター
    CC_SYNTHESIZE(cocos2d::Vector<Fune*>,_funeList,FuneList);
    
    //マップを保持するメンバ
    CC_SYNTHESIZE_RETAIN(cocos2d::TMXTiledMap*,_tiledMap,TiledMap);
    //マーカーレイヤー
    CC_SYNTHESIZE_RETAIN(cocos2d::Layer*,_makerLayer,MakerLayer);
    //船を配置するレイヤー
    CC_SYNTHESIZE_RETAIN(cocos2d::Layer*,_unitLayer,UnitLayer);
    CC_SYNTHESIZE_RETAIN(cocos2d::Layer*,_effectLayer,EffectLayer);//エフェクトを表示するレイヤー
    CC_SYNTHESIZE_RETAIN(cocos2d::Layer*,_messageLayer,MessageLayer);//メッセージを表示するレイヤー
    CC_SYNTHESIZE_RETAIN(AStar*,_aStar,AStar);//探査アルゴリズム
    
    /*ステージ番号からステージを生成します
     *@param level ステージ番号
     *@return ステージ
     */
    static Stage* createWithLevel(int level);
    
    
    ////座標関係
    /*ステージ上の座標をタイルマップ上の座標に変換する
     *
     *@param stagePosition ステージ上の座標
     *@return タイルマップ上の座標
     */
    cocos2d::Vec2 convertToTiledMapSpace(const cocos2d::Vec2& stagePosition);
    
    /*タイルマップ上の座標をステージ上の座標に変換する 戻り値の座標はタイルの真ん中
     *@param position タイルマップ上の座標
     *@return ステージ上の座標
     */
    cocos2d::Vec2 convertToStageSpace(const cocos2d::Vec2& position);
    
    /*タイルマップ上の二つの座標からタイル間の距離を計算する
     *@param position1 position2 タイルの座標
     *@return タイル間の距離
     */
    int getDistance(const cocos2d::Vec2& position1,const cocos2d::Vec2& position2);
    
    /*タイルマップ上の座標を受け取り、その座標のTileTypesを返す
     *@param position タイルマップ上の座標
     *@return その座標のTileTypes
     */
    TileTypes getTileType(const cocos2d::Vec2& position);
    
    /*通行可能なタイルかどうか
     *@param position タイルマップ上の座標
     *@return true:通行可 false:通行不可
     */
    bool tileMoveCheck(const cocos2d::Vec2& position);
    
    /*ステージ上の座標がタイルマップ上にあるか
     *@param stagePosition ステージ上の座標
     *@return true:ステージ上の座標上にタイルマップがある
     */
    bool onTiledMapCheck(const cocos2d::Vec2& stagePosition);
    
    /*ステージ上のタイルにspriteを配置
     *@param sprite 配置するsprite
     *@param position タイルマップ上の座標
     */
    void positionObject(cocos2d::Sprite* sprite,const cocos2d::Vec2& position);
    //end/座標関係
    
    
    
    
    ////marker関係
    /*マーカー表示を消す
     */
    void markerHide();
    
    /*任意のマーカーを指定したタイルマップ座標に表示する
     *@param markerType 表示するマーカー
     *@param position タイルマップ上の座標
     */
    void markerShow(MapMarkerTypes markerType,const cocos2d::Vec2& position);
    
    /*船の位置とタッチされた位置から適したマーカーを表示する
     *@param fune 選択中の船
     *@param position タッチされたタイルの位置
     */
    void setMarker(Fune* fune,const cocos2d::Vec2& position);
    //end/marker関係
    
    
    
    
    ////areaRange関係
    /*移動範囲を表すレイヤーを作成する
     *@param fune 行動する船
     */
    void createAreaRangeLayer(Fune* fune);
    
    /*移動範囲を表すレイヤーを消去する
     */
    void removeAreaRangeLayer();
    //end/areaRange関係
    
    
    
    ////fune関係
    /*リストに船を登録
     *@param fune _funeListに登録する船
     */
    void addFuneList(Fune* fune);
    
    /*リストから船を削除
     *@param fune _funelistから削除する船
     */
    void removeFuneList(Fune* fune);
    
    /*ステージ上のタイルに船を配置
     *@param fune 配置する船
     *@param position タイルマップ上の座標
     */
    void positionFune(Fune* fune,const cocos2d::Vec2& position);
    
    /*ユニットレイヤーに船を置く
     *@param fune 配置する船
     */
    void setFuneLayer(Fune* fune);
    
    /*指定したタイルマップ上の座標に船があれば返す
     *@param position タイルマップ上の座標
     *@return 座標上の船 無ければnullptr
     */
    Fune* getOnTiledMapFune(cocos2d::Vec2& position);
    
    /*船をアニメーション付きで移動させる
     *@param fune 移動する船
     *@param position タイルマップ上の座標
     *@param callfunc アニメーション終了後の処理
     */
    void moveFuneAnimation(Fune* fune,const cocos2d::Vec2& position,cocos2d::CallFunc* callfunc);
    //end/fune関係
    
    
    
    
    //エフェクト関係
    /*指定した座標に爆発表現
     *@param position タイルマップ上の座標
     *@param callfunc アニメーション後に呼び出す処理
     */
    void effectExplosion(const cocos2d::Vec2& position,cocos2d::CallFunc* callfunc);
    
    /*ラベルを表示する
     *@param fune ラベルを表示する船
     *@param string 表示する文字
     *@param color 文字の色
     *@param callFunc アニメーション後に呼び出す処理
     */
    void effectLabel(Fune* fune,std::string string,cocos2d::Color3B color,cocos2d::CallFunc* callFunc);
    
    /*攻撃を受けた船が受けたダメージを表示
     *@param fune 攻撃を受けた船
     *@param string 表示する文字列
     *@param callfunc アニメーション後に呼び出す処理
     */
    void damageLabel(Fune* fune,std::string string,cocos2d::CallFunc* callfunc);
    
    /*盾を張るエフェクト
     *@param fune 対象の船
     */
    void effectShield(Fune* fune);
    
    /*舵を回すエフェクト
     *@param position エフェクトを表示する位置
     *@param direction 回転の方向 true:右回転 false:左回転
     */
    void effectWheel(const cocos2d::Vec2& position,bool direction);
    //end/エフェクト関係
    
    
    
    
    ////探索関係
    /*探索マップを作成し探索アルゴリズムにセットする
     *@param activeFune 選択中の船
     *@param activePlayerFuneList 行動中のプレイヤーの船リスト
     */
    void setAStarMap(const Fune* activeFune,const cocos2d::Vector<Fune*>& activePlayerFuneList);
    //end/探索関係
};
#endif /* defined(__pirateTaxtics__Stage__) */
