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
public:
    enum class GameType{
        VERSUS,//対戦モード
        STORY1,//ストーリーモード
        STORY2,
        STORY3,
        NONE
    };
    /*ゲームの初期化設定
     *@param gameType ゲームモードの設定
     *@param isPlayer2CPU player2がCPUか true:CPU
     */
    static GameInitialise* create(GameType gameType,bool isPlayer2CPU);
    
    CC_SYNTHESIZE_READONLY(GameType,_gameType,GameType);//ゲームモード
    CC_SYNTHESIZE_READONLY(bool,_isPlayer2CPU,IsPlayer2CPU);//plyaer2がCPUか true:CPU
    
    //enumを文字列に変換
    //@param enum
    //@return 文字列
    static std::string convertString(GameType gameType);
    
    //文字列をenumに変換
    //@param 文字列
    //@return enum
    static GameInitialise::GameType convertGameType(std::string string);
    
protected:
    GameInitialise();
    virtual ~GameInitialise();
    bool init(GameType gameType,bool isPlayer2CPU);
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
    
    //渡された船の攻撃範囲内にいる船のvectorを返す
    //@param attackFune 攻撃する船
    //@return 攻撃範囲内にいる船のvector
    cocos2d::Vector<Fune*> getWithinRangeFuneList(Fune* attackFune);
    
    //渡された船の攻撃範囲内に船があるか
    //@param attackFune 攻撃する船
    //@return true:攻撃対象がいる
    bool checkWithinRangeFune(Fune* attackFune);
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
    
    
    
    
    /*HPが0以下になった船があれば消去する
     *@return true:船を消去した
     */
    bool deleteFune();
    
    void simulatePlay();//CPU操作
    
    
    
    ////条件式
    //現在のプレイヤーがバレットストームを使うか判定 条件を満たせば使用しtrueを返す
    //使用条件:スキル使用可能な船がある && その船がスキル使用可能な状態にある && その船の攻撃範囲内に2隻以上の敵船がある
    //@return true:スキルを使用した(endTurn()を実行) false:スキルを使用しなかった(何もしない)
    bool checkCommandBulletStorm();
    
    //現在のプレイヤーが通常攻撃を行うか判定　条件を満たせば実行しtrueを返す
    //実行条件:攻撃範囲内に敵がいる船がある 複数いる場合は攻撃前のHPが一番低い敵に攻撃力が一番高い味方が攻撃
    //@return true:通常攻撃を行った false:攻撃範囲内に敵がいなかった
    bool checkCommandNormalAttack();
    
    //現在のプレイヤーが攻撃を行うか判定 通常攻撃かブレットストームを行う場合がある ハリーアップを使用する場合も有る
    //実行条件:スキルを使用可能な船があり、使用可能な状態がある && その船の攻撃範囲内に2隻以上の敵がいる バレットストーム使用
    //実行条件:攻撃範囲内に敵がおり、バレットストームを使用しなかった 通常攻撃
    //@return true:攻撃を行った false:攻撃範囲内に敵がいなかった
    bool checkCommandAttack();
    
    //現在のプレイヤーが船を移動させる
    //実行条件:移動可能な船がある
    //移動内容:移動可能な船が複数ある場合はランダムに選択。より遠くへの移動ほど確率が高い
    //@return true:移動を行った false:移動できる船がなかった
    bool checkCommandMove();
    
    //プレイヤーの船にアイロンシールドが使用可能な船があれば実行
    //実行条件:アイロンシールドが使用可能な船があり、残り回数が残っている
    //@return true:アイロンシールドを使用した false:使用しなかった
    bool checkIronShield();
    
    //プレイヤーの船にセカンドウィンドが使用可能な船があり、残り回数があり、HPの減った味方がいれば実行
    //実行条件:セカンドウィンドが使用可能な船があり、残り回数があり、HPが60%未満の味方がいる
    //@return true:セカンドウィンドを使用した false:使用しなかった
    bool checkHeal();
    
    //プレイヤーの船にハリーアップが使用可能な状態な船がある
    //実行条件:ハリーアップが使用可能な船があり、残り回数が残っている
    //@return true:ハリーアップを使用した flase:使用しなかった
    bool checkHurryUp();
    //end/条件式
    
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
};

#endif /* defined(__pirateTaxtics__SceneGameMain__) */
