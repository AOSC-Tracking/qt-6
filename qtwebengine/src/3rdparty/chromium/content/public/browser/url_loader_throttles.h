// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_BROWSER_URL_LOADER_THROTTLES_H_
#define CONTENT_PUBLIC_BROWSER_URL_LOADER_THROTTLES_H_

#include <vector>

#include "base/functional/callback.h"
#include "content/common/content_export.h"
#include "third_party/abseil-cpp/absl/types/optional.h"

namespace blink {
class URLLoaderThrottle;
}  // namespace blink

namespace network {
struct ResourceRequest;
}  // namespace network

namespace content {

class BrowserContext;
class NavigationUIData;
class WebContents;

// Wrapper around ContentBrowserClient::CreateURLLoaderThrottles which inserts
// additional content specific throttles.
CONTENT_EXPORT
std::vector<std::unique_ptr<blink::URLLoaderThrottle>>
CreateContentBrowserURLLoaderThrottles(
    const network::ResourceRequest& request,
    BrowserContext* browser_context,
    const base::RepeatingCallback<WebContents*()>& wc_getter,
    NavigationUIData* navigation_ui_data,
    int frame_tree_node_id,
    absl::optional<int64_t> navigation_id);

// Wrapper around `ContentBrowserClient::CreateURLLoaderThrottlesForKeepAlive()`
// which inserts additional content specific throttles for handling fetch
// keepalive requests when their initiator is destroyed.
CONTENT_EXPORT
std::vector<std::unique_ptr<blink::URLLoaderThrottle>>
CreateContentBrowserURLLoaderThrottlesForKeepAlive(
    const network::ResourceRequest& request,
    BrowserContext* browser_context,
    const base::RepeatingCallback<WebContents*()>& wc_getter,
    int frame_tree_node_id);

}  // namespace content

#endif  // CONTENT_PUBLIC_BROWSER_URL_LOADER_THROTTLES_H_
