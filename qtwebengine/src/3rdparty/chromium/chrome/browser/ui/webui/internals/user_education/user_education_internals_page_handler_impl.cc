// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/internals/user_education/user_education_internals_page_handler_impl.h"

#include <stdint.h>

#include <concepts>
#include <sstream>
#include <string>

#include "base/feature_list.h"
#include "base/i18n/time_formatting.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/time/time.h"
#include "chrome/browser/feature_engagement/tracker_factory.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/browser_window.h"
#include "chrome/browser/ui/webui/internals/user_education/user_education_internals.mojom-forward.h"
#include "chrome/browser/user_education/user_education_service.h"
#include "chrome/browser/user_education/user_education_service_factory.h"
#include "components/feature_engagement/public/tracker.h"
#include "components/user_education/common/feature_promo_data.h"
#include "components/user_education/common/feature_promo_registry.h"
#include "components/user_education/common/feature_promo_specification.h"
#include "components/user_education/common/feature_promo_storage_service.h"
#include "components/user_education/common/tutorial_description.h"
#include "components/user_education/common/user_education_features.h"
#include "content/public/browser/web_ui.h"
#include "third_party/abseil-cpp/absl/strings/ascii.h"
#include "ui/base/interaction/element_identifier.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/webui/resource_path.h"

using mojom::user_education_internals::FeaturePromoDemoPageData;
using mojom::user_education_internals::FeaturePromoDemoPageDataPtr;
using mojom::user_education_internals::FeaturePromoDemoPageInfo;
using mojom::user_education_internals::FeaturePromoDemoPageInfoPtr;

namespace {

user_education::TutorialService* GetTutorialService(Profile* profile) {
  auto* service = UserEducationServiceFactory::GetForBrowserContext(profile);
  return service ? &service->tutorial_service() : nullptr;
}

user_education::FeaturePromoRegistry* GetFeaturePromoRegistry(
    Profile* profile) {
  auto* const service =
      UserEducationServiceFactory::GetForBrowserContext(profile);
  return service ? &service->feature_promo_registry() : nullptr;
}

user_education::FeaturePromoStorageService* GetStorageService(
    Profile* profile) {
  auto* const service =
      UserEducationServiceFactory::GetForBrowserContext(profile);
  return service ? &service->feature_promo_storage_service() : nullptr;
}

user_education::FeaturePromoController* GetFeaturePromoController(
    content::WebUI* web_ui) {
  return chrome::FindBrowserWithTab(web_ui->GetWebContents())
      ->window()
      ->GetFeaturePromoController();
}

std::string GetPromoTypeString(
    const user_education::FeaturePromoSpecification& spec) {
  switch (spec.promo_type()) {
    case user_education::FeaturePromoSpecification::PromoType::kUnspecified:
      return "Unknown";
    case user_education::FeaturePromoSpecification::PromoType::kCustomAction:
      return "Custom Action";
    case user_education::FeaturePromoSpecification::PromoType::kLegacy:
      return "Legacy Promo";
    case user_education::FeaturePromoSpecification::PromoType::kSnooze:
      return "Snooze";
    case user_education::FeaturePromoSpecification::PromoType::kToast:
      return "Toast";
    case user_education::FeaturePromoSpecification::PromoType::kTutorial:
      return "Tutorial";
  }
}

const base::Feature* GetFeatureByName(const std::string& feature_name,
                                      Profile* profile) {
  auto* const registry = GetFeaturePromoRegistry(profile);
  if (registry) {
    const auto& feature_promo_specifications =
        registry->GetRegisteredFeaturePromoSpecifications();
    for (const auto& [feature, spec] : feature_promo_specifications) {
      if (feature_name == feature->name) {
        return feature;
      }
    }
  }
  return nullptr;
}

std::string RemovePrefixAndCamelCase(std::string str, const char* prefix) {
  // Remove the prefix if one is present.
  if (str.starts_with(prefix)) {
    str = str.substr(strlen(prefix));
  }

  // De-camel-case the string. This inserts spaces between segments that are
  // either capitalized words or all-caps acronyms.
  //
  // For example, "SaveToCSVPromo" would become "Save To CSV Promo".
  //
  // This doesn't work for every possible string but does work for almost
  // anything that follows established IPH naming conventions.
  std::string result;
  bool was_cap_before = false;
  bool was_cap = false;
  for (char ch : str) {
    const bool is_cap = absl::ascii_isupper(ch);
    if (result.length() > 1U) {
      if (was_cap && (!is_cap || (is_cap && !was_cap_before))) {
        const char prev = result.back();
        result.pop_back();
        result.push_back(' ');
        result.push_back(prev);
      }
    }
    result.push_back(ch);
    was_cap_before = was_cap;
    was_cap = is_cap;
  }

  return result;
}

// Takes the name of a feature and creates a human-readable title out of it to
// be displayed on the tester page.
std::string GetTitleFromFeaturePromoData(
    const base::Feature* feature,
    const user_education::FeaturePromoSpecification& spec) {
  return RemovePrefixAndCamelCase(feature->name, "IPH_");
}

std::string GetDescriptionFromFeaturePromoData(
    const user_education::FeaturePromoSpecification& spec) {
  const auto& desc = spec.metadata().triggering_condition_description;
  return desc.empty() ? desc : "May be triggered when: " + desc;
}

std::vector<std::string> GetSupportedPlatforms(
    const user_education::FeaturePromoSpecification& spec) {
  std::vector<std::string> result;
  using Platforms =
      user_education::FeaturePromoSpecification::Metadata::Platforms;
  for (const auto platform : spec.metadata().platforms) {
    switch (platform) {
      case Platforms::kWindows:
        result.push_back("Windows");
        break;
      case Platforms::kMac:
        result.push_back("Mac");
        break;
      case Platforms::kLinux:
        result.push_back("Linux");
        break;
      case Platforms::kChromeOSAsh:
        result.push_back("ChromeOS Ash");
        break;
      case Platforms::kChromeOSLacros:
        result.push_back("ChromeOS Lacros");
        break;
    }
  }

  if (result.empty()) {
    result.push_back("Unknown");
  }
  return result;
}

std::vector<std::string> GetRequiredFeatures(
    const user_education::FeaturePromoSpecification& spec) {
  std::vector<std::string> result;
  const auto& required_features = spec.metadata().required_features;
  std::transform(required_features.begin(), required_features.end(),
                 std::back_inserter(result),
                 [](const base::Feature* feature) { return feature->name; });
  return result;
}

// Takes a string resource which may have placeholder substitutions and/or
// plural variations, and creates a single, readable exemplar string.
//
// This is used to describe the title and text of an IPH which normally
// requires specific parameters to be passed at runtime via a
// `FeaturePromoSpecification::FormatParameters`.
//
// For example, say the string was:
// ```
//   {COUNT, plural,
//       =0 {All tabs were opened in Incognito window.}
//       =1 {One tab was opened in Incognito window.}
//       other {{COUNT} tabs were opened in Incognito window.}}
// ```
//
// Then the resulting example text to be displayed on the tester page would be:
//
//   "All tabs were opened in Incognito window."
//
// This is not perfect, but it at least shows the form the text could take; an
// unmodified string resource with singular/plural substitutions is quite messy.
std::string RemoveStringPlaceholders(int message_id) {
  std::string str = l10n_util::GetStringUTF8(message_id);

  // If this is a plural string, pick the first string. Note that the first
  // string almost never uses the count, so that won't be substituted; this
  // could be changed in the future if that assumption is wrong.
  if (str.starts_with('{')) {
    auto start = str.find('{', 1);
    if (start != std::string::npos) {
      const auto end = str.find('}', ++start);
      if (end != std::string::npos) {
        str = str.substr(start, end - start);
      }
    }
  }

  // Allocate a full 9 replacement arguments.
  std::vector<std::string> replacements;
  for (int i = 0; i < 9; ++i) {
    const char digit = static_cast<char>(u'1' + i);
    auto& replacement = replacements.emplace_back();
    replacement.push_back('[');
    replacement.push_back(digit);
    replacement.push_back(']');
  }

  return base::ReplaceStringPlaceholders(str, replacements, nullptr);
}

// Converts the title and text of a promo into the multi-part "instructions"
// format shared by IPH and Tutorials on the tester page.
//
// Substitutions and plurals are stripped out of both to produce more readable
// text; see `RemoveStringPlaceholders()`.
std::vector<std::string> GetPromoInstructions(
    const user_education::FeaturePromoSpecification& spec) {
  std::vector<std::string> instructions;
  if (spec.bubble_title_string_id()) {
    instructions.push_back(
        RemoveStringPlaceholders(spec.bubble_title_string_id()));
  }
  instructions.push_back(
      RemoveStringPlaceholders(spec.bubble_body_string_id()));
  return instructions;
}

std::string GetPromoFollowedBy(
    const user_education::FeaturePromoSpecification& spec) {
  return spec.tutorial_id();
}

template <typename T>
auto FormatDemoPageData(const char* key,
                        const T& value,
                        bool is_constant = false) {
  std::ostringstream oss;
  oss << value;
  std::string strvalue = oss.str();
  if (is_constant) {
    strvalue = RemovePrefixAndCamelCase(strvalue, "k");
  }
  return FeaturePromoDemoPageData::New(key, strvalue);
}

auto FormatDemoPageData(const char* key, bool value) {
  return FeaturePromoDemoPageData::New(key, value ? "yes" : "no");
}

auto FormatDemoPageData(const char* key, base::Time value) {
  auto result = value.is_null()
                    ? "unknown"
                    : base::UTF16ToUTF8(
                          base::TimeFormatShortDateAndTimeWithTimeZone(value));
  return FeaturePromoDemoPageData::New(key, result);
}

auto GetPromoData(
    const user_education::FeaturePromoSpecification& spec,
    const user_education::FeaturePromoStorageService* storage_service,
    const feature_engagement::Tracker* tracker) {
  std::vector<FeaturePromoDemoPageDataPtr> result;
  if (storage_service) {
    auto promo_data = storage_service->ReadPromoData(*spec.feature());
    if (promo_data.has_value()) {
      if (spec.promo_subtype() ==
          user_education::FeaturePromoSpecification::PromoSubtype::kPerApp) {
        result.emplace_back(FormatDemoPageData(
            "Shown for apps", promo_data->shown_for_apps.size()));
      } else {
        result.emplace_back(
            FormatDemoPageData("Show count", promo_data->show_count));
        result.emplace_back(
            FormatDemoPageData("First show time", promo_data->first_show_time));
        result.emplace_back(
            FormatDemoPageData("Last show time", promo_data->last_show_time));
        if (spec.promo_type() ==
                user_education::FeaturePromoSpecification::PromoType::kSnooze ||
            spec.promo_type() == user_education::FeaturePromoSpecification::
                                     PromoType::kTutorial) {
          result.emplace_back(
              FormatDemoPageData("Snooze count", promo_data->snooze_count));
          result.emplace_back(FormatDemoPageData("Last snooze time",
                                                 promo_data->last_snooze_time));
        }
        result.emplace_back(
            FormatDemoPageData("Dismissed?", promo_data->is_dismissed));
        result.emplace_back(FormatDemoPageData("Last dismissed by",
                                               promo_data->last_dismissed_by,
                                               /*is_constant=*/true));
      }
    }
  }
  const bool is_enabled = base::FeatureList::IsEnabled(*spec.feature());
  result.emplace_back(FormatDemoPageData("Feature enabled?", is_enabled));
  for (const auto& [config, count] : tracker->ListEvents(*spec.feature())) {
    std::ostringstream oss;
    oss << "Required condition: " << config.name << config.comparator
        << " Actual:";
    result.emplace_back(FormatDemoPageData(oss.str().c_str(), count));
  }
  if (is_enabled) {
    result.emplace_back(
        FormatDemoPageData("Feature Engagement Tracker OK?",
                           tracker->WouldTriggerHelpUI(*spec.feature())));
  }
  return result;
}

std::string GetTutorialDescription(
    const user_education::TutorialDescription& desc) {
  return std::string();
}

std::vector<std::string> GetTutorialInstructions(
    const user_education::TutorialDescription& desc) {
  std::vector<std::string> instructions;
  for (const auto& step : desc.steps) {
    if (step.body_text_id()) {
      instructions.emplace_back(l10n_util::GetStringUTF8(step.body_text_id()));
    }
  }
  return instructions;
}

std::string GetTutorialTypeString(
    const user_education::TutorialDescription& desc) {
  return desc.can_be_restarted ? "Restartable Tutorial" : "Tutorial";
}

int64_t GetTutorialMilestone(const user_education::TutorialDescription& desc) {
  return 0;
}

std::vector<std::string> GetSupportedPlatforms(
    const user_education::TutorialDescription& desc) {
  return std::vector<std::string>{"All"};
}

}  // namespace

UserEducationInternalsPageHandlerImpl::UserEducationInternalsPageHandlerImpl(
    content::WebUI* web_ui,
    Profile* profile,
    mojo::PendingReceiver<
        mojom::user_education_internals::UserEducationInternalsPageHandler>
        receiver)
    : web_ui_(web_ui),
      profile_(profile),
      receiver_(this, std::move(receiver)) {}

UserEducationInternalsPageHandlerImpl::
    ~UserEducationInternalsPageHandlerImpl() = default;

void UserEducationInternalsPageHandlerImpl::GetTutorials(
    GetTutorialsCallback callback) {
  auto* const tutorial_service = GetTutorialService(profile_);
  if (!tutorial_service) {
    std::move(callback).Run({});
    return;
  }

  std::vector<FeaturePromoDemoPageInfoPtr> info_list;
  const auto ids =
      tutorial_service->tutorial_registry()->GetTutorialIdentifiers();
  for (const auto& id : ids) {
    auto* const description =
        tutorial_service->tutorial_registry()->GetTutorialDescription(id);
    if (description) {
      info_list.emplace_back(FeaturePromoDemoPageInfo::New(
          id, GetTutorialDescription(*description), id,
          GetTutorialTypeString(*description),
          GetTutorialMilestone(*description),
          GetSupportedPlatforms(*description), std::vector<std::string>(),
          GetTutorialInstructions(*description),
          /*followed_by=*/"", std::vector<FeaturePromoDemoPageDataPtr>()));
    } else {
      NOTREACHED();
    }
  }
  std::move(callback).Run(std::move(info_list));
}

void UserEducationInternalsPageHandlerImpl::StartTutorial(
    const std::string& tutorial_id,
    StartTutorialCallback callback) {
  auto* const tutorial_service = GetTutorialService(profile_);
  std::string result;
  if (tutorial_service) {
    const ui::ElementContext context =
        chrome::FindBrowserWithProfile(profile_)->window()->GetElementContext();
    tutorial_service->StartTutorial(tutorial_id, context);
    if (!tutorial_service->IsRunningTutorial()) {
      result = "Failed to start tutorial " + tutorial_id;
    }
  } else {
    result = "No tutorial service.";
  }
  std::move(callback).Run(result);
}

void UserEducationInternalsPageHandlerImpl::GetSessionData(
    GetSessionDataCallback callback) {
  std::vector<FeaturePromoDemoPageDataPtr> data;

  auto* const storage_service = GetStorageService(profile_);
  if (storage_service) {
    const auto session_data = storage_service->ReadSessionData();
    data.emplace_back(
        FormatDemoPageData("Session start", session_data.start_time));
    data.emplace_back(FormatDemoPageData("Last active at",
                                         session_data.most_recent_active_time));
    const auto policy_data = storage_service->ReadPolicyData();
    data.emplace_back(FormatDemoPageData(
        "Last heavyweight promo at", policy_data.last_heavyweight_promo_time));
  }
  if (!base::FeatureList::IsEnabled(
          user_education::features::kUserEducationExperienceVersion2)) {
    data.emplace_back(FormatDemoPageData(
        "(Feature Engagement session data not available.)", ""));
  }
  return std::move(callback).Run(std::move(data));
}

void UserEducationInternalsPageHandlerImpl::GetFeaturePromos(
    GetFeaturePromosCallback callback) {
  std::vector<FeaturePromoDemoPageInfoPtr> info_list;

  auto* const registry = GetFeaturePromoRegistry(profile_);
  auto* const storage_service = GetStorageService(profile_);
  auto* const tracker =
      feature_engagement::TrackerFactory::GetForBrowserContext(profile_);
  if (registry) {
    const auto& feature_promo_specifications =
        registry->GetRegisteredFeaturePromoSpecifications();
    for (const auto& [feature, spec] : feature_promo_specifications) {
      info_list.emplace_back(FeaturePromoDemoPageInfo::New(
          GetTitleFromFeaturePromoData(feature, spec),
          GetDescriptionFromFeaturePromoData(spec), feature->name,
          GetPromoTypeString(spec), spec.metadata().launch_milestone,
          GetSupportedPlatforms(spec), GetRequiredFeatures(spec),
          GetPromoInstructions(spec), GetPromoFollowedBy(spec),
          GetPromoData(spec, storage_service, tracker)));
    }
  }

  return std::move(callback).Run(std::move(info_list));
}

void UserEducationInternalsPageHandlerImpl::ShowFeaturePromo(
    const std::string& feature_name,
    ShowFeaturePromoCallback callback) {
  const base::Feature* feature = GetFeatureByName(feature_name, profile_);
  if (!feature) {
    std::move(callback).Run(std::string("Cannot find IPH."));
    return;
  }

  auto* const feature_promo_controller = GetFeaturePromoController(web_ui_);

  const auto showed_promo =
      feature_promo_controller->MaybeShowPromoForDemoPage(*feature);

  std::string reason;
  if (!showed_promo) {
    using Failure = user_education::FeaturePromoResult::Failure;
    switch (*showed_promo.failure()) {
      case Failure::kBlockedByContext:
        reason = "Cannot show IPH in this browser window.";
        break;
      case Failure::kBlockedByPromo:
        reason = "Failed to show IPH due to high-priority IPH.";
        break;
      case Failure::kBlockedByUi:
        reason = "Cannot show IPH due to conflicting UI or missing anchor.";
        break;
      case Failure::kCanceled:
        reason = "IPH was canceled before it could be shown.";
        break;
      case Failure::kError:
        reason = "Internal error.";
        break;
      case Failure::kBlockedByConfig:
      case Failure::kFeatureDisabled:
      case Failure::kPermanentlyDismissed:
      case Failure::kSnoozed:
      case Failure::kBlockedByGracePeriod:
      case Failure::kBlockedByCooldown:
      case Failure::kRecentlyAborted:
        reason = "Unexpected failure (should not happen for demo).";
    }
  }
  std::move(callback).Run(reason);
}

void UserEducationInternalsPageHandlerImpl::ClearFeaturePromoData(
    const std::string& feature_name,
    ClearFeaturePromoDataCallback callback) {
  const base::Feature* feature = GetFeatureByName(feature_name, profile_);
  if (!feature) {
    std::move(callback).Run(std::string("Cannot find IPH."));
    return;
  }

  auto* const tracker =
      feature_engagement::TrackerFactory::GetForBrowserContext(profile_);
  if (!tracker || !tracker->IsInitialized()) {
    std::move(callback).Run(std::string("Feature Engagement not ready."));
  }

  auto* const storage_service = GetStorageService(profile_);
  if (!storage_service) {
    std::move(callback).Run(std::string("No storage service."));
    return;
  }

  tracker->ClearEventData(*feature);
  storage_service->Reset(*feature);
  std::move(callback).Run(std::string());
}

void UserEducationInternalsPageHandlerImpl::ClearSessionData(
    ClearSessionDataCallback callback) {
  auto* const storage_service = GetStorageService(profile_);
  if (!storage_service) {
    std::move(callback).Run(std::string("No storage service."));
    return;
  }

  storage_service->ResetPolicy();

  // Create a session with start time well in the past to avoid grace period,
  // and most recent active time as now to prevent a new session from
  // immediately starting.
  user_education::FeaturePromoSessionData session_data;
  session_data.most_recent_active_time = storage_service->GetCurrentTime();
  storage_service->SaveSessionData(session_data);

  std::move(callback).Run(std::string());
}
