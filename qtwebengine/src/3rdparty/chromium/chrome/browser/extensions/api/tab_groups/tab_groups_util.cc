// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/extensions/api/tab_groups/tab_groups_util.h"

#include "base/hash/hash.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "base/token.h"
#include "chrome/browser/extensions/api/tab_groups/tab_groups_constants.h"
#include "chrome/browser/extensions/extension_tab_util.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/browser_list.h"
#include "chrome/browser/ui/tabs/saved_tab_groups/saved_tab_group_keyed_service.h"
#include "chrome/browser/ui/tabs/saved_tab_groups/saved_tab_group_service_factory.h"
#include "chrome/browser/ui/tabs/tab_group.h"
#include "chrome/browser/ui/tabs/tab_group_model.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "chrome/browser/ui/ui_features.h"
#include "components/tab_groups/tab_group_color.h"
#include "components/tab_groups/tab_group_id.h"
#include "components/tab_groups/tab_group_visual_data.h"
#include "content/public/browser/browser_context.h"
#include "extensions/common/error_utils.h"

namespace extensions {
namespace tab_groups_util {

int GetGroupId(const tab_groups::TabGroupId& id) {
  uint32_t hash = base::PersistentHash(id.ToString());
  return std::abs(static_cast<int>(hash));
}

int GetWindowIdOfGroup(const tab_groups::TabGroupId& id) {
  Browser* browser = chrome::FindBrowserWithGroup(id, nullptr);
  if (browser)
    return browser->session_id().id();
  return -1;
}

api::tab_groups::TabGroup CreateTabGroupObject(
    const tab_groups::TabGroupId& id,
    const tab_groups::TabGroupVisualData& visual_data) {
  api::tab_groups::TabGroup tab_group_object;
  tab_group_object.id = GetGroupId(id);
  tab_group_object.collapsed = visual_data.is_collapsed();
  tab_group_object.color = ColorIdToColor(visual_data.color());
  tab_group_object.title = base::UTF16ToUTF8(visual_data.title());
  tab_group_object.window_id = GetWindowIdOfGroup(id);

  return tab_group_object;
}

std::optional<api::tab_groups::TabGroup> CreateTabGroupObject(
    const tab_groups::TabGroupId& id) {
  Browser* browser = chrome::FindBrowserWithGroup(id, nullptr);
  if (!browser)
    return std::nullopt;

  CHECK(browser->tab_strip_model()->SupportsTabGroups());
  TabGroupModel* group_model = browser->tab_strip_model()->group_model();
  const tab_groups::TabGroupVisualData* visual_data =
      group_model->GetTabGroup(id)->visual_data();

  DCHECK(visual_data);

  return CreateTabGroupObject(id, *visual_data);
}

bool GetGroupById(int group_id,
                  content::BrowserContext* browser_context,
                  bool include_incognito,
                  Browser** browser,
                  tab_groups::TabGroupId* id,
                  const tab_groups::TabGroupVisualData** visual_data,
                  std::string* error) {
  if (group_id == -1)
    return false;

  Profile* profile = Profile::FromBrowserContext(browser_context);
  Profile* incognito_profile =
      include_incognito && profile->HasPrimaryOTRProfile()
          ? profile->GetPrimaryOTRProfile(/*create_if_needed=*/true)
          : nullptr;
  for (Browser* target_browser : *BrowserList::GetInstance()) {
    if (target_browser->profile() == profile ||
        target_browser->profile() == incognito_profile) {
      TabStripModel* target_tab_strip = target_browser->tab_strip_model();
      if (!target_tab_strip->SupportsTabGroups())
        continue;
      for (tab_groups::TabGroupId target_group :
           target_tab_strip->group_model()->ListTabGroups()) {
        if (GetGroupId(target_group) == group_id) {
          if (browser)
            *browser = target_browser;
          if (id)
            *id = target_group;
          if (visual_data) {
            *visual_data = target_tab_strip->group_model()
                               ->GetTabGroup(target_group)
                               ->visual_data();
          }
          return true;
        }
      }
    }
  }

  *error =
      ErrorUtils::FormatErrorMessage(tab_groups_constants::kGroupNotFoundError,
                                     base::NumberToString(group_id));
  return false;
}

bool GetGroupById(int group_id,
                  content::BrowserContext* browser_context,
                  bool include_incognito,
                  tab_groups::TabGroupId* id,
                  std::string* error) {
  return GetGroupById(group_id, browser_context, include_incognito, nullptr, id,
                      nullptr, error);
}

api::tab_groups::Color ColorIdToColor(
    const tab_groups::TabGroupColorId& color_id) {
  switch (color_id) {
    case tab_groups::TabGroupColorId::kGrey:
      return api::tab_groups::Color::kGrey;
    case tab_groups::TabGroupColorId::kBlue:
      return api::tab_groups::Color::kBlue;
    case tab_groups::TabGroupColorId::kRed:
      return api::tab_groups::Color::kRed;
    case tab_groups::TabGroupColorId::kYellow:
      return api::tab_groups::Color::kYellow;
    case tab_groups::TabGroupColorId::kGreen:
      return api::tab_groups::Color::kGreen;
    case tab_groups::TabGroupColorId::kPink:
      return api::tab_groups::Color::kPink;
    case tab_groups::TabGroupColorId::kPurple:
      return api::tab_groups::Color::kPurple;
    case tab_groups::TabGroupColorId::kCyan:
      return api::tab_groups::Color::kCyan;
    case tab_groups::TabGroupColorId::kOrange:
      return api::tab_groups::Color::kOrange;
  }

  NOTREACHED();
  return api::tab_groups::Color::kCyan;
}

tab_groups::TabGroupColorId ColorToColorId(api::tab_groups::Color color) {
  switch (color) {
    case api::tab_groups::Color::kGrey:
      return tab_groups::TabGroupColorId::kGrey;
    case api::tab_groups::Color::kBlue:
      return tab_groups::TabGroupColorId::kBlue;
    case api::tab_groups::Color::kRed:
      return tab_groups::TabGroupColorId::kRed;
    case api::tab_groups::Color::kYellow:
      return tab_groups::TabGroupColorId::kYellow;
    case api::tab_groups::Color::kGreen:
      return tab_groups::TabGroupColorId::kGreen;
    case api::tab_groups::Color::kPink:
      return tab_groups::TabGroupColorId::kPink;
    case api::tab_groups::Color::kPurple:
      return tab_groups::TabGroupColorId::kPurple;
    case api::tab_groups::Color::kCyan:
      return tab_groups::TabGroupColorId::kCyan;
    case api::tab_groups::Color::kOrange:
      return tab_groups::TabGroupColorId::kOrange;
    case api::tab_groups::Color::kNone:
      NOTREACHED();
  }

  NOTREACHED();
  return tab_groups::TabGroupColorId::kGrey;
}

bool IsGroupSaved(tab_groups::TabGroupId tab_group_id,
                  TabStripModel* tab_strip_model) {
  // If the feature is turned off, then the tab is not in a saved group.
  if (!base::FeatureList::IsEnabled(features::kTabGroupsSave)) {
    return false;
  }

  // If the service failed to start, then there are no saved tab groups.
  SavedTabGroupKeyedService* saved_tab_group_service =
      SavedTabGroupServiceFactory::GetForProfile(tab_strip_model->profile());
  if (!saved_tab_group_service) {
    return false;
  }
  return saved_tab_group_service->model()->Contains(tab_group_id);
}

}  // namespace tab_groups_util
}  // namespace extensions
