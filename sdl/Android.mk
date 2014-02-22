LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main

SDL_PATH := ../SDL

LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(SDL_PATH)/include

LOCAL_CFLAGS := -std=c99

# Add your application source files here...
LOCAL_SRC_FILES := ../fractal-memory/jni/SDL/src/main/android/SDL_android_main.c \
	game.c debug.c fractal-generator.c fractal-simulator.c \
	secure-random.c sha512.c single-histogram-game.c

LOCAL_SHARED_LIBRARIES := SDL2

LOCAL_LDLIBS := -lGLESv1_CM -llog

include $(BUILD_SHARED_LIBRARY)
