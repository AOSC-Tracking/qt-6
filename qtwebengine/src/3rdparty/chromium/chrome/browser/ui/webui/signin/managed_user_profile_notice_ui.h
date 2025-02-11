// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_SIGNIN_MANAGED_USER_PROFILE_NOTICE_UI_H_
#define CHROME_BROWSER_UI_WEBUI_SIGNIN_MANAGED_USER_PROFILE_NOTICE_UI_H_

#include <optional>

#include "base/functional/callback.h"
#include "base/memory/raw_ptr.h"
#include "build/chromeos_buildflags.h"
#include "chrome/browser/ui/webui/signin/signin_utils.h"
#include "content/public/browser/web_ui_controller.h"
#include "third_party/skia/include/core/SkColor.h"

class Browser;
class ManagedUserProfileNoticeHandler;
struct AccountInfo;

namespace content {
class WebUI;
}

class ManagedUserProfileNoticeUI : public content::WebUIController {
 public:
  // Type of a managed user notice screen.
  enum class ScreenType {
    kEntepriseAccountSyncEnabled,
    kEntepriseAccountSyncDisabled,
    kConsumerAccountSyncDisabled,
    kEnterpriseAccountCreation,
  };

  explicit ManagedUserProfileNoticeUI(content::WebUI* web_ui);
  ~ManagedUserProfileNoticeUI() override;

  ManagedUserProfileNoticeUI(const ManagedUserProfileNoticeUI&) = delete;
  ManagedUserProfileNoticeUI& operator=(const ManagedUserProfileNoticeUI&) =
      delete;

  // Initializes the ManagedUserProfileNoticeUI, which will obtain the user's
  // choice about how to set up the profile with the new account.
  // `proceed_callback` will be called when the user performs an action to exit
  // the screen. Their choice will depend on other flags passed to this method.
  // If `profile_creation_required_by_policy` is true, the wording of the dialog
  // will tell the user that an admin requires a new profile for the account,
  // otherwise the default wording will be used.
  // `show_link_data_option` will make the screen display a checkbox, and when
  // selected, will indicate that the user wants the current profile to be used
  // as dedicated profile for the new account, linking the current data with
  // synced data from the new account.
  void Initialize(Browser* browser,
                  ScreenType type,
                  const AccountInfo& account_info,
                  bool profile_creation_required_by_policy,
                  bool show_link_data_option,
                  signin::SigninChoiceCallback proceed_callback);

  // Allows tests to trigger page events.
  ManagedUserProfileNoticeHandler* GetHandlerForTesting();

 private:
  // Stored for tests.
  raw_ptr<ManagedUserProfileNoticeHandler> handler_ = nullptr;

  WEB_UI_CONTROLLER_TYPE_DECL();
};

#endif  // CHROME_BROWSER_UI_WEBUI_SIGNIN_MANAGED_USER_PROFILE_NOTICE_UI_H_
