// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_COMPOSE_CORE_BROWSER_COMPOSE_METRICS_H_
#define COMPONENTS_COMPOSE_CORE_BROWSER_COMPOSE_METRICS_H_

#include "base/time/time.h"
#include "services/metrics/public/cpp/ukm_builders.h"
#include "services/metrics/public/cpp/ukm_source_id.h"

namespace base {
class TimeDelta;
}  // namespace base

namespace compose {

// Compose histogram names.
extern const char kComposeDialogOpenLatency[];
extern const char kComposeDialogSelectionLength[];
extern const char kComposeRequestReason[];
extern const char kComposeRequestDurationOk[];
extern const char kComposeRequestDurationError[];
extern const char kComposeRequestStatus[];
extern const char kComposeSessionComposeCount[];
extern const char kComposeSessionCloseReason[];
extern const char kComposeSessionDialogShownCount[];
extern const char kComposeSessionEventCounts[];
extern const char kComposeSessionDuration[];
extern const char kComposeSessionOverOneDay[];
extern const char kComposeSessionUndoCount[];
extern const char kComposeSessionUpdateInputCount[];
extern const char kComposeShowStatus[];
extern const char kComposeFirstRunSessionCloseReason[];
extern const char kComposeFirstRunSessionDialogShownCount[];
extern const char kComposeMSBBSessionCloseReason[];
extern const char kComposeMSBBSessionDialogShownCount[];
extern const char kInnerTextNodeOffsetFound[];
extern const char kComposeContextMenuCtr[];
extern const char kOpenComposeDialogResult[];

// Enum for calculating the CTR of the Compose context menu item.
// These values are persisted to logs. Entries should not be renumbered and
// numeric values should never be reused. Keep in sync with
// ComposeContextMenuCtrEvent in
// src/tools/metrics/histograms/metadata/compose/enums.xml.
enum class ComposeContextMenuCtrEvent {
  kMenuItemDisplayed = 0,
  kMenuItemClicked = 1,
  kMaxValue = kMenuItemClicked,
};

// Keep in sync with ComposeRequestReason in
// src/tools/metrics/histograms/metadata/compose/enums.xml.
enum class ComposeRequestReason {
  kFirstRequest = 0,
  kRetryRequest = 1,
  kUpdateRequest = 2,
  kLengthShortenRequest = 3,
  kLengthElaborateRequest = 4,
  kToneCasualRequest = 5,
  kToneFormalRequest = 6,
  kMaxValue = kToneFormalRequest,
};

// Keep in sync with ComposeMSBBSessionCloseReasonType in
// src/tools/metrics/histograms/metadata/compose/enums.xml.
enum class ComposeMSBBSessionCloseReason {
  kMSBBEndedImplicitly = 0,
  kMSBBCloseButtonPressed = 1,
  kMSBBAcceptedWithoutInsert = 2,
  kMSBBAcceptedWithInsert = 3,
  kMaxValue = kMSBBAcceptedWithInsert,
};

// Keep in sync with ComposeFirstRunSessionCloseReasonType in
// src/tools/metrics/histograms/metadata/compose/enums.xml.
enum class ComposeFirstRunSessionCloseReason {
  kEndedImplicitly = 0,
  kCloseButtonPressed = 1,
  kFirstRunDisclaimerAcknowledgedWithoutInsert = 2,
  kFirstRunDisclaimerAcknowledgedWithInsert = 3,
  kNewSessionWithSelectedText = 4,
  kMaxValue = kNewSessionWithSelectedText,
};

// Keep in sync with ComposeSessionCloseReasonType in
// src/tools/metrics/histograms/metadata/compose/enums.xml.
enum class ComposeSessionCloseReason {
  kAcceptedSuggestion = 0,
  kCloseButtonPressed = 1,
  kEndedImplicitly = 2,
  kNewSessionWithSelectedText = 3,
  kCanceledBeforeResponseReceived = 4,
  kMaxValue = kCanceledBeforeResponseReceived,
};

// Keep in sync with ComposeSessionEventCounts in
// src/tools/metrics/histograms/metadata/compose/enums.xml.
enum class ComposeSessionEventTypes {
  kDialogShown = 0,
  kFREShown = 1,
  kFREAccepted = 2,
  kMSBBShown = 3,
  kMSBBSettingsOpened = 4,
  kMSBBEnabled = 5,
  kStartedWithSelection = 6,
  kCreateClicked = 7,
  kUpdateClicked = 8,
  kRetryClicked = 9,
  kUndoClicked = 10,
  kShortenClicked = 11,
  kElaborateClicked = 12,
  kCasualClicked = 13,
  kFormalClicked = 14,
  kThumbsDown = 15,
  kThumbsUp = 16,
  kInsertClicked = 17,
  kCloseClicked = 18,
  kMaxValue = kCloseClicked,
};

// Enum for recording the show status of the Compose context menu item.
// These values are persisted to logs. Entries should not be renumbered and
// numeric values should never be reused. Keep in sync with
// ComposeShowStatus in
// src/tools/metrics/histograms/metadata/compose/enums.xml.
enum class ComposeShowStatus {
  kShouldShow = 0,
  kGenericBlocked = 1,
  kIncompatibleFieldType = 2,
  // kDisabledMsbb is no longer used now that we have a MSBB dialog.
  kDisabledMsbb = 3,  // obsolete
  kSignedOut = 4,
  kUnsupportedLanguage = 5,
  kFormFieldInCrossOriginFrame = 6,
  kPerUrlChecksFailed = 7,
  kUserNotAllowedByOptimizationGuide = 8,
  kNotComposeEligible = 9,
  kIncorrectScheme = 10,
  kFormFieldNestedInFencedFrame = 11,
  kFeatureFlagDisabled = 12,
  kMaxValue = kFeatureFlagDisabled,
};

// Struct containing event and logging information for an individual
// |ComposeSession|.
struct ComposeSessionEvents {
  ComposeSessionEvents();
  ComposeSessionEvents(ComposeSessionEvents& e) = delete;
  ComposeSessionEvents& operator=(ComposeSessionEvents& e) = delete;
  ~ComposeSessionEvents() = default;

  // Logging counters.
  // The total number of Compose Requests for the session.
  unsigned int compose_count = 0;
  // Times we have shown the compose dialog.
  unsigned int dialog_shown_count = 0;
  // Times we have shown the first run dialog.
  unsigned int fre_dialog_shown_count = 0;
  // Times we have shown the dialog to enable MSBB.
  unsigned int msbb_dialog_shown_count = 0;
  // Times the user has pressed "undo" this session.
  unsigned int undo_count = 0;
  // Compose request after input edited.
  unsigned int update_input_count = 0;
  // Tiems the user has pressed the "Retry" button.
  unsigned int regenerate_count = 0;
  // Times the user has picked the "Shorter" option.
  unsigned int shorten_count = 0;
  // Times the user has picked the "Elaborate" option.
  unsigned int lengthen_count = 0;
  // Times the user has picked the "Formal" option.
  unsigned int formal_count = 0;
  // Times the user has picked the "Casual" option.
  unsigned int casual_count = 0;

  // Logging flags
  // True if the FRE was completed in the session.
  bool fre_completed_in_session = false;
  // True if the MSBB settings were opened.
  bool msbb_settings_opened = false;
  // True if the MSBB was enabled in the session.
  bool msbb_enabled_in_session = false;

  // True if the session started with selected text.
  bool has_initial_text = false;
  // True if thumbs up was ever clicked during the session.
  bool has_thumbs_up = false;
  // True if thumbs down was ever clicked during the session.
  bool has_thumbs_down = false;

  // True if the results were eventually inserted back to the web page.
  bool inserted_results = false;
  // True if the the user closed the compose session via the "x" button.
  bool close_clicked = false;
};

// Enum with the possible reasons for it being impossible to open the Compose
// dialog after the user requested it.
// Keep in sync with OpenComposeDialogResult in
// src/tools/metrics/histograms/metadata/compose/enums.xml.
enum class OpenComposeDialogResult {
  kSuccess = 0,
  kNoChromeComposeClient = 1,
  kNoRenderFrameHost = 2,
  kNoContentAutofillDriver = 3,
  kAutofillFormDataNotFound = 4,
  kAutofillFormFieldDataNotFound = 5,
  kNoWebContents = 6,
  kFailedCreatingComposeDialogView = 7,
  kMaxValue = kFailedCreatingComposeDialogView
};

// Enum to log if the inner text succusfuly found an offset
// Keep in sync with ComposeInnerTextNodeOffset in
// src/tools/metrics/histograms/metadata/compose/enums.xml.
enum class ComposeInnerTextNodeOffset {
  kNoOffsetFound = 0,
  kOffsetFound = 1,
  kMaxValue = kOffsetFound
};

// Class that automatically reports any UKM metrics for the page-level Compose
// UKM as defined in go/ukm-collection-chrome-compose.
class PageUkmTracker {
 public:
  PageUkmTracker(ukm::SourceId source_id);
  ~PageUkmTracker();

  // The compose menu item was shown in a context menu.
  void MenuItemShown();

  // The compose menu item was clicked, opening Compose.
  void MenuItemClicked();

  // The composed text was accepted and inserted into the webpage by the user.
  void ComposeTextInserted();

  // The compose dialog was requested but not shown due to problems obtaining
  // form data from Autofill.
  void ShowDialogAbortedDueToMissingFormData();
  void ShowDialogAbortedDueToMissingFormFieldData();

  // Records UKM if any of the above events happened during this object's
  // lifetime.  Called in the destructor.
  void MaybeLogUkm();

 private:
  bool event_was_recorded_ = false;
  unsigned int menu_item_shown_count_ = 0;
  unsigned int menu_item_clicked_count_ = 0;
  unsigned int compose_text_inserted_count_ = 0;
  unsigned int missing_form_data_count_ = 0;
  unsigned int missing_form_field_data_count_ = 0;

  ukm::SourceId source_id;
};

void LogComposeContextMenuCtr(ComposeContextMenuCtrEvent event);

void LogComposeContextMenuShowStatus(ComposeShowStatus status);

void LogOpenComposeDialogResult(OpenComposeDialogResult result);

void LogComposeRequestReason(ComposeRequestReason reason);

// Log the duration of a compose request. |is_valid| indicates the status of
// the request.
void LogComposeRequestDuration(base::TimeDelta duration, bool is_ok);

void LogComposeFirstRunSessionCloseReason(
    ComposeFirstRunSessionCloseReason reason);

// Log session based metrics when a FRE session ends.
void LogComposeFirstRunSessionDialogShownCount(
    ComposeFirstRunSessionCloseReason reason,
    int dialog_shown_count);

void LogComposeMSBBSessionCloseReason(ComposeMSBBSessionCloseReason reason);

// Log session based metrics when a consent session ends.
void LogComposeMSBBSessionDialogShownCount(ComposeMSBBSessionCloseReason reason,
                                           int dialog_shown_count);

// Log session based metrics when a session ends.
// Should only be called once per session.
void LogComposeSessionCloseMetrics(ComposeSessionCloseReason reason,
                                   const ComposeSessionEvents& session_events);

// Log session based UKM metrics when the session ends.
void LogComposeSessionCloseUkmMetrics(
    ukm::SourceId source_id,
    const ComposeSessionEvents& session_events);

// Log the amount trimmed from the inner text from the page (in bytes) when the
// dialog is opened.
void LogComposeDialogInnerTextShortenedBy(int shortened_by);

// Log the size (in bytes) of the untrimmed inner text from the page when the
// dialog is opened.
void LogComposeDialogInnerTextSize(int size);

// Log if the inner text node offset was found successfully.
void LogComposeDialogInnerTextOffsetFound(bool inner_offset_found);

// Log the time taken for the dialog to be fully shown and interactable.
void LogComposeDialogOpenLatency(base::TimeDelta duration);

// Log the character length of the selection when the dialog is opened.
void LogComposeDialogSelectionLength(int length);

// Log the session duration with |session_suffix| applied to histogram name.
void LogComposeSessionDuration(base::TimeDelta session_duration,
                               std::string session_suffix = "");

}  // namespace compose

#endif  // COMPONENTS_COMPOSE_CORE_BROWSER_COMPOSE_METRICS_H_
