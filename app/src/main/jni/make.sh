#!/bin/sh

ndk-build \
	NDK_PROJECT_PATH=../../../build \
	APP_BUILD_SCRIPT=../../../src/main/jni/Android.mk \
	NDK_APPLICATION_MK=../../../src/main/jni/Application.mk
