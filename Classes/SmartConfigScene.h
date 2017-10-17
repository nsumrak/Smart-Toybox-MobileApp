#pragma once

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "AppDelegate.h"
#include "NativeDefs.h"
#include "RecordScene.h"

using namespace cocos2d;

#define PROGRESS_STEP (100.0 / (float)(SMART_CFG_DURATION + 1))
#define SMART_CFG_DURATION 30 // in seconds

class SmartConfigScene : public LayerColor
{
public:
	typedef std::function<void(SmartConfigScene*)> onDlgClosedCallback;

	//static Scene* createScene()
	//{
	//	auto scene = Scene::create();

	//	// create background, scale to height
	//	auto winsize = Director::getInstance()->getWinSize();
	//	auto bgimage = Sprite::create("BG.png");
	//	bgimage->setAnchorPoint(Vec2(0.0f, 0.0f));
	//	auto bgisz = bgimage->getContentSize();
	//	float xscale = winsize.width * 2 / bgisz.width;
	//	float yscale = winsize.height / bgisz.height;
	//	if (xscale > yscale) bgimage->setScale(xscale);
	//	else bgimage->setScale(yscale);
	//	bgimage->setPosition(Vec2(-winsize.width, 0.0f));
	//	scene->addChild(bgimage, -1);

	//	auto layer = SmartConfigScene::create();
	//	scene->addChild(layer);
	//	return scene;
	//}

	ui::EditBox *ssidInput, *pwdInput;
	ui::Button *btn;
	ui::LoadingBar *progress;
	Sprite *ball;
	Size progSize, ballSize;
	onDlgClosedCallback onClosedCb;

	virtual bool init()
	{
		if (!LayerColor::initWithColor(Color4B(80, 80, 100, 220))) return false;
		setKeyboardEnabled(true);
		setAnchorPoint(Vec2(0.5, 1));
		//setScale(g_scale);
		//setAnchorPoint(Vec2::ZERO);

		int startx = 50;
		int yoff = 100;
		int y0 = g_screenSize.height - yoff;
		// TODO temp code to fix problem on Pera's phone for NTI presentation
		//y0 = g_screenSize.height - 500;
		int y1 = y0 - yoff;
		int labelw = 100;
		Vec2 anchor(0, 0.5);
		Vec2 progPos(g_screenSize.width / 2, y1 - yoff);

		char ssid[32];
		native_getSSID(ssid, 32);
		auto ssidLabel = Label::createWithTTF("SSID", "fonts/Big_Bottom_Typeface_Normal.ttf", 20);
		ssidLabel->setColor(Color3B::ORANGE);
		ssidLabel->setPosition(Vec2(startx, y0));
		ssidLabel->setContentSize(Size(labelw, 68));
		ssidLabel->setAnchorPoint(anchor);
		addChild(ssidLabel);
		ssidInput = ui::EditBox::create(Size(400, 68), "textinput.png");
		ssidInput->setAnchorPoint(anchor);
		ssidInput->setPosition(Vec2(startx + labelw + 15, y0));
		ssidInput->setFontColor(Color3B::BLACK);
		ssidInput->setFontSize(24);
		ssidInput->setText(ssid);
		ssidInput->setInputMode(ui::EditBox::InputMode::SINGLE_LINE);
		ssidInput->setReturnType(ui::EditBox::KeyboardReturnType::DONE);
		ssidInput->setMaxLength(32);
		addChild(ssidInput);

		auto pwdLabel = Label::createWithTTF("Key", "fonts/Big_Bottom_Typeface_Normal.ttf", 20);
		pwdLabel->setColor(Color3B::ORANGE);
		pwdLabel->setPosition(Vec2(startx, y1));
		pwdLabel->setContentSize(Size(labelw, 68));
		pwdLabel->setAnchorPoint(anchor);
		addChild(pwdLabel);
		pwdInput = ui::EditBox::create(Size(400, 68), "textinput.png");
		pwdInput->setAnchorPoint(anchor);
		pwdInput->setPosition(Vec2(startx + labelw + 15, y1));
		pwdInput->setFontSize(24);
		pwdInput->setFontColor(Color3B::BLACK);
		pwdInput->setInputFlag(ui::EditBox::InputFlag::PASSWORD);
		pwdInput->setInputMode(ui::EditBox::InputMode::SINGLE_LINE);
		pwdInput->setReturnType(ui::EditBox::KeyboardReturnType::GO);
		addChild(pwdInput);
		//auto checkbox = ui::CheckBox::create("", "");

		auto progBg = Sprite::create("bar.png");
		progBg->setPosition(progPos);
		addChild(progBg);

		progress = ui::LoadingBar::create("barPress.png", 100);
		progress->setPosition(progPos);
		progress->setPercent(PROGRESS_STEP);
		progress->retain();
		addChild(progress);
		progSize = progress->getContentSize();
		startx = (g_screenSize.width - progSize.width) / 2;

		ball = Sprite::create("vanzemljak.png");
		ball->setPosition(Vec2(startx, progPos.y));
		ball->setAnchorPoint(Vec2(0, 0.5));
		addChild(ball);
		ballSize = ball->getContentSize();

		btn = ui::Button::create("btn.png", "btnPress.png");
		btn->setTitleText("Start");
		btn->setTitleFontSize(BUTTON_FONT_SIZE);
		btn->setPosition(Vec2(g_screenSize.width / 2, progPos.y - yoff));
		btn->addTouchEventListener([&](Ref *sender, ui::Widget::TouchEventType type) {
			if (type == ui::Widget::TouchEventType::ENDED) {
				start();
			}
		});
		addChild(btn);

		return true;
	}

	void setOnClosedCb(const onDlgClosedCallback &cb)
	{
		onClosedCb = cb;
	}

	void start()
	{
		native_startSmartConfig(ssidInput->getText(), pwdInput->getText());

		// update progress bar
		auto move = MoveBy::create(SMART_CFG_DURATION, Vec2(progSize.width - ballSize.width, 0));
		move->setTag(111);
		ball->runAction(move);
		schedule(schedule_selector(SmartConfigScene::update), 1);

		btn->setTitleText("Stop");
		btn->addTouchEventListener([&](Ref *sender, ui::Widget::TouchEventType type) {
			if (type == ui::Widget::TouchEventType::ENDED) {
				stop();
			}
		});

		auto delay = Sequence::createWithTwoActions(DelayTime::create(SMART_CFG_DURATION), CallFuncN::create([=](Node *n) {
			stop();
		}));
		runAction(delay);
	}

	void stop()
	{
		unschedule(schedule_selector(SmartConfigScene::update));
		ball->stopAllActions(); stopAllActions();
		native_stopSmartConfig();
		if (onClosedCb) onClosedCb(this);
	}

	void update(float dt) {
		progress->setPercent(progress->getPercent() + PROGRESS_STEP);
	}

	void menuCloseCallback(cocos2d::Ref* pSender)
	{
		Director::getInstance()->end();

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


	CREATE_FUNC(SmartConfigScene);
};
