// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_RENDERER_ACCESSIBILITY_RENDER_ACCESSIBILITY_IMPL_H_
#define CONTENT_RENDERER_ACCESSIBILITY_RENDER_ACCESSIBILITY_IMPL_H_

#include <list>
#include <memory>
#include <vector>

#include "base/memory/raw_ptr.h"
#include "base/memory/raw_ptr_exclusion.h"
#include "base/memory/weak_ptr.h"
#include "base/time/time.h"
#include "content/common/content_export.h"
#include "content/public/renderer/plugin_ax_tree_source.h"
#include "content/public/renderer/render_accessibility.h"
#include "content/public/renderer/render_frame.h"
#include "content/public/renderer/render_frame_observer.h"
#include "third_party/blink/public/mojom/render_accessibility.mojom.h"
#include "third_party/blink/public/web/web_ax_context.h"
#include "third_party/blink/public/web/web_ax_object.h"
#include "ui/accessibility/ax_event.h"
#include "ui/accessibility/ax_node_data.h"
#include "ui/accessibility/ax_relative_bounds.h"
#include "ui/accessibility/ax_tree.h"
#include "ui/accessibility/ax_tree_data.h"
#include "ui/accessibility/ax_tree_serializer.h"
#include "ui/accessibility/ax_tree_update.h"
#include "ui/gfx/geometry/rect_f.h"

namespace base {
class ElapsedTimer;
}  // namespace base

namespace blink {
class WebDocument;
}  // namespace blink

namespace ui {

struct AXActionData;
class AXActionTarget;
struct AXEvent;
class AXTreeID;

}  // namespace ui

namespace ukm {
class MojoUkmRecorder;
}

namespace content {

class AXImageAnnotator;
class RenderFrameImpl;
class RenderAccessibilityManager;

// The browser process implements native accessibility APIs, allowing assistive
// technology (e.g., screen readers, magnifiers) to access and control the web
// contents with high-level APIs. These APIs are also used by automation tools,
// and Windows 8 uses them to determine when the on-screen keyboard should be
// shown.
//
// An instance of this class belongs to the RenderAccessibilityManager object.
// Accessibility is initialized based on the ui::AXMode passed from the browser
// process to the manager object; it lazily starts as Off or EditableTextOnly
// depending on the operating system, and switches to Complete if assistive
// technology is detected or a flag is set.
//
// A tree of accessible objects is built here and sent to the browser process;
// the browser process maintains this as a tree of platform-native accessible
// objects that can be used to respond to accessibility requests from other
// processes.
//
// This class implements complete accessibility support for assistive
// technology. It turns on Blink's accessibility code and sends a serialized
// representation of that tree whenever it changes. It also handles requests
// from the browser to perform accessibility actions on nodes in the tree (e.g.,
// change focus, or click on a button).
class CONTENT_EXPORT RenderAccessibilityImpl : public RenderAccessibility,
                                               public RenderFrameObserver {
 public:
  // A call to AccessibilityModeChanged() is required after construction to
  // start accessibility.
  RenderAccessibilityImpl(
      RenderAccessibilityManager* const render_accessibility_manager,
      RenderFrameImpl* const render_frame);

  RenderAccessibilityImpl(const RenderAccessibilityImpl&) = delete;
  RenderAccessibilityImpl& operator=(const RenderAccessibilityImpl&) = delete;

  ~RenderAccessibilityImpl() override;

  ui::AXMode GetAccessibilityMode() { return accessibility_mode_; }

  // RenderAccessibility implementation.
  bool HasActiveDocument() const override;
  int GenerateAXID() override;
  ui::AXMode GetAXMode() const override;
  ui::AXTreeID GetTreeIDForPluginHost() const override;
  void SetPluginTreeSource(PluginAXTreeSource* source) override;
  void OnPluginRootNodeUpdated() override;
  void ShowPluginContextMenu() override;
  void RecordInaccessiblePdfUkm() override;

  // RenderFrameObserver implementation.
  void DidCreateNewDocument() override;
  void DidCommitProvisionalLoad(ui::PageTransition transition) override;
  void AccessibilityModeChanged(const ui::AXMode& mode) override;

  void HitTest(const gfx::Point& point,
               ax::mojom::Event event_to_fire,
               int request_id,
               blink::mojom::RenderAccessibility::HitTestCallback callback);
  void PerformAction(const ui::AXActionData& data);
  void Reset(uint32_t reset_token);

  // Called when accessibility mode changes so that any obsolete accessibility
  // bundles for the old mode can be ignored.
  void set_reset_token(uint32_t reset_token);

  // Called when an accessibility notification occurs in Blink.
  void HandleAXEvent(const ui::AXEvent& event);
  // An AXObject should be serialized at the next available opportunity.
  void MarkWebAXObjectDirty(
      const blink::WebAXObject& obj,
      bool subtree,
      ax::mojom::EventFrom event_from = ax::mojom::EventFrom::kNone,
      ax::mojom::Action event_from_action = ax::mojom::Action::kNone,
      std::vector<ui::AXEventIntent> event_intents = {},
      ax::mojom::Event event_type = ax::mojom::Event::kNone);

  void NotifyWebAXObjectMarkedDirty(const blink::WebAXObject& obj,
    ax::mojom::Event event_type = ax::mojom::Event::kNone);
  // Called when it is safe to begin a serialization.
  void AXReadyCallback();

  // Returns the main top-level document for this page, or NULL if there's
  // no view or frame.
  blink::WebDocument GetMainDocument() const;

  blink::WebAXContext* GetAXContext() { return ax_context_.get(); }

  // Returns the page language.
  std::string GetLanguage();

  // Access the UKM recorder.
  ukm::MojoUkmRecorder* ukm_recorder() const { return ukm_recorder_.get(); }

  // Called when the renderer has closed the connection to reset the state
  // machine.
  void ConnectionClosed();

 protected:
  // Send queued events from the renderer to the browser.
  void SendPendingAccessibilityEvents();

  // Check the entire accessibility tree to see if any nodes have
  // changed location, by comparing their locations to the cached
  // versions. If any have moved, send an IPC with the new locations.
  void SendLocationChanges();

 private:
  // Called whenever the "ack" message is received for a serialization message
  // sent to the browser process, indicating it was received.
  void OnSerializationReceived();

  // RenderFrameObserver implementation.
  void OnDestruct() override;

  // Handlers for messages from the browser to the renderer.
  void OnLoadInlineTextBoxes(const ui::AXActionTarget* target);
  void OnGetImageData(const ui::AXActionTarget* target,
                      const gfx::Size& max_size);
  void AddPluginTreeToUpdate(ui::AXTreeUpdate* update,
                             bool mark_plugin_subtree_dirty);

  // If the document is loaded, fire a load complete event.
  void FireLoadCompleteIfLoaded();

  // Creates and takes ownership of an instance of the class that automatically
  // labels images for accessibility.
  void CreateAXImageAnnotator();

  // Automatically labels images for accessibility if the accessibility mode for
  // this feature is turned on, otherwise stops automatic labeling and removes
  // any automatic annotations that might have been added before.
  void StartOrStopLabelingImages(ui::AXMode old_mode, ui::AXMode new_mode);

  // Marks all AXObjects with the given role in the current tree dirty.
  void MarkAllAXObjectsDirty(ax::mojom::Role role,
                             ax::mojom::Action event_from_action);

  // Ensure that AXReadyCallback() will be called at the next available
  // opportunity, so that any dirty objects will be serialized soon.
  void ScheduleImmediateAXUpdate();

  void AddImageAnnotationDebuggingAttributes(
      const std::vector<ui::AXTreeUpdate>& updates);

  // Returns the document for the active popup if any.
  blink::WebDocument GetPopupDocument();

  // Searches the accessibility tree for plugin's root object and returns it.
  // Returns an empty WebAXObject if no root object is present.
  blink::WebAXObject GetPluginRoot();

  blink::WebAXObject ComputeRoot();

  // Sends the URL-keyed metrics for the maximum amount of time spent in
  // SendPendingAccessibilityEvents if they meet the minimum criteria for
  // sending.
  void MaybeSendUKM();

  // Reset all of the UKM data. This can be called after sending UKM data,
  // or after navigating to a new page when any previous data will no
  // longer be valid.
  void ResetUKMData();

  bool SerializeUpdatesAndEvents(blink::WebDocument document,
                                 blink::WebAXObject root,
                                 std::vector<ui::AXEvent>& events,
                                 std::vector<ui::AXTreeUpdate>& updates,
                                 bool mark_plugin_subtree_dirty);

  void AddImageAnnotations(const blink::WebDocument& document,
                           std::vector<ui::AXNodeData*>&);
  void AddImageAnnotationsForNode(blink::WebAXObject& src, ui::AXNodeData* dst);

  static void IgnoreProtocolChecksForTesting();

  // The RenderAccessibilityManager that owns us.
  raw_ptr<RenderAccessibilityManager, ExperimentalRenderer>
      render_accessibility_manager_;

  // The associated RenderFrameImpl by means of the RenderAccessibilityManager.
  raw_ptr<RenderFrameImpl, ExperimentalRenderer> render_frame_;

  // This keeps accessibility enabled as long as it lives.
  std::unique_ptr<blink::WebAXContext> ax_context_;

  // Manages the automatic image annotations, if enabled.
  std::unique_ptr<AXImageAnnotator> ax_image_annotator_;

  using PluginAXTreeSerializer =
      ui::AXTreeSerializer<const ui::AXNode*, std::vector<const ui::AXNode*>>;
  // AXTreeSerializer's AXSourceNodeVectorType is not a vector<raw_ptr> due to
  // performance regressions detected in blink_perf.accessibility tests.
  RAW_PTR_EXCLUSION std::unique_ptr<PluginAXTreeSerializer> plugin_serializer_;
  raw_ptr<PluginAXTreeSource, ExperimentalRenderer> plugin_tree_source_;

  // Token to return this token in the next IPC, so that RenderFrameHostImpl
  // can discard stale data, when the token does not match the expected token.
  std::optional<uint32_t> reset_token_;

  // Whether or not we've injected a stylesheet in this document
  // (only when debugging flags are enabled, never under normal circumstances).
  bool has_injected_stylesheet_ = false;

  // Whether we should highlight annotation results visually on the page
  // for debugging.
  bool image_annotation_debugging_ = false;

  // The specified page language, or empty if unknown.
  std::string page_language_;

  // The URL-keyed metrics recorder interface.
  std::unique_ptr<ukm::MojoUkmRecorder> ukm_recorder_;

  // The longest amount of time spent serializing the accessibility tree
  // in SendPendingAccessibilityEvents. This is periodically uploaded as
  // a UKM and then reset.
  base::TimeDelta slowest_serialization_time_;

  // Tracks the stage in the loading process for a document, which is used to
  // determine if serialization is part of the loading process or post load.
  enum class LoadingStage {
    // Expect to process a kLoadComplete event.
    kPreload,
    // kLoadComplete event has been processed and waiting on next AXReady call
    // to indicate that we have completed processing all events associated with
    // loading the document.
    kLoadCompleted,
    // All accessibility events associated with the initial document load have
    // been serialized.
    kPostLoad
  };
  LoadingStage loading_stage_ = LoadingStage::kPreload;

  // The amount of time since the last UKM upload.
  std::unique_ptr<base::ElapsedTimer> ukm_timer_;

  // The UKM Source ID that corresponds to the web page represented by
  // slowest_serialization_ms_. We report UKM before the user navigates
  // away, or every few minutes.
  ukm::SourceId last_ukm_source_id_;

  // The AxID of the first unlabeled image we have encountered in this tree.
  //
  // Used to ensure that the tutor message that explains to screen reader users
  // how to turn on automatic image labels is provided only once.
  mutable std::optional<int32_t> first_unlabeled_image_id_ = std::nullopt;

  // Note: this is the accessibility mode communicated to this object.
  // The actual accessibility mode on a Document is the combination of this
  // mode and any other active AXContext objects' accessibility modes.
  ui::AXMode accessibility_mode_;

  // A set of IDs for which we should always load inline text boxes.
  std::set<int32_t> load_inline_text_boxes_ids_;

  // So we can queue up tasks to be executed later.
  base::WeakPtrFactory<RenderAccessibilityImpl>
      weak_factory_for_pending_events_{this};

  friend class AXImageAnnotatorTest;
  friend class PluginActionHandlingTest;
  friend class RenderAccessibilityImplTest;
  friend class RenderAccessibilityImplUKMTest;
};

}  // namespace content

#endif  // CONTENT_RENDERER_ACCESSIBILITY_RENDER_ACCESSIBILITY_IMPL_H_
