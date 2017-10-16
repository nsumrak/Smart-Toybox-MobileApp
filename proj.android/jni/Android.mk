LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

$(call import-add-path,$(LOCAL_PATH)/../../cocos2d)
$(call import-add-path,$(LOCAL_PATH)/../../cocos2d/external)
$(call import-add-path,$(LOCAL_PATH)/../../cocos2d/cocos)

LOCAL_MODULE := cocos2dcpp_shared

LOCAL_MODULE_FILENAME := libcocos2dcpp

LOCAL_SRC_FILES := hellocpp/main.cpp \
                   ../../Classes/AppDelegate.cpp \
				   ../../Classes/wavpack/bits.c \
				   ../../Classes/wavpack/pack.c \
				   ../../Classes/wavpack/tinypack.c \
				   ../../Classes/wavpack/words.c \
				   ../../Classes/wavpack/wputils.c \
../../SoundTouch/AAFilter.cpp \
../../SoundTouch/BPMDetect.cpp \
../../SoundTouch/cpu_detect_x86.cpp \
../../SoundTouch/FIFOSampleBuffer.cpp \
../../SoundTouch/FIRFilter.cpp \
../../SoundTouch/InterpolateCubic.cpp \
../../SoundTouch/InterpolateLinear.cpp \
../../SoundTouch/InterpolateShannon.cpp \
../../SoundTouch/mmx_optimized.cpp \
../../SoundTouch/PeakFinder.cpp \
../../SoundTouch/RateTransposer.cpp \
../../SoundTouch/SoundTouch.cpp \
../../SoundTouch/sse_optimized.cpp \
../../SoundTouch/TDStretch.cpp


LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../Classes
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../SoundTouch/include

# _COCOS_HEADER_ANDROID_BEGIN
# _COCOS_HEADER_ANDROID_END


LOCAL_STATIC_LIBRARIES := cocos2dx_static
LOCAL_STATIC_LIBRARIES += soundtouch_static

# _COCOS_LIB_ANDROID_BEGIN
# _COCOS_LIB_ANDROID_END

include $(BUILD_SHARED_LIBRARY)

$(call import-module,.)

# _COCOS_LIB_IMPORT_ANDROID_BEGIN
# _COCOS_LIB_IMPORT_ANDROID_END

