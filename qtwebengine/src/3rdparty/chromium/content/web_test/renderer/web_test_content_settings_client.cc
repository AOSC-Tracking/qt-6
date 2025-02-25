// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/web_test/renderer/web_test_content_settings_client.h"

#include "content/public/common/origin_util.h"
#include "content/web_test/common/web_test_runtime_flags.h"
#include "content/web_test/common/web_test_string_util.h"
#include "content/web_test/renderer/test_runner.h"
#include "content/web_test/renderer/web_frame_test_proxy.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "third_party/blink/public/common/thread_safe_browser_interface_broker_proxy.h"
#include "third_party/blink/public/platform/platform.h"
#include "third_party/blink/public/platform/web_url.h"

namespace content {

WebTestContentSettingsClient::WebTestContentSettingsClient(
    WebFrameTestProxy* frame,
    TestRunner* test_runner)
    : RenderFrameObserver(frame), test_runner_(test_runner) {}

WebTestContentSettingsClient::~WebTestContentSettingsClient() = default;

void WebTestContentSettingsClient::OnDestruct() {
  delete this;
}

bool WebTestContentSettingsClient::AllowImage(bool enabled_per_settings,
                                              const blink::WebURL& image_url) {
  bool allowed =
      enabled_per_settings && test_runner_->GetFlags().images_allowed();
  if (test_runner_->GetFlags().dump_web_content_settings_client_callbacks()) {
    auto* frame_proxy = static_cast<WebFrameTestProxy*>(render_frame());
    frame_proxy->GetWebTestControlHostRemote()->PrintMessage(
        std::string("WebTestContentSettingsClient: allowImage(") +
        web_test_string_util::NormalizeWebTestURLForTextOutput(
            image_url.GetString().Utf8()) +
        "): " + (allowed ? "true" : "false") + "\n");
  }
  return allowed;
}

bool WebTestContentSettingsClient::AllowScriptFromSource(
    bool enabled_per_settings,
    const blink::WebURL& script_url) {
  bool allowed = enabled_per_settings;
  if (test_runner_->GetFlags().dump_web_content_settings_client_callbacks()) {
    auto* frame_proxy = static_cast<WebFrameTestProxy*>(render_frame());
    frame_proxy->GetWebTestControlHostRemote()->PrintMessage(
        std::string("WebTestContentSettingsClient: allowScriptFromSource(") +
        web_test_string_util::NormalizeWebTestURLForTextOutput(
            script_url.GetString().Utf8()) +
        "): " + (allowed ? "true" : "false") + "\n");
  }
  return allowed;
}

bool WebTestContentSettingsClient::AllowStorageAccessSync(
    StorageType storage_type) {
  return test_runner_->GetFlags().storage_allowed();
}

bool WebTestContentSettingsClient::AllowRunningInsecureContent(
    bool enabled_per_settings,
    const blink::WebURL& url) {
  return enabled_per_settings ||
         test_runner_->GetFlags().running_insecure_content_allowed();
}

bool WebTestContentSettingsClient::IncreaseViewTransitionCallbackTimeout()
    const {
  // In tests we want larger timeout to account for slower running tests.
  return true;
}

}  // namespace content
