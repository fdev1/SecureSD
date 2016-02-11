#APP_ABI := armeabi x86
APP_ABI := armeabi
MY_APP_PATH := $(call my-dir)
NDK_PROJECT_PATH := $(MY_APP_PATH)/../../../build
NDK_APP_OUT := $(MY_APP_PATH)/../../../build
NDK_OUT := $(MY_APP_PATH)/../../../build
APP_BUILD_SCRIPT := $(MY_APP_PATH)/../../../src/main/jni/Android.mk
NDK_APPLICATION_MK := $(MY_APP_PATH)/../../../src/main/jni/Application.mk
