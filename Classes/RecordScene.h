#pragma once

#include "cocos2d.h"
#include "NativeDefs.h"
#include "AdjustScene.h"
#include "AppDelegate.h"

using namespace cocos2d;

class RecordScene : public Layer
{
public:
	static Scene* createScene()
	{
		auto scene = Scene::create();

		// create background, scale to height
		auto winsize = Director::getInstance()->getWinSize();
		auto bgimage = Sprite::create("BG.png");
		bgimage->setAnchorPoint(Vec2(0.0f, 0.0f));
		auto bgisz = bgimage->getContentSize();
		float xscale = winsize.width*2 / bgisz.width;
		float yscale = winsize.height / bgisz.height;
		if (xscale > yscale) bgimage->setScale(xscale);
		else bgimage->setScale(yscale);
		scene->addChild(bgimage,-1);

		auto layer = RecordScene::create();
		scene->addChild(layer);
		return scene;
	}

	enum {
		ID_VANZEMLJAK = 1,
		ID_TIMEBAR,
		ID_RECORD,
		ID_STOP
	};

	bool isRecording;

	virtual bool init()
	{
		if (!Layer::init()) return false;
		setKeyboardEnabled(true);
		setAnchorPoint(Vec2(0, 1));
		setScale(g_scale);
		setAnchorPoint(Vec2::ZERO);

		isRecording = false;

		Size visibleSize(g_screenSize);
		Vec2 origin = Vec2::ZERO;// Director::getInstance()->getVisibleOrigin();

		// create back button
		auto closeItem = MenuItemImage::create("back.png", "back.png", CC_CALLBACK_1(RecordScene::menuCloseCallback, this));
		auto pos = Vec2(origin.x + closeItem->getContentSize().width, origin.y + visibleSize.height - closeItem->getContentSize().height);
		closeItem->setPosition(pos);
		auto menu = Menu::create(closeItem, NULL);
		menu->setPosition(Vec2::ZERO);
		this->addChild(menu, 1);

		// add title label
		auto label = Label::createWithTTF("Record a sound!", "fonts/Big_Bottom_Typeface_Normal.ttf", 24);
		label->setColor(Color3B::ORANGE);
		label->setPosition(Vec2(origin.x + visibleSize.width / 2, pos.y));
		this->addChild(label, 1);

		// create record button
		auto recItem = MenuItemImage::create("record.png", "record.png", CC_CALLBACK_1(RecordScene::RecordCallback, this));
		recItem->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2));
		menu = Menu::create(recItem, NULL);
		menu->setPosition(Vec2::ZERO);
		menu->setTag(ID_RECORD);
		this->addChild(menu, 1);

		// create stop button
		recItem = MenuItemImage::create("stop.png", "stop.png", CC_CALLBACK_1(RecordScene::StopCallback, this));
		recItem->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2));
		menu = Menu::create(recItem, NULL);
		menu->setPosition(Vec2::ZERO);
		menu->setVisible(false);
		menu->setTag(ID_STOP);
		this->addChild(menu, 1);

		// create alien to show time
		auto sprite = Sprite::create("vanzemljak.png");
		pos = Vec2(origin.x + sprite->getContentSize().width / 2, origin.y + visibleSize.height / 4);
		sprite->setPosition(pos);
		sprite->setTag(ID_VANZEMLJAK);

		// and time bar
		int ss4 = sprite->getContentSize().height / 4;
		auto timebar = LayerColor::create(Color4B(255, 255, 255, 255), 1, ss4);
		pos.y -= ss4 / 2;
		timebar->setAnchorPoint(Vec2(0, 0.5f));
		timebar->setPosition(pos);
		timebar->setTag(ID_TIMEBAR);
		addChild(timebar);
		addChild(sprite, 0);

		return true;
	}
    
	// implemented in appDelgate
	void menuBackToChooseThemeScene(cocos2d::Ref* pSender);

	void menuCloseCallback(cocos2d::Ref* pSender)
	{
		if (isRecording) StopCallback(pSender);
		menuBackToChooseThemeScene(this);

		#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
			exit(0);
		#endif
	}

	virtual void onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event)
	{
		if (keyCode == EventKeyboard::KeyCode::KEY_ESCAPE) {
			menuCloseCallback(this);
			event->stopPropagation();
		}
	}

	void RecordCallback(cocos2d::Ref* pSender)
	{
		native_startAudioRec();
		isRecording = true;
		auto sprite = getChildByTag(ID_VANZEMLJAK);
		int sizex = g_screenSize.width - sprite->getContentSize().width;
		sprite->runAction(
			Sequence::createWithTwoActions(
				MoveBy::create(10.0, Vec2(sizex, 0)),
				CallFuncN::create(CC_CALLBACK_1(RecordScene::StopCallback, this))))->setTag(333);

		getChildByTag(ID_TIMEBAR)->runAction(ScaleBy::create(10.0, sizex, 1.0f))->setTag(333);
		getChildByTag(ID_RECORD)->setVisible(false);
		getChildByTag(ID_STOP)->setVisible(true);
	}

	void StopCallback(cocos2d::Ref* pSender)
	{
		static short buf[160000];
		getChildByTag(ID_TIMEBAR)->stopActionByTag(333);
		getChildByTag(ID_VANZEMLJAK)->stopActionByTag(333);
		getChildByTag(ID_RECORD)->setVisible(true);
		getChildByTag(ID_STOP)->setVisible(false);
		isRecording = false;

		Director::getInstance()->replaceScene(TransitionMoveInR::create(0.5f, AdjustScene::createScene()));

		int rec = native_stopAudioRec();
		if (rec && native_getRecBuffer(buf, rec)) {
			log("native_playBuffer() stop callback");
			native_playBuffer(buf, rec);
		}

	}

    // implement the "static create()" method manually
    CREATE_FUNC(RecordScene);
};
