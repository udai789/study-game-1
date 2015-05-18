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

class Stage :public cocos2d::Layer
{
protected:
    Stage();//コンストラクタ
    virtual ~Stage();//デストラクタ
    //bool init() override;
    bool initWithLevel(int level);
    
    //フレーム、マップ、タイルマップを作成しStageに追加
    void createMap();
    //マーカーレイヤーの作成
    void createMarkerLayer();
    //ユニットレイヤーの作成
    void createUnitLayer();
    
public:
    //レイヤーの重なり
    enum class LayerZPositions{
        NORMAL,//0 標準
        UNIT,//1 船を置くレイヤー
        AREARANGE,//2 移動範囲を表示するレイヤー
        MARKER,//3 マーカーを表示するレイヤー
        HELP//4 ヘルプを表示するレイヤー
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
        REDCROSS,//赤い×マーク
        GRAYCROSS,//灰色の×マーク
        BLUECOVER,//青いカバー
        REDCOVER,//赤いカバー
        COUNT//マーカーの数
    };
    
    //マップの番号
    CC_SYNTHESIZE_READONLY(int,_level,Level);
    
    //船を保持するベクター
    CC_SYNTHESIZE(cocos2d::Vector<Fune*>,_funeList,FuneList);
    
    //マップを保持するメンバ
    CC_SYNTHESIZE_RETAIN(cocos2d::TMXTiledMap*,_tiledMap,TiledMap);
    //マーカーレイヤー
    CC_SYNTHESIZE_RETAIN(cocos2d::Layer*,_makerLayer,MakerLayer);
    //移動範囲を表すレイヤー
    CC_SYNTHESIZE_RETAIN(cocos2d::Layer*,_areaRangeLayer,AreaRangeLayer);
    //船を配置するレイヤー
    CC_SYNTHESIZE_RETAIN(cocos2d::Layer*,_unitLayer,UnitLayer);
    CC_SYNTHESIZE_RETAIN(cocos2d::Layer*,_helpLayer,HelpLayer);//ヘルプを表示するレイヤー
    //CREATE_FUNC(Stage);
    
    /*ステージ番号からステージを生成します
     *@param level ステージ番号
     *@return ステージ
     */
    static Stage* createWithLevel(int level);
    
    /*通行可能なタイルかどうか
     *@param position タイルマップ上の座標
     *@return true:通行可 false:通行不可
     */
    bool tileMoveCheck(const cocos2d::Vec2& position);
    
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
    
    /*タイルマップ上の座標を受け取り、その座標のTileTypesを返す
     *@param position タイルマップ上の座標
     *@return その座標のTileTypes
     */
    TileTypes getTileType(const cocos2d::Vec2& position);
    
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
    
    /*ステージ上のタイルに船を配置
     *@param fune 配置する船
     *@param position タイルマップ上の座標
     */
    void positionFune(Fune* fune,const cocos2d::Vec2& position);
    
    /*船をアニメーション付きで移動させる
     *@param fune 移動する船
     *@param position タイルマップ上の座標
     *@param callfunc アニメーション終了後の処理
     */
    void moveFuneAnimation(Fune* fune,const cocos2d::Vec2& position,cocos2d::CallFunc* callfunc);
    
    /*タイルマップ上の二つの座標からタイル間の距離を計算する
     *@param position1 position2 タイルの座標
     *@return タイル間の距離
     */
    int getDistance(const cocos2d::Vec2& position1,const cocos2d::Vec2& position2);
    
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
    
    /*移動範囲を表すレイヤーを作成する
     *@param fune 行動する船
     */
    void createAreaRangeLayer(Fune* fune);
    
    /*移動範囲を表すレイヤーを消去する
     */
    void removeAreaRangeLayer();
    
    /*リストに船を登録
     *@param fune _funeListに登録する船
     */
    void addFuneList(Fune* fune);
    
    /*リストから船を削除
     *@param fune _funelistから削除する船
     */
    void removeFuneList(Fune* fune);
    
    /*ユニットレイヤーに船を置く
     *@param fune 配置する船
     */
    void setFuneLayer(Fune* fune);
    
    /*指定したタイルマップ上の座標に船があれば返す
     *@param position タイルマップ上の座標
     *@return 座標上の船 無ければnullptr
     */
    Fune* getOnTiledMapFune(cocos2d::Vec2& position);
    
    /*船のヘルプを表示
     *@param ヘルプを表示する船
     */
    void createHelpLayer(Fune* fune);
    void removeHelpLayer();//ヘルプレイヤーを消す
    
    /*指定した座標に爆発表現
     *@param position タイルマップ上の座標
     *@param callfunc アニメーション後に呼び出す処理
     */
    void effectExplosion(const cocos2d::Vec2& position,cocos2d::CallFunc* callfunc);
    
    /*攻撃を受けた船が受けたダメージを表示
     *@param fune 攻撃を受けた船
     *@param string 表示する文字列
     *@param callfunc アニメーション後に呼び出す処理
     */
    void damageLabel(Fune* fune,std::string string,cocos2d::CallFunc* callfunc);
};
#endif /* defined(__pirateTaxtics__Stage__) */
