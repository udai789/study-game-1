//
//  AStar.cpp
//  pirateTaxtics
//
//  Created by kk on 2015/05/20.
//
//

#include "AStar.h"

USING_NS_CC;

////MPoint
MPoint::MPoint()
:_flag(false)
{
    
}

MPoint::~MPoint()
{
    
}

MPoint* MPoint::create(const Vec2& position,const int &inCost,const int &outCost)
{
    MPoint* ret=new MPoint();
    if(ret->init(position,inCost,outCost)){
        ret->autorelease();
        return ret;
    }
    
    CC_SAFE_RELEASE_NULL(ret);
    return nullptr;
}

bool MPoint::init(const Vec2 &position,const int &inCost,const int &outCost)
{
    if(inCost<1 || outCost<0){return false;}//inCostが1未満もしくはoutCostが負の値のときfalse
    _position=position;
    _inCost=inCost;
    _outCost=outCost;
    return true;
}
//end/MPoint

////PointCost
AStar::PointCost::PointCost()
:_mPoint(nullptr)
{
    
}

AStar::PointCost::~PointCost()
{
    CC_SAFE_RELEASE_NULL(_mPoint);
}

AStar::PointCost* AStar::PointCost::create(MPoint* mPoint,const int &realCost,const int& estimateCost)
{
    PointCost* ret=new PointCost();
    if(ret->init(mPoint,realCost,estimateCost)){
        ret->autorelease();
        return ret;
    }
    
    CC_SAFE_RELEASE_NULL(ret);
    return nullptr;
}

bool AStar::PointCost::init(MPoint* mPoint,const int &realCost,const int& estimateCost)
{
    if(!mPoint){return false;}
    this->setMPoint(mPoint);
    _realCost=realCost;
    _estimateCost=estimateCost;
    return true;
}

int AStar::PointCost::getTotalEstimateCost()
{
    return _realCost+_estimateCost;
}
//end/PointCost

////AStar
AStar::AStar()
:_minInCost(AStar::getMaxCost())
,_minOutCost(AStar::getMaxCost())
,_minCost(AStar::getMaxCost())
,_hasCost(0)
,_endPosition(Vec2(0,0))
{
    
}

AStar::~AStar()
{
    
}

AStar* AStar::create()
{
    AStar* ret=new AStar();
    if(ret->init()){
        ret->autorelease();
        return ret;
    }
    
    CC_SAFE_RELEASE_NULL(ret);
    return nullptr;
}

bool AStar::init()
{
    return true;
}

bool AStar::checkSafePosition(const Vec2 &position)
{
    auto x=std::abs(std::floor(position.x));
    auto y=std::abs(std::floor(position.y));
    
    if(x==position.x && y==position.y){//x,y共に0または正の整数ならば
        if(x<AStar::getMaxSize() && y<AStar::getMaxSize()){
            return true;
        }
    }
    
    return false;
}

bool AStar::checkExistMPoint(const Vec2 &position)
{
    if(this->checkSafePosition(position)){//座標が正規のものか
        if(_mPoints.at(AStar::createKey(position))){//その座標の点が存在するか
            return true;
        }
    }
    return false;
}

int AStar::createKey(const Vec2 &position)
{
    return static_cast<int>(position.x+position.y*AStar::getMaxSize());
}

int AStar::getNormalDistance(const Vec2 &position1,const Vec2 &position2)
{
    auto v=position1-position2;
    auto distance=std::abs(v.x)+std::abs(v.y);
    
    return static_cast<int>(distance*(_minInCost+_minOutCost));
}

int AStar::getRealDistance(const Vec2 &startPosition,const Vec2 &nextPosition)
{
    auto outCost=_mPoints.at(AStar::createKey(startPosition))->getOutCost();//現在の点から出るコスト
    auto inCost=_mPoints.at(AStar::createKey(nextPosition))->getInCost();//次の点に入るコスト
    
    return outCost+inCost;
}

void AStar::setAllMPointFlagFalse()
{
    for(auto& key : _mPoints.keys()){
        _mPoints.at(key)->setFlag(false);
    }
}

void AStar::addPointCostVector(Vector<PointCost *> &pointCostVector,AStar::PointCost *pointCost)
{
    if(pointCost->getTotalEstimateCost()>_hasCost){//コスト以内に移動できないため格納しない
        return;
    }
    
    if(pointCostVector.empty()){//要素が無ければ格納して終了
        pointCostVector.pushBack(pointCost);
        return;
    }
    
    ////入れ替え
    auto count=pointCostVector.size();//現在の要素数
    
    auto index=count;//要素を挿入する位置
    for(int i=0;i<count;i++){
        auto pCost=pointCostVector.at(i);
        if(pointCost->getTotalEstimateCost() < pCost->getTotalEstimateCost()){//予想される総コストが下回れば
            index=i;
            break;
        }else if(pointCost->getTotalEstimateCost() == pCost->getTotalEstimateCost()){//予想される総コストが同じならば
            if(pointCost->getRealCost() < pCost->getRealCost()){//現在までの実コストが下回れば
                index=i;
                break;
            }
        }
    }
    
    auto popCount=count-index;//後ろから挿入位置までの要素数
    Vector<PointCost*> box;//PointCostの退避先
    while(popCount){
        box.pushBack(pointCostVector.back());//一番後ろの要素を入れる
        pointCostVector.popBack();//一番後ろの要素を取り出す
        popCount--;
    }
    
    pointCostVector.pushBack(pointCost);//要素を挿入
    
    auto pushCount=box.size();//退避させた要素数
    while(pushCount){
        pointCostVector.pushBack(box.back());//退避させた要素を戻す
        box.popBack();
        pushCount--;
    }
    //end/入れ替え
}

std::vector<Vec2> AStar::searchN(AStar::PointCost *pointCost)
{
    pointCost->getMPoint()->setFlag(true);//探索済みに設定
    std::vector<Vec2> result;//経路
    auto position=pointCost->getMPoint()->getPosition();//現在の座標
    if(pointCost->getTotalEstimateCost() >= _minCost){//予想される最小コストが最小コストを超えている
        pointCost->getMPoint()->setFlag(false);//この経路の探索は終了したので解除
        return std::move(result);
    }
    if(position==_endPosition){//終点に着いた
        pointCost->getMPoint()->setFlag(false);//この経路の探索は終了したので解除
        _minCost=pointCost->getRealCost();//最小コストを更新
        result.push_back(position);
        return std::move(result);
    }
    
    std::vector<Vec2> nextPosition;//上下左右の座標
    auto up=position+Vec2(0,-1);//開始位置から上の座標
    auto down=position+Vec2(0,1);//開始位置から下の座標
    auto left=position+Vec2(-1,0);//開始位置から左の座標
    auto right=position+Vec2(1,0);//開始位置から右の座標
    if(this->checkExistMPoint(up)){//点の存在する座標か
        nextPosition.push_back(up);
    }
    if(this->checkExistMPoint(down)){
        nextPosition.push_back(down);
    }
    if(this->checkExistMPoint(left)){
        nextPosition.push_back(left);
    }
    if(this->checkExistMPoint(right)){
        nextPosition.push_back(right);
    }
    
    Vector<PointCost*> pointCostVector;//現在の座標に隣接する座標のPointCost
    //最短経路が存在する可能性のある座標のPointCostを作成
    for(int i=0;i<nextPosition.size();i++){
        auto next=nextPosition.at(i);
        this->addPointCostVector(pointCostVector,
                                 PointCost::create(_mPoints.at(AStar::createKey(next)),
                                                   pointCost->getRealCost()+this->getRealDistance(position,next),
                                                   this->getNormalDistance(next,_endPosition)
                                                   ));
    }
    
    for(int i=0;i<pointCostVector.size();i++){//予想されるトータルコストが低いほうから探索
        std::vector<Vec2> box;
        box=searchN(pointCostVector.at(i));
        if(!box.empty()){//vectorの中身があれば現時点で最短経路
            result=std::move(box);
        }
    }
    if(!result.empty()){//経路が返されていれば
        result.push_back(position);
    }
    
    pointCost->getMPoint()->setFlag(false);//この経路の探索は終了したので解除
    return std::move(result);
}

bool AStar::searchNCheckLine(AStar::PointCost *pointCost)
{
    pointCost->getMPoint()->setFlag(true);//探索済みに設定
    auto position=pointCost->getMPoint()->getPosition();//現在の座標
    if(position==_endPosition){//終点に着いた
        return true;
    }
    
    std::vector<Vec2> nextPosition;//上下左右の座標
    auto up=position+Vec2(0,-1);//開始位置から上の座標
    auto down=position+Vec2(0,1);//開始位置から下の座標
    auto left=position+Vec2(-1,0);//開始位置から左の座標
    auto right=position+Vec2(1,0);//開始位置から右の座標
    if(this->checkExistMPoint(up)){//点の存在する座標か
        nextPosition.push_back(up);
    }
    if(this->checkExistMPoint(down)){
        nextPosition.push_back(down);
    }
    if(this->checkExistMPoint(left)){
        nextPosition.push_back(left);
    }
    if(this->checkExistMPoint(right)){
        nextPosition.push_back(right);
    }
    
    Vector<PointCost*> pointCostVector;//現在の座標に隣接する座標のPointCost
    //最短経路が存在する可能性のある座標のPointCostを作成
    for(int i=0;i<nextPosition.size();i++){
        auto next=nextPosition.at(i);
        this->addPointCostVector(pointCostVector,
                                 PointCost::create(_mPoints.at(AStar::createKey(next)),
                                                   pointCost->getRealCost()+this->getRealDistance(position,next),
                                                   this->getNormalDistance(next,_endPosition)
                                                   ));
    }
    
    for(int i=0;i<pointCostVector.size();i++){//予想されるトータルコストが低いほうから探索
        if(searchNCheckLine(pointCostVector.at(i))){//経路が見つかれば終了
            return true;
        }
    }
    
    pointCost->getMPoint()->setFlag(false);//この経路の探索は終了したので解除
    return false;//見つからなかったためfalse
}

bool AStar::createMPoint(Map<int,MPoint*>& map,const Vec2 &position,const int &inCost,const int &outCost)
{
    if(AStar::checkSafePosition(position)){//座標に問題が無ければ
        auto point=MPoint::create(position,inCost,outCost);
        
        if(point){//点の作成ができたら
            auto key=AStar::createKey(position);
            if(map.at(key)){map.erase(key);}//既に点がある場合削除する
            map.insert(key,point);
            return true;
        }
    }
    
    return false;
}

bool AStar::setOutCost(Map<int, MPoint *> &map,const Vec2 &position,const int &outCost)
{
    if(outCost>=0){
        if(AStar::checkSafePosition(position)){
            auto point=map.at(AStar::createKey(position));
            if(point){
                point->setOutCost(outCost);
                return true;
            }
        }
    }
    return false;
}

bool AStar::setInCost(Map<int, MPoint *> &map,const Vec2 &position,const int &inCost)
{
    if(inCost>=1){
        if(AStar::checkSafePosition(position)){
            auto point=map.at(AStar::createKey(position));
            if(point){
                point->setInCost(inCost);
                return true;
            }
        }
    }
    return false;
}

Map<int,MPoint*> AStar::copyMap(const Map<int, MPoint*> &map)
{
    Map<int,MPoint*> cMap;
    
    for(auto& key : map.keys()){
        auto point=map.at(key);
        AStar::createMPoint(cMap,point->getPosition(),point->getInCost(),point->getOutCost());
    }
    
    return std::move(cMap);
}

void AStar::setMap(Map<int, MPoint *> mPoints)
{
    _mPoints=std::move(mPoints);
    _minInCost=AStar::getMaxCost();
    _minOutCost=AStar::getMaxCost();
    for(auto& key : _mPoints.keys()){
        auto mPoint=_mPoints.at(key);
        if(mPoint->getInCost()<_minInCost){
            _minInCost=mPoint->getInCost();
        }
        if(mPoint->getOutCost()<_minOutCost){
            _minOutCost=mPoint->getOutCost();
        }
    }
}

std::vector<Vec2> AStar::search(const Vec2 &startPosition,const Vec2 &endPosition,const int &hasCost)
{
    _minCost=AStar::getMaxCost();//探査開始時に最大コストを設定
    _hasCost=hasCost;//使用可能なコストを設定
    _endPosition=endPosition;//目標地点
    this->setAllMPointFlagFalse();//全ての探査フラグをfalseに
    
    std::vector<Vec2> result;//最短経路
    //座標に問題があれば終了
    if(!this->checkExistMPoint(startPosition)){return std::move(result);}
    if(!this->checkExistMPoint(endPosition)){return std::move(result);}
    
    //最小コストが使用可能なコストを上回っている場合は終了
    auto distance=this->getNormalDistance(startPosition,endPosition);
    if(distance>hasCost){return std::move(result);}
    
    //startPositionから探査開始
    result=this->searchN(PointCost::create(_mPoints.at(AStar::createKey(startPosition)),0,distance));
    std::reverse(std::begin(result),std::end(result));//終点から開始点の順なので逆転する
    return std::move(result);
}

bool AStar::checkLine(const Vec2 &startPosition,const Vec2 &endPosition,const int &hasCost)
{
    _minCost=AStar::getMaxCost();//探査開始時に最大コストを設定
    _hasCost=hasCost;//使用可能なコストを設定
    _endPosition=endPosition;//目標地点
    this->setAllMPointFlagFalse();//全ての探査フラグをfalseに
    
    //座標に問題があれば終了
    if(!this->checkExistMPoint(startPosition)){return false;}
    if(!this->checkExistMPoint(endPosition)){return false;}
    
    //最小コストが使用可能なコストを上回っている場合は終了
    auto distance=this->getNormalDistance(startPosition,endPosition);
    if(distance>hasCost){return false;}
    
    //startPositionから探査開始
    return searchNCheckLine(PointCost::create(_mPoints.at(AStar::createKey(startPosition)),0,distance));
}
//end/Astar