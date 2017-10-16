#include "AppDelegate.h"
#include "cocos2d.h"
#include "platform/android/jni/JniHelper.h"
#include <jni.h>
#include <android/log.h>
#include <wavpack\wavpack.h>

#define  LOG_TAG    "main"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)

using namespace cocos2d;

void cocos_android_app_init (JNIEnv* env) {
    LOGD("cocos_android_app_init");
    AppDelegate *pAppDelegate = new AppDelegate();
}


bool native_startAudioRec()
{
	JniMethodInfo t;
	bool res = false;

	LOGD("JNI resolve recordAudio");
	if (JniHelper::getStaticMethodInfo(t, "org/cocos2dx/cpp/AppActivity", "recordAudio", "()I")) {
		LOGD("JNI resolved recordAudio, call");
		jint i = t.env->CallStaticIntMethod(t.classID, t.methodID);
		LOGD("JNI recordAudio returned %d", i);
		if (!i) res = true;
		t.env->DeleteLocalRef(t.classID);
	}

	return res;
}

int native_stopAudioRec()
{
	JniMethodInfo t;

	LOGD("JNI resolve stopRecording");
	if (JniHelper::getStaticMethodInfo(t, "org/cocos2dx/cpp/AppActivity", "stopRecording", "()I")) {
		LOGD("JNI resolved stopRecording, call");
		jint i = t.env->CallStaticIntMethod(t.classID, t.methodID);
		LOGD("JNI stopRecording returned %d", i);
		t.env->DeleteLocalRef(t.classID);
		return i;
	}
	return 0;
}

bool native_getRecBuffer(short *buf, int size)
{
	JniMethodInfo t2;
	bool res = false;

	LOGD("JNI resolve getRecBuffer");
	if (JniHelper::getStaticMethodInfo(t2, "org/cocos2dx/cpp/AppActivity", "getRecBuffer", "()[S")) {
		LOGD("JNI resolved getRecBuffer, call");
		jshortArray arr = (jshortArray)t2.env->CallStaticObjectMethod(t2.classID, t2.methodID);
		if (arr) {
			LOGD("JNI getRecBuffer returned aRR");
			t2.env->GetShortArrayRegion(arr, 0, size, buf);
			res = true;
			//jshort *data = t2.env->GetShortArrayElements(arr, NULL);
			//if (data != NULL) {
			//	LOGD("JNI getRecBuffer got data from arr");
			//	// use here @data in size @i
			//	pack_buffer((char*)data, i, "/sdcard/Prenos/output.wv");
			//	// release
			//	t2.env->ReleaseShortArrayElements(arr, data, JNI_ABORT);
			//	res = true;
			//}
		}
		t2.env->DeleteLocalRef(t2.classID);
	}
	return res;
}

bool native_playBuffer(short *buf, int size)
{
	JniMethodInfo t;
	bool res = false;

	LOGD("JNI resolve playAudio");
	if (JniHelper::getStaticMethodInfo(t, "org/cocos2dx/cpp/AppActivity", "playAudio", "([S)I")) {
		LOGD("JNI resolved playAudio, copy buffer");
		jshortArray arr = t.env->NewShortArray(size);
		if (arr) {
			t.env->SetShortArrayRegion(arr, 0, size, buf);
			LOGD("JNI cal playAudio");
			jint i = t.env->CallStaticIntMethod(t.classID, t.methodID, arr);
			if (!i) res = true;
			LOGD("JNI playAudio returned %d", i);
		}
		t.env->DeleteLocalRef(t.classID);
	}

	return res;
}

void native_stopPlay()
{
	JniMethodInfo t;

	LOGD("JNI resolve stopAudio");
	if (JniHelper::getStaticMethodInfo(t, "org/cocos2dx/cpp/AppActivity", "stopAudioPlay", "()V")) {
		LOGD("JNI resolved stopAudio, call");
		t.env->CallStaticVoidMethod(t.classID, t.methodID);
		LOGD("JNI stopAudio");
		t.env->DeleteLocalRef(t.classID);
	}
}



void native_startDnsSD()
{
	JniMethodInfo t;

//	LOGD("JNI resolve startDnsSD");
	if (JniHelper::getStaticMethodInfo(t, "org/cocos2dx/cpp/AppActivity", "startDnsSD", "()V")) {
//		LOGD("JNI resolved startDnsSD, call");
		t.env->CallStaticVoidMethod(t.classID, t.methodID);
//		LOGD("JNI startDnsSD");
		t.env->DeleteLocalRef(t.classID);
	}
}


int native_getDnsSDdiscoveryNum()
{
	JniMethodInfo t;

	LOGD("JNI resolve getDnsSDdiscoveryNum");
	if (JniHelper::getStaticMethodInfo(t, "org/cocos2dx/cpp/AppActivity", "getDnsSDdiscoveryNum", "()I")) {
//		LOGD("JNI resolved getDnsSDdiscoveryNum, call");
		jint i = t.env->CallStaticIntMethod(t.classID, t.methodID);
//		LOGD("JNI getDnsSDdiscoveryNum returned %d", i);
		t.env->DeleteLocalRef(t.classID);
		return i;
	}
	return 0;
}


void native_getDnsSDdiscoveryItem(int i, char *buf, int bsize)
{
	JniMethodInfo t;

	LOGD("JNI resolve getDnsSDdiscoveryItem");
	if (JniHelper::getStaticMethodInfo(t, "org/cocos2dx/cpp/AppActivity", "getDnsSDdiscoveryItem", "(I)Ljava/lang/String;")) {
//		LOGD("JNI resolved getDnsSDdiscoveryItem, call");
		jstring jstr = (jstring)t.env->CallStaticObjectMethod(t.classID, t.methodID, i);
		const char *str = t.env->GetStringUTFChars(jstr, 0);
		if(!str) LOGD("JNI getDnsSDdiscoveryItem returned NULL!");
		else {
			const char *s = str;
			while (*s == '/') s++;
			strncpy(buf, s, bsize);
			buf[bsize-1] = 0;
			LOGD("JNI getDnsSDdiscoveryItem returned %s", str);
			t.env->ReleaseStringUTFChars(jstr, str);
		}
		t.env->DeleteLocalRef(t.classID);
	}
}


void native_stopDnsSD()
{
	JniMethodInfo t;

	LOGD("JNI resolve stopDnsSD");
	if (JniHelper::getStaticMethodInfo(t, "org/cocos2dx/cpp/AppActivity", "stopDnsSD", "()V")) {
		LOGD("JNI resolved stopDnsSD, call");
		t.env->CallStaticVoidMethod(t.classID, t.methodID);
		LOGD("JNI stopDnsSD");
		t.env->DeleteLocalRef(t.classID);
	}
}

void native_startSmartConfig(const char *ssid, const char *pwd)
{
	JniMethodInfo t;

	LOGD("JNI resolve startSmartConfig");
	if (JniHelper::getStaticMethodInfo(t, "org/cocos2dx/cpp/AppActivity", "startSmartConfig", "(Ljava/lang/String;Ljava/lang/String;)V")) {
		LOGD("JNI resolved startSmartConfig, call");
		jstring jssid = t.env->NewStringUTF(ssid);
		jstring jpwd = t.env->NewStringUTF(pwd);
		if (jssid && pwd) {
			t.env->CallStaticVoidMethod(t.classID, t.methodID, jssid, jpwd);
			LOGD("JNI startSmartConfig");
		}
		t.env->DeleteLocalRef(t.classID);
	}
}

void native_stopSmartConfig()
{
	JniMethodInfo t;

	LOGD("JNI resolve stopSmartConfig");
	if (JniHelper::getStaticMethodInfo(t, "org/cocos2dx/cpp/AppActivity", "stopSmartConfig", "()V")) {
		LOGD("JNI resolved stopSmartConfig, call");
		t.env->CallStaticVoidMethod(t.classID, t.methodID);
		LOGD("JNI stopSmartConfig");
		t.env->DeleteLocalRef(t.classID);
	}
}

void native_getSSID(char *buf, unsigned bsize)
{
	JniMethodInfo t;
	if (!buf || !bsize) {
		LOGD("JNI getSSID bad params!");
	}

	LOGD("JNI resolve getSSID");
	if (JniHelper::getStaticMethodInfo(t, "org/cocos2dx/cpp/AppActivity", "getSSID", "()Ljava/lang/String;")) {
		LOGD("JNI resolved getSSID, call");
		jstring jstr = (jstring)t.env->CallStaticObjectMethod(t.classID, t.methodID);
		const char *str = jstr ? t.env->GetStringUTFChars(jstr, 0) : 0;
		if(!str) {
			buf[0] = 0;
			LOGD("JNI getSSID returned NULL!");
		} else {
			strncpy(buf, str, bsize);
			buf[bsize-1] = 0;
			LOGD("JNI getSSID returned %s", str);
			t.env->ReleaseStringUTFChars(jstr, str);
		}
		t.env->DeleteLocalRef(t.classID);
	}
}
