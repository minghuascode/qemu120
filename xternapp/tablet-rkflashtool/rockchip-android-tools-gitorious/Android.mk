LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := rkdecrypt.c

LOCAL_MODULE := rkdecrypt
LOCAL_MODULE_TAGS := eng

include $(BUILD_HOST_EXECUTABLE)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := rkcrc.c

LOCAL_MODULE := rkcrc
LOCAL_MODULE_TAGS := eng

include $(BUILD_HOST_EXECUTABLE)

$(call dist-for-goals,droid,$(LOCAL_BUILT_MODULE))

include $(CLEAR_VARS)

LOCAL_SRC_FILES := rkpack.c

LOCAL_MODULE := rkpack
LOCAL_MODULE_TAGS := eng

include $(BUILD_HOST_EXECUTABLE)

$(call dist-for-goals,droid,$(LOCAL_BUILT_MODULE))

include $(CLEAR_VARS)

LOCAL_SRC_FILES := rkunpack.c

LOCAL_MODULE := rkunpack
LOCAL_MODULE_TAGS := eng

include $(BUILD_HOST_EXECUTABLE)

$(call dist-for-goals,droid,$(LOCAL_BUILT_MODULE))

include $(CLEAR_VARS)

LOCAL_SRC_FILES := rkflash.c

LOCAL_MODULE := rkflash
LOCAL_MODULE_TAGS := eng

LOCAL_LDLIBS += -lusb

include $(BUILD_HOST_EXECUTABLE)

$(call dist-for-goals,droid,$(LOCAL_BUILT_MODULE))

include $(CLEAR_VARS)

LOCAL_SRC_FILES := afptool.c

LOCAL_MODULE := afptool
LOCAL_MODULE_TAGS := eng

include $(BUILD_HOST_EXECUTABLE)

$(call dist-for-goals,droid,$(LOCAL_BUILT_MODULE))
