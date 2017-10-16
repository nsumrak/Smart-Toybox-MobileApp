#pragma once

#include "cocos2d.h"
#include "AppDelegate.h"

using namespace cocos2d;

#define SLIDER_PAGE_SIZE	10

typedef std::function<void()> ccSliderCallback;

class _SliderBatchSprite : public SpriteBatchNode
{
protected:
	ParticleSystemQuad *prstac;
	Sprite *thumb, *bar, *levi, *desni, *thumb2, *rangebar;
	int _height;
	int _pos, _pos2;
	int _pressed;

public:
	ccSliderCallback _callback;

	void initme(const char *title, const char *leviName, const char *desniName, bool range)
	{
		SpriteFrameCache::getInstance()->addSpriteFramesWithFile("appsprites.plist");
		auto listener1 = EventListenerTouchOneByOne::create();
		listener1->setSwallowTouches(true);
		listener1->onTouchBegan = [&](Touch* touch, Event* event) {
			Vec2 local = convertToNodeSpace(touch->getLocation());

			if (local.y > _height || local.y < 0) return false;
			Vec2 pos = thumb->getPosition();
			Size s(thumb->getContentSize());

			const float scl= 2.f;
			Rect r(pos.x - s.width / 2.f * scl, pos.y - s.height / 2.f * scl, s.width  * scl, s.height * scl);
			if (r.containsPoint(local)) {
				// on thumb
				if (_pressed) listener1->onTouchEnded(touch, event);
				_pressed = 1; // track this
				prstac->setPosition(prstac->getParent()->convertToNodeSpace(convertToWorldSpace(pos)));
				prstac->resetSystem();
				return true;
			}
			else if (thumb2) {
				pos = thumb2->getPosition();
				s = thumb2->getContentSize();
//				Rect r2(pos.x - s.width / 2, pos.y - s.height / 2, s.width, s.height);
				Rect r2(pos.x - s.width / 2.f * scl, pos.y - s.height / 2.f * scl, s.width  * scl, s.height * scl);
				if (r2.containsPoint(local)) {
					// on thumb
					if (_pressed) listener1->onTouchEnded(touch, event);
					_pressed = 2; // track this
					prstac->setPosition(prstac->getParent()->convertToNodeSpace(convertToWorldSpace(pos)));
					prstac->resetSystem();
				}
				else {
					// left of left or right of right
					if (local.x < r.getMinX()) {
						setPos(_pos - SLIDER_PAGE_SIZE);
						_callback();
					}
					else if (local.x > r2.getMaxX()) {
						setPos2(_pos2 + SLIDER_PAGE_SIZE);
						_callback();
					}
				}
			}
			else {
				// left or right
				if (local.x < r.getMinX()) setPos(_pos - SLIDER_PAGE_SIZE);
				else setPos(_pos + SLIDER_PAGE_SIZE);
				_callback();
			}
			return true;
		};
		listener1->onTouchMoved = [&](Touch* touch, Event* event) {
			if (!_pressed) return;
			Vec2 local = convertToNodeSpace(touch->getLocation());
			float lx = local.x - bar->getPosition().x;
			float w = bar->getContentSize().width / 2;
			if (lx < -w || lx > w) return;
			if (thumb2) {
				// prevent overlap
				if (_pressed == 1 && local.x + thumb->getContentSize().width / 2 > thumb2->getPosition().x - thumb2->getContentSize().width / 2) return;
				if (_pressed == 2 && local.x - thumb2->getContentSize().width / 2 < thumb->getPosition().x + thumb->getContentSize().width / 2) return;
			}
			if (_pressed == 1) setPos((lx / w) * 100, false);
			else setPos2((lx / w) * 100, false);
			if (this->prstac) 
				this->prstac->setPosition(this->prstac->getParent()->convertToNodeSpace(Vec2(touch->getLocation().x, touch->getLocation().y)));
		};

		listener1->onTouchEnded = [=](Touch* touch, Event* event) {
			if (_pressed) _callback();
			_pressed = 0;
			if (prstac)
				prstac->stopSystem();
		};
		listener1->onTouchCancelled = [=](Touch* touch, Event* event) {
			listener1->onTouchEnded(touch, event);
		};
		Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener1, this);

		_pos = _pos2 = 0;
		_pressed = false;

		levi = Sprite::createWithSpriteFrameName(leviName);
		desni = Sprite::createWithSpriteFrameName(desniName);
		thumb = Sprite::createWithSpriteFrameName("vanzemljak");
		bar = Sprite::createWithSpriteFrameName("bar");

		Size cs(g_screenSize);
		float width = (cs.width - bar->getContentSize().width) / 2;
		float height = thumb->getContentSize().height;
		if (levi->getContentSize().height > height) height = levi->getContentSize().height;
		if (desni->getContentSize().height > height) height = desni->getContentSize().height;
		_height = height;

		height /= 2;
		width /= 2;
		levi->setPosition(Vec2(width, height));
		addChild(levi);

		desni->setPosition(Vec2(cs.width - width, height));
		addChild(desni);

		Vec2 pos(cs.width / 2, height);
		bar->setPosition(pos);
		addChild(bar);
		if (range) {
			thumb2 = Sprite::createWithSpriteFrameName("vanzemljak2");
			rangebar = Sprite::createWithSpriteFrameName("whitebar");
			rangebar->setOpacity(160);
			rangebar->setAnchorPoint(Vec2(0, 0.5f));
			pos.x -= bar->getContentSize().width / 2;
			rangebar->setPosition(pos);
			thumb->setPosition(pos);
			pos.x += bar->getContentSize().width;
			thumb2->setPosition(pos);
			//thumb2->setScale(2.0f);
			_pos2 = 100;
			_pos = -100;
			rangebar->setScaleX(bar->getContentSize().width);
			addChild(rangebar);
			addChild(thumb2, 1);
		}
		else {
			thumb->setPosition(pos);
			thumb2 = 0;
			rangebar = 0;
		}
		//thumb->setScale(2.0f);
		addChild(thumb, 1);

		prstac = ParticleSystemQuad::create("thum_cloud.plist");
		prstac->stopSystem();
		prstac->setPosition(pos);
		auto p = this->getParent(); //get dummy layer
		CCAssert(p, "This object has to be added as child to Layer object before this function call!");
		p->addChild(prstac, -1);
		//prstac->setPositionType(ParticleSystem::PositionType::FREE);
		//prstac->runAction(Follow::create(thumb));
	}

public:
	void setPos(int newpos, bool animate = true)
	{
		if (newpos < -100) newpos = -100;
		else if (newpos > 100) newpos = 100;
		if (thumb2 && newpos > _pos2) newpos = _pos2;
		_pos = newpos;
		float w = bar->getContentSize().width / 2;
		Vec2 tpos(bar->getPosition().x + ((float)_pos * w / 100.0), thumb->getPosition().y);
		if (animate) {
			thumb->runAction(MoveTo::create(0.1f, tpos));
			if (rangebar)
				rangebar->runAction(Spawn::createWithTwoActions(
					MoveTo::create(0.1f, tpos), ScaleTo::create(0.1f, thumb2->getPosition().x - tpos.x, 1.0f)));
		}
		else {
			thumb->setPosition(tpos);
			if (rangebar) {
				rangebar->setPosition(tpos);
				rangebar->setScaleX(thumb2->getPosition().x - tpos.x);
			}
		}
	}

	void setPos2(int newpos, bool animate = true)
	{
		if (!thumb2) return;
		if (newpos < -100) newpos = -100;
		else if (newpos > 100) newpos = 100;
		if (newpos < _pos) newpos = _pos;
		_pos2 = newpos;
		float w = bar->getContentSize().width / 2;
		Vec2 tpos(bar->getPosition().x + ((float)_pos2 * w / 100.0), thumb2->getPosition().y);
		if (animate) {
			thumb2->runAction(MoveTo::create(0.1f, tpos));
			rangebar->runAction(ScaleTo::create(0.1f, tpos.x - thumb->getPosition().x, 1.0f));
		}
		else {
			thumb2->setPosition(tpos);
			rangebar->setScaleX(tpos.x - thumb->getPosition().x);
		}
	}

	int getPos() { return _pos; }
	int getPos2() { return _pos2; }
};

class Slider : public Layer {
public:
	static Slider *docreate(const char *title, const char *leviName, const char *desniName, ccSliderCallback callback, bool ranged=false)
	{
		auto outer = new (std::nothrow) Slider();
		if (!outer) return 0;
		_SliderBatchSprite *inner = new (std::nothrow) _SliderBatchSprite();
		if (!inner) {
			delete outer;
			return 0;
		}
		inner->autorelease();
		outer->autorelease();
		outer->inner = inner;
		outer->addChild(inner);
		inner->initWithFile("appsprites.png", 30);
		inner->_callback = callback;
		inner->initme(title, leviName, desniName, ranged);
		return outer;
	}

	inline int getPos() { return inner->getPos(); }
	inline int getPos2() { return inner->getPos2(); }

protected:
	Slider() : Layer() {}
	bool init() override 
	{
		return Layer::init();
	}
private:
	_SliderBatchSprite * inner;

};
