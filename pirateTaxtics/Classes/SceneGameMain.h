//
//  SceneGameMain.h
//  pirateTaxtics
//
//  Created by kk on 2015/05/04.
//
//

#ifndef __pirateTaxtics__SceneGameMain__
#define __pirateTaxtics__SceneGameMain__

#include <stdio.h>
#include "cocos2d.h"
#include "Stage.h"
#include "Player.h"

class GameInitialise:public cocos2d::Ref
{
protected:
    GameInitialise();
    virtual ~GameInitialise();
    bool init(int stageNumber,bool isPlayer2CPU,bool isVersus);
    
public:
    /*ゲームの初期化設定
     *@param stageNumber ステージ番号　現在0のみ
     *@param isPlayer2CPU player2がCPUか true:CPU
     *@param isVersus 対戦モードか true:対戦モード false:ストーリーモード
     */
    static GameInitialise* create(int stageNumber,bool isPlayer2CPU,bool isVersus);
    
    CC_SYNTHESIZE_READONLY(int,_stageNumber,StageNumber);//ステージの番号 現状0のみ対応
    CC_SYNTHESIZE_READONLY(bool,_isPlayer2CPU,IsPlayer2CPU);//plyaer2がCPUか true:CPU
    CC_SYNTHESIZE_READONLY(bool,_isVersus,IsVersus);//対戦モードか true:対戦モード false:ストーリーモード
};

class SceneGameMain:public cocos2d::Layer
{
protected:
    enum class GameState{
        GAMESTART,//ゲーム開始
        GAMEEND,//ゲーム終了
        TURNSTART,//ターン開始時
        TURNUPDATE,//ターン中
        TURNEND,//ターン終了時
        RESULT,//対戦終了時
        ACTION,//移動、攻撃実行時
        INPUTWAITING,//ユーザー操作待ち
        HELP//ヘルプなど表示時
    };
    
    enum class LayerZPositions{
        NORMAL,//標準
        STAGE,//ステージ
        LABEL,//ラベル
        HELP,//ヘルプ画面
        RESULT,//リザルト
        SETTING//設定画面
    };
    
    SceneGameMain();//コンストラクタ
    virtual ~SceneGameMain();//デストラクタ
    //bool initWithLevel(int level);//インスタンス作成時に実行する関数
    bool initWithGameInitialise(GameInitialise* gameInitialise);
    
    
    ////stage関係
    /*マップを作成する
     */
    void createMap();
    //end/stage関係
    
    
    
    
    ////player関係
    void createPlayers();//参加するプレイヤーを作成する
    
    /*プレイヤーを追加する
     *@param player 追加するプレイヤー
     */
    void addPlayerList(Player* player);
    
    int getPlayerListCount();//現在のプレイヤーの数
    
    /*指定した位置のプレイヤーを返す
     *@param index プレイヤーリストの位置
     *@return その位置のプレイヤー
     */
    Player* getPlayer(int index);
    
    /*指定したプレイヤーのリストの位置を返す
     *@param player 調べるプレイヤー
     *@return プレイヤーリストの位置
     */
    int getPlayerIndex(Player* player);
    
    int getActivePlayerIndex();//現在のターンに行動できるプレイヤーの_playerListの位置を返す
    
    /*現在のターンに行動できるプレイヤーを返す
     *@return 行動できるプレイヤー
     */
    Player* getActivePlayer();
    //end/player関係
    
    
    
    ////fune関係
    /*新しく船を作成する
     *@param player 船が所属するプレイヤー
     *@param characterType 船のタイプ
     *@param position 初期位置　タイルマップ上の座標
     */
    void createFune(Player* player,Fune::CharacterTypes characterType,const cocos2d::Vec2& position);
    //end/fune関係
    
    
    
    ////UI関係
    void createLabelLayer();//ラベルを表示するレイヤーを作成
    void createTurnLabel();//ターンを表示するラベルを作成
    void createPlayerLabel();//現在のプレイヤーを表示するラベルを作成
    void createSettingButton();//ゲームの設定を呼び出すボタンを作成
    void createSettingWindow();//設定画面を作成する
    /*船のヘルプを表示
     *@param ヘルプを表示する船
     */
    void createHelpLayer(Fune* fune);
    void updateTurnLabel();//現在の_turnCounterの値に更新する
    void updatePlayerLabel();//現在のプレイヤーを表示する
    void updateUI();//ラベルなどの表示を更新する
    //end/UI関係
    
    
    
    //ターン処理関係
    void beginGame();//ゲーム開始時の処理
    bool endGame();//ゲーム終了時の処理 @return true:ゲーム終了 false:続行
    void startTurn();//ターン開始時の処理
    void startTurnUI(cocos2d::CallFunc* callfunc);//ターン開始時のUI表示
    void updateTurn();//ターン中の処理
    void endTurn();//ターン終了時の処理
    //end/ターン処理関係
    
    
    
    /*船の移動処理を行う
     *@param fune 移動する船
     *@param position 移動するタイルマップ上の座標
     */
    void moveProcess(Fune* fune,const cocos2d::Vec2& position);
    
    /*攻撃を行う
     *@param attackFune 攻撃を行う船
     *@param defenseFune 攻撃を受ける船
     */
    void normalAttack(Fune* attackFune,Fune* defenseFune);
    
    
    /*ダメージ計算、結果の反映(hpへの適用など) 表示用のstringを返す
     *@param attackFune 攻撃を行う船
     *@param defenseFune 攻撃を受ける船
     *@return 攻撃結果の文字列
     */
    std::string damageProcess(const Fune* attackFune,Fune* defenseFune);
    
    /*攻撃を受けるアニメーション処理
     *@param targetFune 対象の船
     *@param string 表示する文字列
     *@param callFunc アニメーション終了時の処理
     */
    void damageAnimationProcess(Fune* targetFune,const std::string& string,cocos2d::CallFunc* callFunc);
    
    
    
    ////スキル関係
    /*スキル　自軍のユニットのHPを最大値の半分回復する
     *@param fune 使用した船
     */
    void skillHeal(Fune* fune);
    
    /*スキル　自船への攻撃を一度だけ防ぐ
     *@param fune 使用した船
     */
    void skillIronShield(Fune* fune);
    
    /*スキル　攻撃範囲内の全ての敵を攻撃する
     *@param fune 使用した船
     */
    void skillBulletStorm(Fune* fune);
    
    /*スキル　現ターン中2回行動できるようにする
     *@param fune 使用した船
     */
    void skillHurryUp(Fune* fune);
    
    /*デバッグ用 他プレイヤーの船を消し対戦を終了させる
     *@param fune 使用した船
     */
    void skillDebug(Fune* fune);
    //end/スキル関係
    
    
    /*HPが0以下になった船があれば消去する
     *@return true:船を消去した
     */
    bool deleteFune();
    
    void simulatePlay();//CPU操作
    
    /*指定した範囲の整数をランダムに生成する
     *@param min 最小値
     *@param max 最大値
     *@return 生成された値x min<=x<=max
     */
    int generateRandomInt(int min,int max);
    
    /*指定した範囲の実数をランダムに生成する [min,max] max値も範囲に入る
     *@param min 最小値
     *@param max 最大値
     *@return 生成された値x min<=x<=max
     */
    float generateRandomFloat(float min,float max);
    
public:
    /*任意の番号のステージでシーンを作成します
     *@param level ステージ番号
     *@return シーン
     */
    //static cocos2d::Scene* createSceneWithLevel(int level);
    static cocos2d::Scene* createSceneWithGameInitialise(GameInitialise* gameInitialise);
    void onEnterTransitionDidFinish() override;//トランジション終了時に実行される
    
    CC_SYNTHESIZE(std::mt19937,_engine,Engine);//乱数発生器
    CC_SYNTHESIZE(cocos2d::Vector<Player*>,_playerList,PlayerList);//プレイヤーを保持するベクター
    CC_SYNTHESIZE(int,_turnCounter,turnCounter);//現在のターン数を保持
    CC_SYNTHESIZE(GameState,_gameState,GameState);//現在のゲームの状態
    CC_SYNTHESIZE(int,_turnActionPlusCount,TurnActionPlusCount);//そのターン行動できる残り回数
    CC_SYNTHESIZE_RETAIN(GameInitialise*,_gameInitialise,GameInitialise);//ゲームの初期設定
    CC_SYNTHESIZE_RETAIN(Stage*,_stage,Stage);//ステージクラスを保持するメンバ
    CC_SYNTHESIZE_RETAIN(cocos2d::Layer*,_labelLayer,LabelLayer);//ラベルを置くレイヤー
    CC_SYNTHESIZE_RETAIN(cocos2d::Label*,_turnLabel,TurnLabel);//ターンを表示するラベル
    CC_SYNTHESIZE_RETAIN(cocos2d::Label*,_playerLabel,PlayerLabel);;//現在のプレイヤーを表示するラベル
    
    CC_SYNTHESIZE(Fune*,_cc,CC);
};

#endif /* defined(__pirateTaxtics__SceneGameMain__) */
