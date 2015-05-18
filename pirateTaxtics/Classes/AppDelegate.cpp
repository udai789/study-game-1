#include "AppDelegate.h"
//#include "HelloWorldScene.h"
#include "SceneGameMain.h"

USING_NS_CC;

//初期ステージ番号
const int INITIAL_LEVEL=0;

AppDelegate::AppDelegate() {

}

AppDelegate::~AppDelegate() 
{
}

//if you want a different context,just modify the value of glContextAttrs
//it will takes effect on all platforms
void AppDelegate::initGLContextAttrs()
{
    //set OpenGL context attributions,now can only set six attributions:
    //red,green,blue,alpha,depth,stencil
    GLContextAttrs glContextAttrs = {8, 8, 8, 8, 24, 8};

    GLView::setGLContextAttrs(glContextAttrs);
}

bool AppDelegate::applicationDidFinishLaunching() {
    // initialize director
    auto director = Director::getInstance();
    auto glview = director->getOpenGLView();
    if(!glview) {
        glview = GLViewImpl::create("My Game");
        director->setOpenGLView(glview);
    }
    
    //すべての端末で画面サイズを960*640に設定
    glview->setDesignResolutionSize(960,640,ResolutionPolicy::SHOW_ALL);
    /* スケール変更
    //現在の端末の画面サイズの取得
    auto frameSize=glview->getFrameSize();
    //DesignResolutionSizeの取得
    auto designResolutionSize=glview->getDesignResolutionSize();
    //ContentScaleFactorの設定 素材のサイズがContentScaleFactor分の1になる？
    director->setContentScaleFactor(frameSize.height/designResolutionSize.height);
    */
    
    FileUtils::getInstance()->addSearchPath("cocostudio");//cocostudioで作ったフォルダを指定

    // turn on display FPS
    director->setDisplayStats(false);

    // set FPS. the default value is 1.0/60 if you don't call this
    director->setAnimationInterval(1.0 / 60);

    // create a scene. it's an autorelease object
    //auto scene = HelloWorld::createScene();
    auto scene=SceneGameMain::createSceneWithLevel(INITIAL_LEVEL);

    // run
    director->runWithScene(scene);

    return true;
}

// This function will be called when the app is inactive. When comes a phone call,it's be invoked too
void AppDelegate::applicationDidEnterBackground() {
    Director::getInstance()->stopAnimation();

    // if you use SimpleAudioEngine, it must be pause
    // SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
}

// this function will be called when the app is active again
void AppDelegate::applicationWillEnterForeground() {
    Director::getInstance()->startAnimation();

    // if you use SimpleAudioEngine, it must resume here
    // SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
}
