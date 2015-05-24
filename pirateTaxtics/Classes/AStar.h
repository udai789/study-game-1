//
//  AStar.h
//  pirateTaxtics
//
//  Created by kk on 2015/05/20.
//
//

/*移動は上下左右の1マスずつ行う
 *座標は左上を(0,0)として正の整数で表す
 *マップは横*縦の四角形
 *inCostは正の整数 outCostは0と正の整数
 *ASter::create(横の点の数,縦の点の数)で作成し
 *aster->addPoint(点の座標,入るコスト,出るコスト)で各点を作成する
 */

#ifndef __pirateTaxtics__AStar__
#define __pirateTaxtics__AStar__

#include <stdio.h>
#include <vector>
#include "cocos2d.h"


#endif /* defined(__pirateTaxtics__AStar__) */

//点を表すクラス
class MPoint :public cocos2d::Ref
{
protected:
    MPoint();
    virtual ~MPoint();
    bool init(const cocos2d::Vec2& position,const int& inCost,const int& outCost);
    
public:
    CC_SYNTHESIZE(cocos2d::Vec2,_position,Position);//点の座標
    CC_SYNTHESIZE(int,_inCost,InCost);//この点に入るためのコスト
    CC_SYNTHESIZE(int,_outCost,OutCost);//この点から出るためのコスト
    CC_SYNTHESIZE(bool,_flag,Flag);//現在の探査ルートで通過済み
    
    /*点を作成する
     *@param position 点の座標
     *@param inCost 点に入るコスト 正の整数(inCost>=1)
     *@param outCost 点から出るコスト 0か正の整数(outCost>=0)
     *@return 点
     */
    static MPoint* create(const cocos2d::Vec2& position,
                          const int& inCost,const int& outCost);
};

class AStar :public cocos2d::Ref
{
protected:
    
    
    //現在の点の座標,現在の点までの実際のコストを格納するクラス
    class PointCost :public cocos2d::Ref
    {
    protected:
        PointCost();
        virtual ~PointCost();
        bool init(MPoint* mPoint,const int& realCost,const int& estimateCost);
        
    public:
        CC_SYNTHESIZE_RETAIN(MPoint*,_mPoint,MPoint);//現在の座標の点
        CC_SYNTHESIZE_READONLY(int,_realCost,RealCost);//現在の座標までの実際のコスト
        CC_SYNTHESIZE_READONLY(int,_estimateCost,EstimateCost);//現在の座標から終点の座標までの予想コスト
        
        /*インスタンスを作成する
         *@param mPoint 現在の座標の点
         *@param realCost 現在の座標までの実際のコスト
         *@param estimateCost 現在の座標から終点の座標までの予想コスト
         *@return ポインタ
         */
        static PointCost* create(MPoint* mPoint,const int& realCost,const int& estimateCost);
        
        int getTotalEstimateCost();//現在の座標を経由した場合に予想される総コスト
    };
    
    cocos2d::Map<int,MPoint*> _mPoints;//点の集合,キー値はx座標+y座標*マップの横幅
    int _minInCost;//一番小さいinCost
    int _minOutCost;//一番小さいoutCost
    int _minCost;//終点までの一番小さいコスト
    int _hasCost;//移動に使用可能なコスト
    cocos2d::Vec2 _endPosition;//探索の目標座標
    
    AStar();
    virtual ~AStar();
    bool init();
    
    /*渡された座標がこのインスタンスで使用する条件を満たしているか
     *.x,.yの値が0または正の整数である。
     *@param position 他のクラスなどから受け取った座標
     *@return true:満たしている false:満たしていない
     */
    static bool checkSafePosition(const cocos2d::Vec2& position);
    
    /*渡された座標にMPointが存在するか
     *@param position 調べる座標
     *@return true:ある false:ない
     */
    bool checkExistMPoint(const cocos2d::Vec2& position);
    
    
    
    /*_minInCostを用いて2点間の最小距離(最小コスト)を求める
     *@param position1 座標1
     *@param position2 座標2
     *@return 2点間の最小距離(最小コスト)
     */
    int getNormalDistance(const cocos2d::Vec2& position1,const cocos2d::Vec2& position2);
    
    /*startPositionからnextPositionに移動するときに必要なコストを求める
     *@param startPosition 現在の座標
     *@param nextPosition 移動する座標
     *@return 移動する際の実際のコスト
     *startPositionとnextPositionは隣接していること,隣接しているかのチェック機能なし
     */
    int getRealDistance(const cocos2d::Vec2& startPosition,const cocos2d::Vec2& nextPosition);
    
    void setAllMPointFlagFalse();//保持する全てのMPointのflagをfalseにする
    
    /*totalEstimateCostが低い(同値ならばrealCostが低い)順になるようpointCostを格納する 
     *hasCostよりtotalEstimateCostが大きければ格納しない
     *@param pointCostVecotr pointCostを格納するvector
     *@param pointCost 格納するpointCost
     */
    void addPointCostVector(cocos2d::Vector<PointCost*>& pointCostVector,PointCost* pointCost);
    
    /*pointCostの座標の上下左右を探査 hasCost以内で辿り着けば経路のvectorを返す searchから呼び出す
     *@param pointCost 現在の位置を格納したPointCost
     *@return 終点に辿り着けば　座標を入れてvectorを返す
     */
    std::vector<cocos2d::Vec2> searchN(PointCost* pointCost);
    
    /*pointCostの座標の上下左右を探査 hasCost以内で辿り着けばtrueを返す checkLineから呼び出す
     *@param pointCost 現在の座標を格納したPointCost
     *@return 終点に辿り着けばtrue 着かなければfalse
     */
    bool searchNCheckLine(PointCost* pointCost);
    
public:
    /*探索マップを作成
     *@return 作成した探索マップ
     */
    static AStar* create();
    
    static int getMaxSize(){return 10000;};//縦横の最大値
    static int getMaxCost(){return AStar::getMaxSize()*AStar::getMaxSize();};//コストの最大値を返す
    
    /*座標から_mPointsのキーを作成する
     *@param position 点の座標
     *@return この座標のキー値
     */
    static int createKey(const cocos2d::Vec2& position);
    
    /*点を作成し渡されたmapに格納する 探査用のマップはこの関数で各点を作成する
     *@param map 点を格納するmap
     *@param position 点の座標 x,yは0か正の整数
     *@param inCost 点に入るコスト 正の整数(inCost>=1)
     *@param outCost 点から出るコスト 0か正の整数(outCost>=0)
     *@return 作成に成功したか true:成功 false:失敗
     */
    static bool createMPoint(cocos2d::Map<int,MPoint*>& map,const cocos2d::Vec2& position,
                      const int& inCost,const int& outCost);
    
    /*渡されたマップの指定した位置のoutCostに値を設定する 点が存在しなければfalseを返す
     *@param map 点を格納するマップ
     *@param position 点の座標 x,yは0か正の整数
     *@param outCost 点から出るコスト 0か正の整数(outCost>=0)
     *@return 設定できればtrue 点が見つからない場合false
     */
    static bool setOutCost(cocos2d::Map<int,MPoint*>& map,const cocos2d::Vec2& position,const int& outCost);
    
    /*渡されたマップの内容をコピーして返す MPointの各インスタンスの内容をコピー
     *@param map コピーするマップ
     *@return 複製されたマップ
     */
    static cocos2d::Map<int,MPoint*> copyMap(const cocos2d::Map<int,MPoint*>& map);
    
    /*マップをセットする
     *@param mPoints 点の集合
     */
    void setMap(cocos2d::Map<int,MPoint*> mPoints);
    
    
    /*最短経路を探す
     *@param startPosition 開始位置
     *@param endPosition 終了位置
     *@param hasCost 支払えるコスト
     *@return 移動した座標を入れたvector .front():startPosition .back():endPosition hasCost以内で移動できなければ要素なし
     */
    std::vector<cocos2d::Vec2> search(const cocos2d::Vec2& startPosition,const cocos2d::Vec2& endPosition,
                                      const int& hasCost);
    
    /*渡されたコスト以内で移動可能か
     *@param startPosition 始点
     *@param endPosition 終点
     *@param hasCost 支払えるコスト
     *@return true:移動可能 false:不可
     */
    bool checkLine(const cocos2d::Vec2& startPosition,const cocos2d::Vec2& endPosition,
                   const int& hasCost);
};
