// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_WEB_CONTENTS_WEB_CONTENTS_VIEW_ANDROID_H_
#define CONTENT_BROWSER_WEB_CONTENTS_WEB_CONTENTS_VIEW_ANDROID_H_

#include <memory>

#include "base/memory/raw_ptr.h"
#include "content/browser/renderer_host/render_view_host_delegate_view.h"
#include "content/browser/web_contents/web_contents_view.h"
#include "content/public/browser/web_contents_view_delegate.h"
#include "content/public/common/drop_data.h"
#include "mojo/public/cpp/bindings/pending_remote.h"
#include "third_party/blink/public/mojom/choosers/popup_menu.mojom.h"
#include "ui/android/overscroll_refresh.h"
#include "ui/android/view_android.h"
#include "ui/android/view_android_observer.h"
#include "ui/events/android/event_handler_android.h"
#include "ui/gfx/geometry/rect_f.h"

namespace content {

class BackForwardTransitionAnimationManagerAndroid;
class ContentUiEventHandler;
class RenderWidgetHostViewAndroid;
class SelectPopup;
class SelectionPopupController;
class SynchronousCompositorClient;
class WebContentsImpl;

// Android-specific implementation of the WebContentsView.
class WebContentsViewAndroid : public WebContentsView,
                               public RenderViewHostDelegateView,
                               public ui::EventHandlerAndroid {
 public:
  WebContentsViewAndroid(WebContentsImpl* web_contents,
                         std::unique_ptr<WebContentsViewDelegate> delegate);

  WebContentsViewAndroid(const WebContentsViewAndroid&) = delete;
  WebContentsViewAndroid& operator=(const WebContentsViewAndroid&) = delete;

  ~WebContentsViewAndroid() override;

  void SetContentUiEventHandler(std::unique_ptr<ContentUiEventHandler> handler);

  void set_synchronous_compositor_client(SynchronousCompositorClient* client) {
    synchronous_compositor_client_ = client;
  }

  SynchronousCompositorClient* synchronous_compositor_client() const {
    return synchronous_compositor_client_;
  }

  void SetOverscrollRefreshHandler(
      std::unique_ptr<ui::OverscrollRefreshHandler> overscroll_refresh_handler);

  RenderWidgetHostViewAndroid* GetRenderWidgetHostViewAndroid();

  // WebContentsView implementation --------------------------------------------
  gfx::NativeView GetNativeView() const override;
  gfx::NativeView GetContentNativeView() const override;
  gfx::NativeWindow GetTopLevelNativeWindow() const override;
  gfx::Rect GetContainerBounds() const override;
  void Focus() override;
  void SetInitialFocus() override;
  void StoreFocus() override;
  void RestoreFocus() override;
  void FocusThroughTabTraversal(bool reverse) override;
  DropData* GetDropData() const override;
  void TransferDragSecurityInfo(WebContentsView* view) override;
  gfx::Rect GetViewBounds() const override;
  void CreateView(gfx::NativeView context) override;
  RenderWidgetHostViewBase* CreateViewForWidget(
      RenderWidgetHost* render_widget_host) override;
  RenderWidgetHostViewBase* CreateViewForChildWidget(
      RenderWidgetHost* render_widget_host) override;
  void SetPageTitle(const std::u16string& title) override;
  void RenderViewReady() override;
  void RenderViewHostChanged(RenderViewHost* old_host,
                             RenderViewHost* new_host) override;
  void SetOverscrollControllerEnabled(bool enabled) override;
  void OnCapturerCountChanged() override;
  void FullscreenStateChanged(bool is_fullscreen) override;
  void UpdateWindowControlsOverlay(const gfx::Rect& bounding_rect) override;

  // Backend implementation of RenderViewHostDelegateView.
  void ShowContextMenu(RenderFrameHost& render_frame_host,
                       const ContextMenuParams& params) override;
  void ShowPopupMenu(
      RenderFrameHost* render_frame_host,
      mojo::PendingRemote<blink::mojom::PopupMenuClient> popup_client,
      const gfx::Rect& bounds,
      int item_height,
      double item_font_size,
      int selected_item,
      std::vector<blink::mojom::MenuItemPtr> menu_items,
      bool right_aligned,
      bool allow_multiple_selection) override;
  ui::OverscrollRefreshHandler* GetOverscrollRefreshHandler() const override;
  void StartDragging(const DropData& drop_data,
                     const url::Origin& source_origin,
                     blink::DragOperationsMask allowed_ops,
                     const gfx::ImageSkia& image,
                     const gfx::Vector2d& cursor_offset,
                     const gfx::Rect& drag_obj_rect,
                     const blink::mojom::DragEventSourceInfo& event_info,
                     RenderWidgetHostImpl* source_rwh) override;
  void UpdateDragOperation(ui::mojom::DragOperation operation,
                           bool document_is_handling_drag) override;
  void GotFocus(RenderWidgetHostImpl* render_widget_host) override;
  void LostFocus(RenderWidgetHostImpl* render_widget_host) override;
  void TakeFocus(bool reverse) override;
  int GetTopControlsHeight() const override;
  int GetTopControlsMinHeight() const override;
  int GetBottomControlsHeight() const override;
  int GetBottomControlsMinHeight() const override;
  bool ShouldAnimateBrowserControlsHeightChanges() const override;
  bool DoBrowserControlsShrinkRendererSize() const override;
  bool OnlyExpandTopControlsAtPageTop() const override;

  // ui::EventHandlerAndroid implementation.
  bool OnTouchEvent(const ui::MotionEventAndroid& event) override;
  bool OnMouseEvent(const ui::MotionEventAndroid& event) override;
  bool OnDragEvent(const ui::DragEventAndroid& event) override;
  bool OnGenericMotionEvent(const ui::MotionEventAndroid& event) override;
  bool OnKeyUp(const ui::KeyEventAndroid& event) override;
  bool DispatchKeyEvent(const ui::KeyEventAndroid& event) override;
  bool ScrollBy(float delta_x, float delta_y) override;
  bool ScrollTo(float x, float y) override;
  void OnSizeChanged() override;
  void OnPhysicalBackingSizeChanged(
      std::optional<base::TimeDelta> deadline_override) override;
  void OnBrowserControlsHeightChanged() override;
  void OnControlsResizeViewChanged() override;
  void NotifyVirtualKeyboardOverlayRect(
      const gfx::Rect& keyboard_rect) override;

  void SetFocus(bool focused);
  void set_device_orientation(int orientation) {
    device_orientation_ = orientation;
  }

  // Insert `screenshot_layer` into the layer tree, as a *direct* sibling of
  // `parent_for_web_page_widgets_`.
  //
  // `screenshot_layer_on_top` controls the position of `screenshot_layer`:
  // `true` means the screenshot will be placed right above
  // `parent_for_web_page_widgets_`; `false` means right below it.
  //
  // TODO(crbug/1488075): The boolean might not be enough if
  // `parent_for_web_page_widgets_` has more siblings, and we need finer control
  // of the position.
  void AddScreenshotLayerForNavigationTransitions(
      scoped_refptr<cc::slim::Layer> screenshot_layer,
      bool screenshot_layer_on_top);

  // See the block comments above `parent_for_web_page_widgets_` for the
  // hierarchies of layers and native views. The callers can operate upon all
  // the web widgets and the web page via this getter.
  cc::slim::Layer* parent_for_web_page_widgets() const {
    return parent_for_web_page_widgets_.get();
  }

  WebContentsImpl* web_contents() { return web_contents_; }

  // Guaranteed non-null if `features::kBackForwardTransitions` is enabled.
  BackForwardTransitionAnimationManagerAndroid*
  back_forward_animation_manager();

 private:
  void OnDragEntered(const std::vector<DropData::Metadata>& metadata,
                     const gfx::PointF& location,
                     const gfx::PointF& screen_location);
  void OnDragUpdated(const gfx::PointF& location,
                     const gfx::PointF& screen_location);
  void OnDragExited();
  void OnPerformDrop(DropData* drop_data,
                     const gfx::PointF& location,
                     const gfx::PointF& screen_location);
  void OnDragEnded();
  void OnSystemDragEnded();

  SelectPopup* GetSelectPopup();

  // Returns the current `SelectionPopupController` from the current
  // `RenderWidgetHostViewAndroid`.
  SelectionPopupController* GetSelectionPopupController();

  // The WebContents whose contents we display.
  raw_ptr<WebContentsImpl> web_contents_;

  // Handles UI events in Java layer when necessary.
  std::unique_ptr<ContentUiEventHandler> content_ui_event_handler_;

  // Handles "overscroll to refresh" events
  std::unique_ptr<ui::OverscrollRefreshHandler> overscroll_refresh_handler_;

  // Interface for extensions to WebContentsView. Used to show the context menu.
  std::unique_ptr<WebContentsViewDelegate> delegate_;

  // The native view associated with the contents of the web.
  ui::ViewAndroid view_;

  // A common parent to all the native widgets as part of a web page.
  //
  // Layer hierarchy:
  // `view_`
  //   |
  //   |- `parent_for_web_page_widgets_`
  //   |                |
  //   |                |- RenderWidgetHostViewAndroid
  //   |                |- Overscroll
  //   |                |- SelectionHandle
  //   |
  //   |- `NavigationEntryScreenshot`  // TODO(https://crbug.com/1509888)
  //
  // ViewAndroid hierarchy:
  // `view_`
  //   |
  //   |- `RenderWidgetHostViewAndroid`
  scoped_refptr<cc::slim::Layer> parent_for_web_page_widgets_;

  // Interface used to get notified of events from the synchronous compositor.
  raw_ptr<SynchronousCompositorClient> synchronous_compositor_client_;

  int device_orientation_ = 0;

  // Show/hide popup UI for <select> tag.
  std::unique_ptr<SelectPopup> select_popup_;

  // Whether drag went beyond the movement threshold to be considered as an
  // intentional drag. If true, ::ShowContextMenu will be ignored.
  bool drag_exceeded_movement_threshold_ = false;
  // Whether there's an active drag process.
  bool is_active_drag_ = false;
  // The first drag location during a specific drag process.
  gfx::PointF drag_entered_location_;

  gfx::PointF drag_location_;
  gfx::PointF drag_screen_location_;

  // Set to true when the document is handling the drag.  This means that
  // the document has registeted interest in the dropped data and the
  // renderer process should pass the data to the document on drop.
  bool document_is_handling_drag_ = false;

  // Manages the animation during a session history navigation.
  std::unique_ptr<BackForwardTransitionAnimationManagerAndroid>
      back_forward_animation_manager_;
};

} // namespace content

#endif  // CONTENT_BROWSER_WEB_CONTENTS_WEB_CONTENTS_VIEW_ANDROID_H_
