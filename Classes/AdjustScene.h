#pragma once

#include "cocos2d.h"
#include "AppDelegate.h"
#include "NativeDefs.h"
#include "Slider.h"
#include "../SoundTouch/include/SoundTouch.h"
#include "ui/CocosGUI.h"
#include "wavpack/wavpack.h"
#include "SendDlg.h"

using namespace cocos2d;
using namespace soundtouch;

#define SOUNDTOUCH_CHUNK_SIZE	6720


int change_pitch(short *outbuf, short *inbuf, int numsamples, double tempoDelta = 0, double pitchDelta = 0, double rateDelta = 0,
	double volumeDelta = 0, bool quick = false, bool doAA = true, bool speach = true)
{
	SoundTouch st;
	st.setChannels(1);
	st.setSampleRate(16000);
	st.setPitchSemiTones(5.0);

	st.setTempoChange(tempoDelta);
	st.setPitchSemiTones(pitchDelta);
	st.setRateChange(rateDelta);
	st.setSetting(SETTING_USE_QUICKSEEK, (int)quick);
	st.setSetting(SETTING_USE_AA_FILTER, (int)doAA);
	if (speach) {
		// use settings for speech processing
		st.setSetting(SETTING_SEQUENCE_MS, 40);
		st.setSetting(SETTING_SEEKWINDOW_MS, 15);
		st.setSetting(SETTING_OVERLAP_MS, 8);
	}

	int res, written = 0;
	for (int readed = 0; readed < numsamples; ) {
		int toread = numsamples - readed;
		if (toread >= SOUNDTOUCH_CHUNK_SIZE) toread = SOUNDTOUCH_CHUNK_SIZE;
		st.putSamples(inbuf + readed, toread);
		readed += toread;

		do {
			int towrite = 160000 - written;
			if (towrite <= 0) break;
			written += (res = st.receiveSamples(outbuf + written, towrite));
		}  while (res != 0);
	}
	st.flush();
	do {
		int towrite = 160000 - written;
		if (towrite <= 0) break;
		written += (res = st.receiveSamples(outbuf + written, towrite));
	}  while (res != 0);

	if (volumeDelta != 0.0) {
		volumeDelta = tan((volumeDelta + 100.0) * 0.0079);
		short *t = outbuf;
		for (int i = 0; i < written; i++) {
			int res = (int)((double)*t * volumeDelta);
			if (res > 0x7fff) res = 0x7fff;
			if (res < -0x7fff) res = -0x7fff;
			*t++ = (short)res;
		}
	}
	return written;
}


void fadein(short *buf, int numsamples, unsigned milisec) {
	if (numsamples <= 0)
		return;
	const int srate = 16000/1000;
	int samples = srate * milisec + 1;
	if (numsamples < samples)
		samples = numsamples;
	int i = 1;
	for (short *iter = buf, *end = &buf[samples-1]; iter != end; ++i, ++iter)
		*iter = int(*iter) * i / samples;
}


void fadeout(short *buf, int numsamples, unsigned short milisec) {
	if (numsamples <= 0)
		return;
	const int srate = 16000 / 1000;
	int samples = srate * milisec + 1;
	if (numsamples < samples)
		samples = numsamples;
	int i = 1;
	for (short *iter = &buf[numsamples - 1], *end = &buf[numsamples - samples]; iter != end; ++i, --iter)
		*iter = int(*iter) * i / samples;
}


class AdjustScene : public Layer
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
		bgimage->setPosition(Vec2(-winsize.width, 0.0f));
		scene->addChild(bgimage, -1);

		auto layer = AdjustScene::create();
		scene->addChild(layer);
		return scene;
	}

	enum {
		ID_VANZEMLJAK = 1,
		ID_TIMEBAR,
		ID_RECORD,
		ID_STOP
	};

	short originalBuf[160000];
	int originalSize;
	short adjBuf[320000];
	unsigned adjSize;
	bool sliderChanged;

	Slider *pitch, *tempo, *rate, *volume, *clip;
	Sprite *rect;

	virtual bool init()
	{
		if (!Layer::init()) return false;
		setKeyboardEnabled(true);
		setAnchorPoint(Vec2(0,1));
		setScale(g_scale);
		setAnchorPoint(Vec2::ZERO);

		originalSize = native_stopAudioRec();
		if (!originalSize) {
			menuBackToRecordScene(this);
		}
		else
		{
			native_getRecBuffer(originalBuf, originalSize);
			log("native_playBuffer() init");
			native_playBuffer(originalBuf, originalSize);
		}

		Size visibleSize = g_screenSize;
		//log("g_screenSize w = %g, h = %g", g_screenSize.width, g_screenSize.height);
		Vec2 origin = Vec2::ZERO;

		// create back button
		auto closeItem = MenuItemImage::create("back.png", "back.png", CC_CALLBACK_1(AdjustScene::menuBackToRecordScene, this));
		auto pos = Vec2(origin.x + closeItem->getContentSize().width, origin.y + visibleSize.height - closeItem->getContentSize().height);
		closeItem->setPosition(pos);
		auto menu = Menu::create(closeItem, NULL);
		menu->setPosition(Vec2::ZERO);
		this->addChild(menu, 1);

		// add title label
		auto label = Label::createWithTTF("Sound adjustment", "fonts/Big_Bottom_Typeface_Normal.ttf", 24);
		label->setColor(Color3B::ORANGE);
		label->setPosition(Vec2(origin.x + visibleSize.width / 2, pos.y));
		this->addChild(label, 1);

		pitch = Slider::docreate("Pitch", "elephant", "mouse", CC_CALLBACK_0(AdjustScene::sliderChange, this));
		tempo = Slider::docreate("Tempo", "turtle", "rabbit", CC_CALLBACK_0(AdjustScene::sliderChange, this));
		rate = Slider::docreate("Rate", "chronobig", "chronosmall", CC_CALLBACK_0(AdjustScene::sliderChange, this));
		volume = Slider::docreate("Volume", "volumelow", "volumehigh", CC_CALLBACK_0(AdjustScene::sliderChange, this));
		clip = Slider::docreate("Clip", "scissorsl", "scissorsr", CC_CALLBACK_0(AdjustScene::sliderChange, this), true);
		float sz = (g_screenSize.height - 200.0) / 5.0;
		volume->setPosition(Vec2(0, 100));
		rate->setPosition(Vec2(0, 100+sz));
		tempo->setPosition(Vec2(0, 100+2*sz));
		pitch->setPosition(Vec2(0, 100+3*sz));
		clip->setPosition(Vec2(0, 100+4*sz));
		addChild(pitch);
		addChild(tempo);
		addChild(rate);
		addChild(volume);
		addChild(clip);

		auto button = ui::Button::create("btn.png", "btnPress.png");
		button->setTitleText("Send");
		button->setTitleFontSize(BUTTON_FONT_SIZE);
		button->addTouchEventListener([&](Ref *sender, ui::Widget::TouchEventType type) {
			if (type == ui::Widget::TouchEventType::ENDED) {
				if (prepare()) {
					auto p = getParent();
					auto dlg = SendSoundDlg::create(p, POST_SOUND, true, (char *)adjBuf, adjSize, this);
					if (dlg) dlg->retain();
					this->removeFromParent();
					adjSize = 0;
				}
			}
		});
		//button->addTouchEventListener([&](Ref *sender, ui::Widget::TouchEventType type) {
		//	if (type == ui::Widget::TouchEventType::ENDED) {
		//		Application::getInstance()->openURL("http://www.smarttoybox.com");
		//	}
		//});

		button->setPosition(Vec2(g_screenSize.width / 2, 20));
		button->setAnchorPoint(Vec2(0.5, 0));
		this->addChild(button, 1);
		//button->setVisible(!isDemo());

		adjSize = 0;
		sliderChanged = false;

		return true;
	}
    
	// implemented in appDelgate
	void menuBackToRecordScene(cocos2d::Ref* pSender);

	virtual void onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event)
	{
		if (keyCode == EventKeyboard::KeyCode::KEY_ESCAPE) {
			menuBackToRecordScene(this);
			event->stopPropagation();
		}
	}

	void sliderChange()
	{
		sliderChanged = true;
		double p = (double)(pitch->getPos()) * 0.12;
		int pos = tempo->getPos();
		double t = (double)pos * (pos < 0 ? 0.5 : 0.95);
		pos = rate->getPos();
		double r = (double)pos	* (pos < 0 ? 0.5 : 0.95);
		double v = (double)(volume->getPos());
		int start = (clip->getPos() + 100)*originalSize / 200;
		int size = ((clip->getPos2() + 100)*originalSize / 200) - start;
		adjSize = change_pitch(adjBuf, originalBuf+start, size, t, p, r, v);
		log("native_playBuffer() slider change");
		native_playBuffer(adjBuf, adjSize);
	}

	bool prepare()
	{
		if (!adjSize) {
			if (sliderChanged)
				sliderChange();
			else
				memcpy(adjBuf, originalBuf, (adjSize = originalSize) * 2);
		}

		fadein(adjBuf, adjSize, 100);
		fadeout(adjBuf, adjSize, 100);

		adjSize = pack_buffer((char *)adjBuf, adjSize);
		log("pack_buffer returned %d", adjSize);
		if (adjSize) {
			//FILE *f = fopen("smarttoybox.wv", "w+b");
			//if (f) {
			//	if (fwrite(adjBuf, 1, adjSize, f) < adjSize)
			//		log("write to file failed: %d", adjSize);
			//	fclose(f);
			//} else {
			//	log("could not open file for writing");
			//}
			return true;
		}
		return false;
	}

    // implement the "static create()" method manually
    CREATE_FUNC(AdjustScene);
};
