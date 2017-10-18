#pragma once

#include "cocos2d.h"
#include "NativeDefs.h"
#include "RecordScene.h"
#include "AppDelegate.h"
#include "SendDlg.h"

using namespace cocos2d;

struct _Theme 
{
	std::string name;
	std::string path;
};

std::vector<_Theme> _fetchThemes()
{
	std::vector< _Theme> rt(0);
	//_Theme t = { "Default" };
	rt.push_back({ "English", "theme.stb" });
	rt.push_back({ "Serbian", "theme_sr.stb" });
	rt.push_back({ "Custom", "theme_custom.stb" });
	return rt;
}


class ChooseThemeScene : public Layer
{
private:
	std::vector< _Theme> themes;
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

		auto layer = ChooseThemeScene::create();
		scene->addChild(layer);
		return scene;
	}

	enum {
		ID_BUTTON,
		ID_THEME_BUTTONS = 10,
	};

	virtual bool init()
	{
		if (!Layer::init()) return false;
		setKeyboardEnabled(true);
		setAnchorPoint(Vec2(0, 1));
		setScale(g_scale);
		setAnchorPoint(Vec2::ZERO);

		Size visibleSize(g_screenSize);
		Vec2 origin = Vec2::ZERO;// Director::getInstance()->getVisibleOrigin();

		// create back button
		auto closeItem = MenuItemImage::create("back.png", "back.png", CC_CALLBACK_1(ChooseThemeScene::menuCloseCallback, this));
		auto pos = Vec2(origin.x + closeItem->getContentSize().width, origin.y + visibleSize.height - closeItem->getContentSize().height);
		closeItem->setPosition(pos);
		auto menu = Menu::create(closeItem, NULL);
		menu->setPosition(Vec2::ZERO);
		this->addChild(menu, 1);

		// add title label
		auto label = Label::createWithTTF("Chose a theme!", "fonts/Big_Bottom_Typeface_Normal.ttf", 24);
		label->setColor(Color3B::ORANGE);
		label->setPosition(Vec2(origin.x + visibleSize.width / 2, pos.y));
		this->addChild(label, 1);
		
		

//		//create start game button (not for users)
//		const float fwdbtn_y = 100.f;
//		auto btn = ui::Button::create("btn.png", "btnPress.png");
//		btn->setTitleText("Box Button");
//		btn->setVisible(true);
//		btn->setTag(ID_BUTTON);
//		btn->setTitleFontSize(BUTTON_FONT_SIZE);
//		btn->setPosition(Vec2(btn->getContentSize().width / 1.8, fwdbtn_y));
//		btn->addTouchEventListener([&](Ref *sender, ui::Widget::TouchEventType type) {
//			//auto btn_press_start = 0;
//			if (type == ui::Widget::TouchEventType::BEGAN) {
//				// TODO: time.now()?
//				//btn_press_start = time.now();
//			}
//			if (type == ui::Widget::TouchEventType::ENDED) {
//				auto p = getParent();
//				//auto dlg = SendDlg::create(p, "btn/" + (time.now() - btn_press_start), true, 0, 0, this);
//				//if (dlg) dlg->retain();
//				//this->removeFromParent();
//			}
//		});
//		addChild(btn);

		//create forward to record sceene button 
		const float fwdbtn_y = 100.f;
		auto btn = ui::Button::create("btn.png", "btnPress.png");
		btn->setTitleText("Record");
		btn->setVisible(true);
		btn->setTag(ID_BUTTON);
		btn->setTitleFontSize(BUTTON_FONT_SIZE);
		btn->setPosition(Vec2(g_screenSize.width - btn->getContentSize().width / 1.8, fwdbtn_y));
		btn->addTouchEventListener([&](Ref *sender, ui::Widget::TouchEventType type) {
			if (type == ui::Widget::TouchEventType::ENDED) {
				nextScreen();
			}
		});
		addChild(btn);

		//create theme button array
		this->themes = _fetchThemes();
		auto cnt = themes.size();

		auto dpi = Device::getDPI();
//		auto wh = Director::getInstance()->getWinSize().height;
//		auto whpx = Director::getInstance()->getWinSizeInPixels().height;
		cocos2d::log("dpi: %d, screen_h: %f", dpi, g_screenSize.height);
		float maxh = 0.8 * dpi / 2.54f ;  //ccca 1.2cm;
		//float h = (pos.y - fwdbtn_y - btn->getContentSize().height * 0.7f) / cnt;
		//if (h > maxh) h = maxh;
		float offset = 20; // label->getContentSize().height / 2;
		//float h = (pos.y - fwdbtn_y - btn->getContentSize().height * 0.6f) / (cnt >= 5 ? cnt : 5);
		//btn = ui::Button::create("bar.png", "barPress.png");
		
		int i = 0;
		for (auto iter= std::begin(themes); iter != std::end(themes); ++iter, ++i) {
			auto btn = ui::Button::create("btn.png", "btnPress.png");
			float y = pos.y - (btn->getContentSize().height + offset) * (i + 1);
			//btn->setSize(Size(btn->getContentSize().width * 2, btn->getContentSize().height));
			btn->setTitleText(iter->name);
			btn->setVisible(true);
			btn->setTag(ID_THEME_BUTTONS + i);
			btn->setTitleFontSize(20);
			btn->setPosition(Vec2(g_screenSize.width / 2, y));
			btn->addTouchEventListener(CC_CALLBACK_2(ChooseThemeScene::sendTheme, this));
			addChild(btn);
			if (isDemo())
				btn->setEnabled(false);
		}
		return true;
	}

	void sendTheme(Ref *sender, ui::Widget::TouchEventType type) 
	{
		if (type == ui::Widget::TouchEventType::ENDED) {
			auto bt = dynamic_cast<ui::Button*>(sender);
			if (bt) {
				auto p = getParent();
				auto t = this->themes[bt->getTag() - ID_THEME_BUTTONS];
				ssize_t size = 0;
				unsigned char *data = FileUtils::getInstance()->getFileData(this->themes[bt->getTag() - ID_THEME_BUTTONS].path, "r", &size);
				cocos2d::log("%s, %d", this->themes[bt->getTag() - ID_THEME_BUTTONS].path.data(), size);
				auto dlg = SendDlg::create(p, POST_THEME, true, (char *)data, size, this, [](int respcode, char *data) { log("freeing data!"); if (data) free(data); });
				if (dlg) dlg->retain();
				this->removeFromParent();
			}
		}
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

	void nextScreen()
	{
		// change to recordscene
		Director::getInstance()->replaceScene(TransitionFlipAngular::create(0.5f, RecordScene::createScene()));
	}
	// implement the "static create()" method manually
	CREATE_FUNC(ChooseThemeScene);
};
