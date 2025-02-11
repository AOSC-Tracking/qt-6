// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/libfuzzer/renderer_fuzzing/renderer_fuzzing.h"

#include <map>

#include "base/memory/singleton.h"

void RendererFuzzing::Run(
    const blink::BrowserInterfaceBrokerProxy* context_interface_broker_proxy,
    blink::ThreadSafeBrowserInterfaceBrokerProxy*
        process_interface_broker_proxy,
    const std::string& fuzzer_id,
    std::vector<uint8_t>&& input,
    base::OnceClosure done_closure) {
  auto it = RendererFuzzing::GetInstance()->fuzzers_.find(fuzzer_id);
  CHECK(it != std::end(RendererFuzzing::GetInstance()->fuzzers_));
  it->second->Run(context_interface_broker_proxy,
                  process_interface_broker_proxy, std::move(input),
                  std::move(done_closure));
}

RendererFuzzing* RendererFuzzing::GetInstance() {
  return base::Singleton<RendererFuzzing>::get();
}
