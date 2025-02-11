// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/core/lcp_critical_path_predictor/lcp_critical_path_predictor.h"

#include "base/metrics/histogram_functions.h"
#include "third_party/blink/public/common/features.h"
#include "third_party/blink/public/common/loader/lcp_critical_path_predictor_util.h"
#include "third_party/blink/renderer/core/frame/local_dom_window.h"
#include "third_party/blink/renderer/core/frame/local_frame.h"
#include "third_party/blink/renderer/core/html/html_image_element.h"
#include "third_party/blink/renderer/core/lcp_critical_path_predictor/element_locator.h"
#include "third_party/blink/renderer/core/loader/document_loader.h"

namespace blink {

namespace {

size_t GetLCPPFontURLPredictorMaxUrlLength() {
  static size_t max_length = base::checked_cast<size_t>(
      features::kLCPPFontURLPredictorMaxUrlLength.Get());
  return max_length;
}

}  // namespace

LCPCriticalPathPredictor::LCPCriticalPathPredictor(LocalFrame& frame)
    : frame_(&frame),
      host_(frame.DomWindow()),
      task_runner_(frame.GetTaskRunner(TaskType::kInternalLoading)) {
  CHECK(LcppEnabled());
  if (base::FeatureList::IsEnabled(features::kLCPScriptObserver)) {
    lcp_script_observer_ = MakeGarbageCollected<LCPScriptObserver>(frame_);
  }
}

LCPCriticalPathPredictor::~LCPCriticalPathPredictor() = default;

bool LCPCriticalPathPredictor::HasAnyHintData() const {
  return !lcp_element_locators_.empty() || !lcp_influencer_scripts_.empty();
}

void LCPCriticalPathPredictor::set_lcp_element_locators(
    const std::vector<std::string>& lcp_element_locator_strings) {
  const wtf_size_t reserved_size =
      base::checked_cast<wtf_size_t>(lcp_element_locator_strings.size());
  if (features::
          kLCPCriticalPathPredictorEnableElementLocatorPerformanceImprovements
              .Get()) {
    // Clear current set of locators before receiving replacements.
    lcp_element_locators_.clear();
    lcp_element_locator_strings_.clear();
  }
  lcp_element_locators_.reserve(reserved_size);
  lcp_element_locator_strings_.reserve(reserved_size);
  for (const std::string& serialized_locator : lcp_element_locator_strings) {
    lcp_element_locators_.push_back(ElementLocator());
    bool result =
        lcp_element_locators_.back().ParseFromString(serialized_locator);
    if (!result) {
      // This can happen when the host LCPP database is corrupted or we
      // updated the ElementLocator schema in an incompatible way.
      LOG(INFO) << "Ignoring an invalid lcp_element_locator hint.";
      lcp_element_locators_.pop_back();
    } else {
      lcp_element_locator_strings_.push_back(std::move(serialized_locator));
    }
  }
  CHECK_EQ(lcp_element_locators_.size(), lcp_element_locator_strings_.size());
}

void LCPCriticalPathPredictor::set_lcp_influencer_scripts(
    HashSet<KURL> scripts) {
  lcp_influencer_scripts_ = std::move(scripts);
}

void LCPCriticalPathPredictor::set_fetched_fonts(Vector<KURL> fonts) {
  fetched_fonts_ = std::move(fonts);
}

void LCPCriticalPathPredictor::Reset() {
  lcp_element_locators_.clear();
  lcp_element_locator_strings_.clear();
  lcp_influencer_scripts_.clear();
  fetched_fonts_.clear();

  lcp_predicted_callbacks_.clear();
  called_predicted_callbacks_ = false;
  is_lcp_candidate_found_ = false;
  is_outermost_main_frame_document_loaded_ = false;
}

void LCPCriticalPathPredictor::AddLCPPredictedCallback(LCPCallback callback) {
  lcp_predicted_callbacks_.push_back(std::move(callback));
}

void LCPCriticalPathPredictor::MayRunPredictedCallbacks(
    const Element* lcp_element) {
  if (called_predicted_callbacks_) {
    return;
  }
  called_predicted_callbacks_ = true;
  // TODO(crbug.com/1493255): Trigger callbacks for the entire frame tree.
  Vector<LCPCallback> callbacks;
  callbacks.swap(lcp_predicted_callbacks_);
  for (auto& callback : callbacks) {
    std::move(callback).Run(lcp_element);
  }
}

bool LCPCriticalPathPredictor::IsElementMatchingLocator(
    const Element& element) {
  std::string lcp_element_locator_string =
      element_locator::OfElement(element).SerializeAsString();
  return lcp_element_locator_strings_.Contains(lcp_element_locator_string);
}

void LCPCriticalPathPredictor::OnLargestContentfulPaintUpdated(
    const Element& lcp_element) {
  if (base::FeatureList::IsEnabled(features::kLCPCriticalPathPredictor)) {
    std::string lcp_element_locator_string =
        element_locator::OfElement(lcp_element).SerializeAsString();

    is_lcp_candidate_found_ = true;
    // Regard `lcp_element` is the candidate if its locator is found in
    // set_lcp_element_locators(lcp_element_locator_strings).
    // See PredictLcpElementLocators() for the contents detail.
    const wtf_size_t predicted_lcp_index =
        lcp_element_locator_strings_.Find(lcp_element_locator_string);
    if (predicted_lcp_index != kNotFound) {
      MayRunPredictedCallbacks(&lcp_element);
    }
    if (is_outermost_main_frame_document_loaded_) {
      // Call callbacks as fallback regardless of prediction because
      // This LCP is much too late.
      MayRunPredictedCallbacks(nullptr);
    }

    features::LcppRecordedLcpElementTypes recordable_lcp_element_type =
        features::kLCPCriticalPathPredictorRecordedLcpElementTypes.Get();
    bool should_record_element_locator =
        (recordable_lcp_element_type ==
         features::LcppRecordedLcpElementTypes::kAll) ||
        (recordable_lcp_element_type ==
             features::LcppRecordedLcpElementTypes::kImageOnly &&
         IsA<HTMLImageElement>(lcp_element));

    if (should_record_element_locator) {
      base::UmaHistogramCounts10000(
          "Blink.LCPP.LCPElementLocatorSize",
          base::checked_cast<int>(lcp_element_locator_string.size()));

      if (lcp_element_locator_string.size() <=
          base::checked_cast<size_t>(
              features::kLCPCriticalPathPredictorMaxElementLocatorLength
                  .Get())) {
        GetHost().SetLcpElementLocator(
            lcp_element_locator_string,
            predicted_lcp_index == kNotFound
                ? absl::nullopt
                : absl::optional<wtf_size_t>(predicted_lcp_index));
      }
    }
  }

  if (base::FeatureList::IsEnabled(features::kLCPScriptObserver)) {
    if (const HTMLImageElement* image_element =
            DynamicTo<HTMLImageElement>(lcp_element)) {
      auto& creators = image_element->creator_scripts();
      size_t max_allowed_url_length = base::checked_cast<size_t>(
          features::kLCPScriptObserverMaxUrlLength.Get());
      size_t max_allowed_url_count = base::checked_cast<size_t>(
          features::kLCPScriptObserverMaxUrlCountPerOrigin.Get());
      size_t max_url_length_encountered = 0;
      size_t prediction_match_count = 0;

      Vector<KURL> filtered_script_urls;

      for (auto& url : creators) {
        max_url_length_encountered =
            std::max<size_t>(max_url_length_encountered, url.length());
        if (url.length() >= max_allowed_url_length) {
          continue;
        }
        KURL parsed_url(url);
        if (parsed_url.IsEmpty() || !parsed_url.IsValid() ||
            !parsed_url.ProtocolIsInHTTPFamily()) {
          continue;
        }
        filtered_script_urls.push_back(parsed_url);
        if (lcp_influencer_scripts_.Contains(parsed_url)) {
          prediction_match_count++;
        }
        if (filtered_script_urls.size() >= max_allowed_url_count) {
          break;
        }
      }
      GetHost().SetLcpInfluencerScriptUrls(filtered_script_urls);

      base::UmaHistogramCounts10000(
          "Blink.LCPP.LCPInfluencerUrlsCount",
          base::checked_cast<int>(filtered_script_urls.size()));
      base::UmaHistogramCounts10000(
          "Blink.LCPP.LCPInfluencerUrlsMaxLength",
          base::checked_cast<int>(max_url_length_encountered));
      base::UmaHistogramCounts10000(
          "Blink.LCPP.LCPInfluencerUrlsPredictionMatchCount",
          base::checked_cast<int>(prediction_match_count));
      if (!lcp_influencer_scripts_.empty()) {
        base::UmaHistogramCounts10000(
            "Blink.LCPP.LCPInfluencerUrlsPredictionMatchPercent",
            base::checked_cast<int>((double)prediction_match_count /
                                    lcp_influencer_scripts_.size() * 100));
      }
    }
  }
}

void LCPCriticalPathPredictor::OnFontFetched(const KURL& url) {
  if (!base::FeatureList::IsEnabled(blink::features::kLCPPFontURLPredictor)) {
    return;
  }
  if (!url.ProtocolIsInHTTPFamily()) {
    return;
  }
  if (url.GetString().length() > GetLCPPFontURLPredictorMaxUrlLength()) {
    return;
  }
  GetHost().NotifyFetchedFont(url);
}

void LCPCriticalPathPredictor::OnStartPreload(const KURL& url) {
  if (!base::FeatureList::IsEnabled(
          blink::features::kHttpDiskCachePrewarming)) {
    return;
  }
  if (!frame_->IsOutermostMainFrame()) {
    return;
  }
  if (!url.ProtocolIsInHTTPFamily()) {
    return;
  }
  static size_t max_url_length = base::checked_cast<size_t>(
      features::kHttpDiskCachePrewarmingMaxUrlLength.Get());
  if (url.GetString().length() > max_url_length) {
    return;
  }
  Document* document = frame_->GetDocument();
  if (!document || !document->Loader()) {
    return;
  }
  base::TimeDelta resource_load_start =
      base::TimeTicks::Now() -
      document->Loader()->GetTiming().NavigationStart();
  CHECK_GE(resource_load_start, base::Seconds(0));
  GetHost().NotifyFetchedSubresource(url, resource_load_start);
}

mojom::blink::LCPCriticalPathPredictorHost&
LCPCriticalPathPredictor::GetHost() {
  if (!host_.is_bound() || !host_.is_connected()) {
    host_.reset();
    GetFrame().GetBrowserInterfaceBroker().GetInterface(
        host_.BindNewPipeAndPassReceiver(task_runner_));
  }
  return *host_.get();
}

bool LCPCriticalPathPredictor::IsLcpInfluencerScript(const KURL& url) {
  return lcp_influencer_scripts_.Contains(url);
}

void LCPCriticalPathPredictor::OnOutermostMainFrameDocumentLoad() {
  is_outermost_main_frame_document_loaded_ = true;
  // Call callbacks as fallback because we can not detect
  // which is lcp in the lcps before onload.
  if (is_lcp_candidate_found_) {
    MayRunPredictedCallbacks(nullptr);
  }
}

void LCPCriticalPathPredictor::Trace(Visitor* visitor) const {
  visitor->Trace(frame_);
  visitor->Trace(host_);
  visitor->Trace(lcp_script_observer_);
}

}  // namespace blink
