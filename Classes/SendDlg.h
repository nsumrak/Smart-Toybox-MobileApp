#pragma once

#include "cocos2d.h"
#include "AppDelegate.h"
#include "NativeDefs.h"
#include "network/HttpClient.h"
#include "ui/CocosGUI.h"

using namespace cocos2d;
using namespace network;

#define PAUSE_MOVE_TIME		0.5f

//#define TOYBOX_URL "http://192.168.0.101:80/"
#define POST_SOUND "snd/a"
#define POST_THEME "thm"


class SendDlg : public Layer
{
public:
	//static void pauseRecursive(Node *n)
	//{
	//	auto children = n->getChildren();
	//	for (int i = 0; i < children.size(); i++) {
	//		Node *q = children.at(i);
	//		q->pause();
	//		pauseRecursive(q);
	//	}
	//}

	//static void resumeRecursive(Node *n)
	//{
	//	auto children = n->getChildren();
	//	for (int i = 0; i < children.size(); i++) {
	//		Node *q = children.at(i);
	//		q->resume();
	//		resumeRecursive(q);
	//	}
	//}

	char *data;
	unsigned size;
	const char *reqobj;
	bool post;
	std::string landingPageUrl;

	float yoffs;
	ui::Button *closeBtn;
	Layer *caller;
	void(*on_finish_cb)(int respcode, char *data);
	ui::Text *textwidget;
	

	void _init()
	{
		auto winsize = getParent()->getContentSize();
		Vec2 lmid(winsize.width / 2, winsize.height / 2 + 70);
		Vec2 mmid(winsize.width / 2, winsize.height / 2 - 70);
		yoffs = winsize.height * 0.75f;

		if (isDemo()) {
			this->textwidget = ui::Text::create("Sending...", "fonts/Big_Bottom_Typeface_Normal.ttf", 24);
			this->textwidget->ignoreContentAdaptWithSize(false);
			this->textwidget->setPosition(Vec2(lmid.x, lmid.y + yoffs));
			this->textwidget->setContentSize(Size(winsize.width *.95f, winsize.height / 2.f - 10 - 70));
			this->textwidget->setOpacityModifyRGB(true);
			this->textwidget->setTextHorizontalAlignment(TextHAlignment::CENTER);
			this->textwidget->setColor(Color3B::ORANGE);
			this->textwidget->setVisible(true);
			addChild(this->textwidget);
			this->textwidget->setTouchEnabled(true);
			this->textwidget->addTouchEventListener([&](Ref *sender, ui::Widget::TouchEventType type) {
				if (type == ui::Widget::TouchEventType::ENDED) {
					auto res = Application::getInstance()->openURL(this->landingPageUrl);
					if (!res) {
						log("SendDlg could not open url: %s", this->landingPageUrl.data());
						assert(res);
						Application::getInstance()->openURL("http://www.smarttoybox.com");
					}
				}
			});
		} else {
			this->textwidget = ui::Text::create("Sending...", "fonts/Big_Bottom_Typeface_Normal.ttf", 30);
			this->textwidget->setPosition(Vec2(lmid.x, lmid.y + yoffs));
			this->textwidget->setColor(Color3B::ORANGE);
			this->textwidget->enableShadow();
			this->textwidget->setVisible(true);
			this->addChild(this->textwidget);
		}

		closeBtn = ui::Button::create("btn.png", "btnPress.png");
		closeBtn->setTitleText("Close");
		closeBtn->setTitleFontSize(BUTTON_FONT_SIZE);
		closeBtn->addTouchEventListener([&](Ref *sender, ui::Widget::TouchEventType type) {
			if (type == ui::Widget::TouchEventType::ENDED)
				doResume();
		});
		closeBtn->setEnabled(false);
		addChild(closeBtn);
		closeBtn->setPosition(Vec2(mmid.x, mmid.y + yoffs));

		this->textwidget->runAction(EaseSineOut::create(MoveTo::create(PAUSE_MOVE_TIME, lmid)));
		closeBtn->runAction(EaseSineOut::create(MoveTo::create(PAUSE_MOVE_TIME, mmid)));

		send();
	}

	void doResume() {
		Vec2 mup(closeBtn->getPosition()), lup(this->textwidget->getPosition());
		mup.y += yoffs; lup.y += yoffs;
		this->textwidget->runAction(EaseSineIn::create(MoveTo::create(PAUSE_MOVE_TIME, lup)));
		closeBtn->runAction(Sequence::createWithTwoActions(EaseSineIn::create(MoveTo::create(PAUSE_MOVE_TIME, mup)), CallFuncN::create([=](Node *n) {
			Node *parent = this->getParent();
			if (caller) parent->addChild(caller);
			removeFromParent();
			//resumeRecursive(parent);
			//getEventDispatcher()->setEnabled(true);
		})));
		release();
	}

	virtual void send() {
		if (post && (!data || !size)) {
			this->textwidget->setString("Error");
			cocos2d::log("There is no data to be sent.");
			on_finish_cb(-1, data);
			closeBtn->setEnabled(true);
			return;
		}
		if (!isDemo()) {
			HttpRequest* request = new (std::nothrow) HttpRequest();
			char buf[256];

			strcpy(buf, "http://");
			size_t len = strlen(buf);
			native_getDnsSDdiscoveryItem(0, buf + len, (int)(sizeof(buf) - len));
			strcat(buf, ":8080/"); // TODO don't hardcore port
			strcat(buf, reqobj);
			request->setUrl(buf);
			//request->setUrl(TOYBOX_URL POST_SOUND);

			request->setRequestType(post ? HttpRequest::Type::POST : HttpRequest::Type::GET);
			log("sending to toybox, %s: %s", post ? "POST" : "GET", buf);

			if (post) request->setRequestData((char *)data, size);
			request->setResponseCallback([&](HttpClient* sender, HttpResponse* response) {
				long respCode = -1;
				if (!response || (respCode = response->getResponseCode()) != 200) {
					log("http error: %ld", respCode);
					this->textwidget->setString("Error");
				}
				else {
					this->textwidget->setString("Success");
				}
				on_finish_cb(respCode, data);
				closeBtn->setEnabled(true);
			});
			HttpClient::getInstance()->sendImmediate(request);
			request->release();
		}
	}

	static SendDlg *create(Node *scene, const char *req, bool post = false, char *data = 0, int size = 0, Layer *l = 0, void(*on_finish_cb)(int respcode, char *data) = [](int rc, char *d) { log("on_finish_cb(): deafult impl, respcode = %d", rc); }) {
		SendDlg *layer = new (std::nothrow) SendDlg();
		return doCreate(layer, scene, req, post, data, size, l, on_finish_cb);
	}

	static SendDlg *doCreate(SendDlg *layer, Node *scene, const char *req, bool post, char *data, int size, Layer *l, void(*on_finish_cb)(int respcode, char *data))
	{
		//pauseRecursive(scene);
		//getEventDispatcher()->setEnabled(false);
		//auto winsize = scene->getContentSize();
		if (layer && layer->init()) { //WithColor(Color4B(0, 0, 0, 128), winsize.width, winsize.height)) {
			layer->autorelease();
			scene->addChild(layer);
			layer->reqobj = req;
			layer->post = post;
			if (post) {
				layer->data = data;
				layer->size = size;
			} else {
				data = 0; size = 0;
			}
			layer->caller = l;
			layer->caller->retain();
			layer->on_finish_cb = on_finish_cb;
			layer->_init();
			return layer;
		}
		CC_SAFE_DELETE(layer);
		return nullptr;
	}
};

class SendSoundDlg : public SendDlg {
public:
	static SendDlg *create(Node *scene, const char *req, bool post = false, char *data = 0, int size = 0, Layer *l = 0, void(*on_finish_cb)(int respcode, char *data) = [](int rc, char *d) { log("on_finish_cb(): deafult imple, respcode = %d", rc); }) {
		SendDlg *layer = new (std::nothrow) SendSoundDlg();
		return doCreate(layer, scene, req, post, data, size, l, on_finish_cb);
	}

protected:
	virtual void send() {
		SendDlg::send();
		//send sound to server
		HttpRequest* request = new (std::nothrow) HttpRequest();
		request->setRequestType(HttpRequest::Type::POST);
		request->setUrl("http://www.smarttoybox.com/soundupload.php");
		std::vector<std::string> headers;
		headers.push_back("Content-Type: application/octet-stream");
		request->setHeaders(headers);
		request->setRequestData((char *)data, size);
		if (isDemo()) {
			request->setResponseCallback([&](HttpClient* sender, HttpResponse* response) {
				long respCode = -1;
				if (!response || (respCode = response->getResponseCode()) != 200) {
					log("http sound upload error: %ld", respCode);
					this->textwidget->setString("Error");
				} else {
					std::string resstr(response->getResponseData()->data());
					auto pos = resstr.find('\n');
					auto last = resstr.find_last_of("\n");
					auto msg = last != std::string::npos ? resstr.substr(pos + 1, last - pos - 1) : resstr;
					this->landingPageUrl = resstr.substr(0, pos);
					assert(this->textwidget);
					this->textwidget->setText(msg);
					this->textwidget->setVisible(true);
					//assert(memcmp(&resstr[0], this->data, this->size) == 0);
				}
				closeBtn->setEnabled(true);
			});
		} else {
			request->setResponseCallback([&](HttpClient* sender, HttpResponse* response) {
				long respCode = -1;
				if (!response || (respCode = response->getResponseCode()) != 200) {
					log("http sound upload error: %ld", respCode);
				}
			});
		}
		HttpClient::getInstance()->send(request);
		request->release();
	}
};