// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/capture/video/chromeos/display_rotation_observer.h"

#include <utility>

#include "base/functional/bind.h"
#include "base/task/single_thread_task_runner.h"

namespace media {

// static
scoped_refptr<ScreenObserverDelegate> ScreenObserverDelegate::Create(
    base::WeakPtr<DisplayRotationObserver> observer,
    scoped_refptr<base::SingleThreadTaskRunner> display_task_runner) {
  auto delegate = base::WrapRefCounted(
      new ScreenObserverDelegate(std::move(observer), display_task_runner));
  display_task_runner->PostTask(
      FROM_HERE,
      base::BindOnce(&ScreenObserverDelegate::AddObserverOnDisplayThread,
                     delegate));
  return delegate;
}

ScreenObserverDelegate::ScreenObserverDelegate(
    base::WeakPtr<DisplayRotationObserver> observer,
    scoped_refptr<base::SingleThreadTaskRunner> display_task_runner)
    : observer_(std::move(observer)),
      display_task_runner_(std::move(display_task_runner)),
      delegate_task_runner_(base::SingleThreadTaskRunner::GetCurrentDefault()) {
}

void ScreenObserverDelegate::RemoveObserver() {
  DCHECK(delegate_task_runner_->BelongsToCurrentThread());
  observer_ = nullptr;
  display_task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(&ScreenObserverDelegate::RemoveObserverOnDisplayThread,
                     this));
}

ScreenObserverDelegate::~ScreenObserverDelegate() {
  DCHECK(!observer_);
}

void ScreenObserverDelegate::OnDisplayMetricsChanged(
    const display::Display& display,
    uint32_t metrics) {
  DCHECK(display_task_runner_->BelongsToCurrentThread());
  if (!(metrics & DISPLAY_METRIC_ROTATION))
    return;
  if (display.IsInternal()) {
    SendInternalDisplayRotation(display.rotation() * 90);
  }
}

void ScreenObserverDelegate::AddObserverOnDisplayThread() {
  DCHECK(display_task_runner_->BelongsToCurrentThread());
  display::Screen* screen = display::Screen::GetScreen();
  if (!screen) {
    return;
  }
  display_observer_.emplace(this);
  for (const auto& display : screen->GetAllDisplays()) {
    if (display.IsInternal()) {
      SendInternalDisplayRotation(display.rotation() * 90);
      break;
    }
  }
}

void ScreenObserverDelegate::RemoveObserverOnDisplayThread() {
  DCHECK(display_task_runner_->BelongsToCurrentThread());
  display_observer_.reset();
}

// Post the screen rotation change from the UI thread to capture thread
void ScreenObserverDelegate::SendInternalDisplayRotation(int rotation) {
  DCHECK(display_task_runner_->BelongsToCurrentThread());
  delegate_task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(
          &ScreenObserverDelegate::SendInternalDisplayRotationOnCaptureThread,
          this, rotation));
}

void ScreenObserverDelegate::SendInternalDisplayRotationOnCaptureThread(
    int rotation) {
  DCHECK(delegate_task_runner_->BelongsToCurrentThread());
  if (observer_)
    observer_->SetInternalDisplayRotation(rotation);
}

}  // namespace media
