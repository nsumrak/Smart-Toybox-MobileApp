#pragma once

#include "cocos2d.h"
#include "NativeDefs.h"
#include "RecordScene.h"
#include "ChooseThemeScene.h"
#include "SmartConfigScene.h"
#include "AppDelegate.h"

using namespace cocos2d;

#define SEARCH_TIME_SEC	60

class FindBoxScene : public Layer
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
		float xscale = winsize.width * 2 / bgisz.width;
		float yscale = winsize.height / bgisz.height;
		if (xscale > yscale) bgimage->setScale(xscale);
		else bgimage->setScale(yscale);
		scene->addChild(bgimage, -1);

		auto layer = FindBoxScene::create();
		scene->addChild(layer);
		return scene;
	}

	enum {
		ID_VANZEMLJAK = 1,
		ID_BOX,
		ID_BUTTON,
		ID_PRSTAC,
		ID_SMALLBOXES = 10,
	};

	ui::Button *btn;
	Menu *menu;

	virtual bool init()
	{
		savednum = 0;

		if (!Layer::init()) return false;
		setKeyboardEnabled(true);
		setAnchorPoint(Vec2(0, 1));
		setScale(g_scale);
		setAnchorPoint(Vec2::ZERO);

		Size visibleSize(g_screenSize);
		Vec2 origin = Vec2::ZERO;// Director::getInstance()->getVisibleOrigin();

		// create back button
		auto closeItem = MenuItemImage::create("back.png", "back.png", CC_CALLBACK_1(FindBoxScene::menuCloseCallback, this));
		auto pos = Vec2(origin.x + closeItem->getContentSize().width, origin.y + visibleSize.height - closeItem->getContentSize().height);
		closeItem->setPosition(pos);

		auto addItem = MenuItemImage::create("add.png", "addPress.png", CC_CALLBACK_1(FindBoxScene::menuAddCallback, this));
		pos = Vec2(visibleSize.width - closeItem->getContentSize().width, origin.y + visibleSize.height - closeItem->getContentSize().height);
		addItem->setPosition(pos);

		menu = Menu::create(closeItem, addItem, NULL);
		menu->setPosition(Vec2::ZERO);
		this->addChild(menu, 1);

		// add title label
		auto label = Label::createWithTTF("Searching for Toybox...", "fonts/Big_Bottom_Typeface_Normal.ttf", 24);
		label->setColor(Color3B::ORANGE);
		label->setPosition(Vec2(origin.x + visibleSize.width / 2, pos.y));
		this->addChild(label, 1);

		// create central box
		auto box = Sprite::create("box.png");
		pos = Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2);
		box->setPosition(pos);
		box->setTag(ID_BOX);
		box->setColor(Color3B(64, 64, 64));
		addChild(box);
		auto boxs = box->getContentSize();

		// create button
		btn = ui::Button::create("btn.png", "btnPress.png");
		btn->setTitleText("Demo");
		btn->setVisible(false);
		btn->setTag(ID_BUTTON);
		btn->setTitleFontSize(BUTTON_FONT_SIZE);
		btn->setPosition(Vec2(g_screenSize.width - (btn->getContentSize().width / 2), 100));
		btn->addTouchEventListener([&](Ref *sender, ui::Widget::TouchEventType type) {
			if (type == ui::Widget::TouchEventType::ENDED) {
				nextScreen();
			}
		});
		addChild(btn);

		//show Demo button immediatelly
		this->btn->setVisible(true);
		this->btn->setScale(0.1f);
		this->btn->runAction(
			Spawn::createWithTwoActions(
				ScaleTo::create(0.2f, 1.0f),
				EaseBackOut::create(MoveTo::create(0.2f, Vec2(g_screenSize.width - (this->btn->getContentSize().width / 1.8), 100)))));


		// create alien to show time
		auto sprite = Sprite::create("vanzemljak.png");
		sprite->setPosition(pos);
		sprite->setTag(ID_VANZEMLJAK);
		auto ap = Vec2(0.5f, boxs.height / sprite->getContentSize().height);
		sprite->setAnchorPoint(ap);
		addChild(sprite);
		sprite->runAction(
			Sequence::createWithTwoActions(
				Repeat::create(
					Sequence::create(
						RotateBy::create(0.25f, 90.0f), CallFunc::create(CC_CALLBACK_0(FindBoxScene::searchTick, this)),
						RotateBy::create(0.25f, 90.0f), CallFunc::create(CC_CALLBACK_0(FindBoxScene::searchTick, this)),
						RotateBy::create(0.25f, 90.0f), CallFunc::create(CC_CALLBACK_0(FindBoxScene::searchTick, this)),
						RotateBy::create(0.25f, 90.0f), CallFunc::create(CC_CALLBACK_0(FindBoxScene::searchTick, this)),
						nullptr), SEARCH_TIME_SEC),
				CallFunc::create(CC_CALLBACK_0(FindBoxScene::searchTimeout, this))));

		auto prstac = ParticleSystemQuad::create("trace.plist");
		prstac->setPosition(pos);
		addChild(prstac, -1);
		prstac->setTag(ID_PRSTAC);
		//spline that emulates circle
		int segments = 12;
		int delta_angle = -360 / segments;
		auto posp = Vec2(ap.x * sprite->getContentSize().width, ap.y * -sprite->getContentSize().height);
		auto r = posp.length();
		auto phaseort = posp.getNormalized();
		auto pc = phaseort.x;
		auto ps = phaseort.y;
		auto pts = PointArray::create(segments + 1);
		for (int i = 0; i < segments; ++i) {
			float ang = CC_DEGREES_TO_RADIANS(float(i * delta_angle));
			float s = sin(ang);
			float c = cos(ang);
			pts->addControlPoint(Vec2(pos.x + r*(c*pc - s*ps), pos.y + r*(s*pc + c*ps)));
		}
		pts->addControlPoint(pts->getControlPointAtIndex(0));
		prstac->runAction(Repeat::create(CardinalSplineTo::create(1.0f, pts, 0.5f), SEARCH_TIME_SEC));

		auto p2 = ParticleSystemQuad::create("thum_cloud.plist");
		p2->setName("prstac2");
		sprite->addChild(p2);
		p2->setPosition(75.f, 75.f);

		native_startDnsSD();
		return true;
	}

	void menuCloseCallback(cocos2d::Ref* pSender)
	{
		Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
		exit(0);
#endif
	}

	void menuAddCallback(cocos2d::Ref* pSender)
	{
		btn->setEnabled(false);
		menu->setEnabled(false);
		auto p = getParent();
		auto dlg = SmartConfigScene::create();
		if (dlg) {
			dlg->setOnClosedCb([&](SmartConfigScene* sender) {
				if (sender) {
					sender->runAction(Sequence::createWithTwoActions(
						MoveTo::create(0.5, Vec2(0, getContentSize().height)),
						CallFuncN::create([=](Node *n) {
						btn->setEnabled(true);
						menu->setEnabled(true);
						sender->removeFromParent();
					})));
				}
				else {
					btn->setEnabled(true);
					menu->setEnabled(true);
				}
			});
			dlg->setScale(0.75);
			dlg->setPosition(0, dlg->getContentSize().height);
			dlg->runAction(MoveTo::create(0.5, Vec2::ZERO));
			p->addChild(dlg);
		}
	}

	virtual void onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event)
	{
		if (keyCode == EventKeyboard::KeyCode::KEY_ESCAPE) {
			menuCloseCallback(this);
			event->stopPropagation();
		}
	}

	int savednum;

	void searchTick()
	{
		int num = native_getDnsSDdiscoveryNum();
		if (num == savednum) return;
		if (num < savednum) {
			// should remove found
			//while (num < savednum) {
			//	savednum--;

			//	getChildByTag(ID_SMALLBOXES + savednum)->runAction(
			//		Sequence::createWithTwoActions(
			//			ScaleTo::create(0.5f, 0.1f),
			//			RemoveSelf::create()));
			//}
			// if !num remove button
			//if (!num) getChildByTag(ID_BUTTON)->setVisible(false);
			return;
		}
		else {
			do {
				doHttpRequest(savednum++);
			} while (num > savednum);
		}
	}

	void doHttpRequest(int num) {
		char buf[256];
		sprintf(buf, "%d", num);
		HttpRequest* request = new (std::nothrow) HttpRequest();
		request->setTag(buf);

		strcpy(buf, "http://");
		int len = strlen(buf);
		native_getDnsSDdiscoveryItem(num, buf + len, sizeof(buf) - len);
		strcat(buf, ":80/");
		strcat(buf, "uname");
		request->setUrl(buf);
		log("doing discovery http request on %s", buf);

		request->setRequestType(HttpRequest::Type::GET);
		request->setResponseCallback([&](HttpClient* sender, HttpResponse* response) {
			log("got discovery http response: %d", response->getResponseCode());
			if (!response || response->getResponseCode() != 200) {
				log("discovery http error, num = %s", response->getHttpRequest()->getTag());
				return;
			}
			int num = atoi(response->getHttpRequest()->getTag());
			log("discovery http response OK num=%d", num);
			// if num = 0, item is gw, not found by mdns. check if it is a toybox
			//std::vector<char> *buffer = response->getResponseData();
			//const char *expected_resp = "esp_toybox";
			//if (num == 0) {
			//	if (strncmp(expected_resp, buffer->data(), strlen(expected_resp))) {
			//		log("gateway is not a toybox, response data");
			//		return;
			//	}
			//}
			char buf[256];
			native_getDnsSDdiscoveryItem(num, buf, 256);
			g_toyboxIPs.push_back(buf);

			auto box = Sprite::create("box.png");
			box->setPosition(Vec2(g_screenSize.width / 2, g_screenSize.height / 2));
			box->setTag(ID_SMALLBOXES + num);
			box->setScale(0.1f);
			addChild(box);
			box->runAction(
				Spawn::createWithTwoActions(
					ScaleTo::create(0.5f, 0.5f),
					EaseBackInOut::create(MoveTo::create(0.5f, Vec2(box->getContentSize().width / 1.5f*(num + 0.5f), 100)))));

			if (g_toyboxIPs.size() == 1) {
				log("found box, changing button to start");
				auto btn = getChildByTag(ID_BUTTON);
				this->btn->setTitleText("Start");
				btn->setVisible(true);
				btn->setScale(0.1f);
				btn->runAction(
					Spawn::createWithTwoActions(
						ScaleTo::create(0.2f, 1.0f),
						EaseBackOut::create(MoveTo::create(0.2f, Vec2(g_screenSize.width - (btn->getContentSize().width / 1.8), 100)))));
			}
		});
		HttpClient::getInstance()->sendImmediate(request);
		request->release();
	}

	void searchTimeout()
	{
		((ParticleSystem*)getChildByTag(ID_PRSTAC))->stopSystem();
		auto p2 = dynamic_cast<ParticleSystem*>(this->getChildByTag(ID_VANZEMLJAK)->getChildByName("prstac2"));
		if (p2) p2->stopSystem();

		if (g_toyboxIPs.size() > 0) {
			this->btn->setTitleText("Start");
			return; // probably should auto switch to next screen
		} else {
			////show Demo button only if search time expired and there was no successifull connection to at least one box
			//this->btn->setTitleText("Demo");
			//this->btn->setVisible(true);
			//this->btn->setScale(0.1f);
			//this->btn->runAction(
			//	Spawn::createWithTwoActions(
			//		ScaleTo::create(0.2f, 1.0f),
			//		EaseBackOut::create(MoveTo::create(0.2f, Vec2(g_screenSize.width - (this->btn->getContentSize().width / 1.8), 100)))));
		}
	}

	void nextScreen()
	{
		native_stopDnsSD();
		Director::getInstance()->replaceScene(TransitionFlipAngular::create(0.5f, ChooseThemeScene::createScene()));
	}

	// implement the "static create()" method manually
	CREATE_FUNC(FindBoxScene);
	};
