LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_CPP_EXTENSION := .cpp .cc
ifeq ($(TARGET_ARCH_ABI), armeabi-v7a)
	LOCAL_MODULE := SAShaderL
else
	LOCAL_MODULE := SAShaderL64
endif
LOCAL_SRC_FILES := main.cpp mod/logger.cpp
LOCAL_CFLAGS += -O2 -mfloat-abi=softfp -DNDEBUG -std=c++14
LOCAL_LDLIBS += -llog
include $(BUILD_SHARED_LIBRARY)
