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
        SETTING,//設定画面
        RESULT//リザルト
    };
    
    SceneGameMain();//コンストラクタ
    virtual ~SceneGameMain();//デストラクタ
    //bool init() override;//インスタンス作成時に実行する関数
    bool initWithLevel(int level);//インスタンス作成時に実行する関数
    
    /*マップを作成する
     *@param level ステージ番号
     */
    void createMap(int level);
    
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
    
    /*新しく船を作成する
     *@param player 船が所属するプレイヤー
     *@param characterType 船のタイプ
     */
    void createFune(Player* player,Fune::CharacterTypes characterType);
    
    void createLabelLayer();//ラベルを表示するレイヤーを作成
    void createTurnLabel();//ターンを表示するラベルを作成
    void createPlayerLabel();//現在のプレイヤーを表示するラベルを作成
    void createSettingButton();//ゲームの設定を呼び出すボタンを作成
    void createPlayers();//参加するプレイヤーを作成する
    
    void updateTurnLabel();//現在の_turnCounterの値に更新する
    void updatePlayerLabel();//現在のプレイヤーを表示する
    void updateUi();//ラベルなどの表示を更新する
    
    void beginGame();//ゲーム開始時の処理
    bool endGame();//ゲーム終了時の処理 @return true:ゲーム終了 false:続行
    void startTurn();//ターン開始時の処理
    void startTurnUI(cocos2d::CallFunc* callfunc);//ターン開始時のUI表示
    void updateTurn();//ターン中の処理
    void endTurn();//ターン終了時の処理
    
    /*攻撃を行う
     *@param attackFune 攻撃を行う船
     *@param defenseFune 攻撃を受ける船
     */
    void attack(Fune* attackFune,Fune* defenseFune);
    
    void createSettingWindow();//設定画面を作成する
    
public:
    //static cocos2d::Scene* createScene();//このクラスのインスタンスを持つsceneを返す
    /*任意の番号のステージでシーンを作成します
     *@param level ステージ番号
     *@return シーン
     */
    static cocos2d::Scene* createSceneWithLevel(int level);
    void onEnterTransitionDidFinish() override;//トランジション終了時に実行される
    
    CC_SYNTHESIZE(cocos2d::Vector<Player*>,_playerList,PlayerList);//プレイヤーを保持するベクター
    CC_SYNTHESIZE(int,_turnCounter,turnCounter);//現在のターン数を保持
    CC_SYNTHESIZE(GameState,_gameState,GameState);//現在のゲームの状態
    CC_SYNTHESIZE_RETAIN(Stage*,_stage,Stage);//ステージクラスを保持するメンバ
    CC_SYNTHESIZE_RETAIN(cocos2d::Layer*,_labelLayer,LabelLayer);//ラベルを置くレイヤー
    CC_SYNTHESIZE_RETAIN(cocos2d::Label*,_turnLabel,TurnLabel);//ターンを表示するラベル
    CC_SYNTHESIZE_RETAIN(cocos2d::Label*,_playerLabel,PlayerLabel);;//現在のプレイヤーを表示するラベル
    //CREATE_FUNC(SceneGameMain);//インスタンスの生成 クラス名::create()
};

#endif /* defined(__pirateTaxtics__SceneGameMain__) */
