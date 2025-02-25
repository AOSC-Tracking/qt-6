// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/android/memory_pressure_listener_android.h"

#include "base/android/pre_freeze_background_memory_trimmer.h"
#include "base/base_jni/MemoryPressureListener_jni.h"
#include "base/memory/memory_pressure_listener.h"

using base::android::JavaParamRef;

// Defined and called by JNI.
static void JNI_MemoryPressureListener_OnMemoryPressure(
    JNIEnv* env,
    jint memory_pressure_level) {
  base::MemoryPressureListener::NotifyMemoryPressure(
      static_cast<base::MemoryPressureListener::MemoryPressureLevel>(
          memory_pressure_level));
}

static void JNI_MemoryPressureListener_OnPreFreeze(JNIEnv* env) {
  base::PreFreezeBackgroundMemoryTrimmer::OnPreFreeze();
}

namespace base::android {

void MemoryPressureListenerAndroid::Initialize(JNIEnv* env) {
  Java_MemoryPressureListener_addNativeCallback(env);
}

}  // namespace base::android
