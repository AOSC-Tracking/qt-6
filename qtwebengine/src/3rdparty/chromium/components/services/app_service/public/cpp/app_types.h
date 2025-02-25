// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_SERVICES_APP_SERVICE_PUBLIC_CPP_APP_TYPES_H_
#define COMPONENTS_SERVICES_APP_SERVICE_PUBLIC_CPP_APP_TYPES_H_

#include <string>
#include <utility>
#include <vector>

#include "base/component_export.h"
#include "base/time/time.h"
#include "base/values.h"
#include "components/services/app_service/public/cpp/icon_types.h"
#include "components/services/app_service/public/cpp/intent_filter.h"
#include "components/services/app_service/public/cpp/macros.h"
#include "components/services/app_service/public/cpp/permission.h"
#include "components/services/app_service/public/cpp/run_on_os_login_types.h"
#include "components/services/app_service/public/protos/app_types.pb.h"
#include "third_party/abseil-cpp/absl/types/optional.h"

namespace apps {

// When updating the enum below, update
// //components/services/app_service/public/cpp/macros.h
// macros if necessary, as well as the ApplicationType enum in
// //components/services/app_service/public/protos/app_types.proto.
ENUM(AppType,
     kUnknown,
     kArc,                         // Android app.
     kBuiltIn,                     // Built-in app.
     kCrostini,                    // Linux (via Crostini) app.
     kChromeApp,                   // Chrome app.
     kWeb,                         // Web app.
     kPluginVm,                    // Plugin VM app, see go/pluginvm.
     kStandaloneBrowser,           // Lacros browser app, see //docs/lacros.md.
     kRemote,                      // Remote app.
     kBorealis,                    // Borealis app, see go/borealis-app.
     kSystemWeb,                   // System web app.
     kStandaloneBrowserChromeApp,  // Chrome app hosted in Lacros.
     kExtension,                   // Browser extension.
     kStandaloneBrowserExtension,  // Extension hosted in Lacros.
     kBruschetta                   // Bruschetta app, see go/bruschetta.
)

// Whether an app is ready to launch, i.e. installed.
// Note the enumeration is used in UMA histogram so entries should not be
// re-ordered or removed. New entries should be added at the bottom.
ENUM(Readiness,
     kUnknown,
     kReady,                // Installed and launchable.
     kDisabledByBlocklist,  // Disabled by SafeBrowsing.
     kDisabledByPolicy,     // Disabled by admin policy.
     kDisabledByUser,       // Disabled by explicit user action.
     kTerminated,           // Renderer process crashed.
     kUninstalledByUser,
     // Removed apps are purged from the registry cache and have their
     // associated memory freed. Subscribers are not notified of removed
     // apps, so publishers must set the app as uninstalled before
     // removing it.
     kRemoved,
     // This is used for all non-user initiated uninstallation.
     kUninstalledByNonUser)

// How the app was installed.
// This should be kept in sync with histograms.xml, InstallReason in
// enums.xml as well as ApplicationInstallReason in
// //components/services/app_service/public/protos/app_types.proto.
//
// Note the enumeration is used in UMA histogram so entries should not be
// re-ordered or removed. New entries should be added at the bottom.
ENUM(InstallReason,
     kUnknown,
     kSystem,   // Installed with the system and is considered a part of the OS.
     kPolicy,   // Installed by policy.
     kOem,      // Installed by an OEM.
     kDefault,  // Preinstalled by default, but is not considered a system app.
     kSync,     // Installed by sync.
     kUser,     // Installed by user action.
     kSubApp,   // Installed by the SubApp API call.
     kKiosk,    // Installed by Kiosk on Chrome OS.
     kCommandLine  // Installed by command line argument.
)

// Where the app was installed from.
// This should be kept in sync with histograms.xml, InstallSource in
// enums.xml as well as ApplicationInstallSource in
// //components/services/app_service/public/protos/app_types.proto.
//
// Note the enumeration is used in UMA histogram so entries should not be
// re-ordered or removed. New entries should be added at the bottom.
ENUM(InstallSource,
     kUnknown,
     kSystem,          // Installed as part of Chrome OS.
     kSync,            // Installed from sync.
     kPlayStore,       // Installed from Play store.
     kChromeWebStore,  // Installed from Chrome web store.
     kBrowser          // Installed from browser.
)

// What caused the app to be uninstalled.
// This should be kept in sync with UninstallSource in enums.xml as well as
// ApplicationUninstallSource in
// //components/services/app_service/public/protos/app_types.proto, so entries
// should not be re-ordered or removed. New entries should be added at the
// bottom.
ENUM(UninstallSource,
     kUnknown,
     kAppList,        // Uninstall by the user from the App List (Launcher)
     kAppManagement,  // Uninstall by the user from the App Management page
     kShelf,          // Uninstall by the user from the Shelf
     kMigration       // Uninstall by app migration.
)

// The window mode that each app will open in.
ENUM(WindowMode,
     kUnknown,
     // Opens in a standalone window
     kWindow,
     // Opens in the default web browser
     kBrowser,
     // Opens in a tabbed app window
     kTabbedWindow)

// Information about an app. See components/services/app_service/README.md.
struct COMPONENT_EXPORT(APP_TYPES) App {
  App(AppType app_type, const std::string& app_id);

  App(const App&) = delete;
  App& operator=(const App&) = delete;

  ~App();

  bool operator==(const App& other) const;
  bool operator!=(const App& other) const;

  std::unique_ptr<App> Clone() const;

  // Adds a new field for `extra`. The type `T` can be any type, e.g. int,
  // double, string, base::Value::Dict, base::Value::List, base::Value, etc. The
  // value is saved in base::Value::Dict `extra`. If the type `T` can't be
  // converted to base::Value, an explicit convert function can be added to
  // convert `value` to base::Value.
  template <typename T>
  void SetExtraField(const std::string& field_name, T&& value) {
    if (!extra.has_value()) {
      extra = base::Value::Dict();
    }
    extra->Set(field_name, value);
  }

  AppType app_type;
  std::string app_id;

  Readiness readiness = Readiness::kUnknown;

  // The full name of the app. Will be used in most UIs.
  absl::optional<std::string> name;
  // A shortened version of the app name. May omit branding (e.g.
  // "Google" prefixes) or rely on abbreviations (e.g. "YT Music"). If no
  // `short_name` is supplied, the `name` will be used instead.
  // The `short_name` may be used in UIs where space is limited and/or we want
  // to optimize for scannability.
  absl::optional<std::string> short_name;

  // An optional, publisher-specific ID for this app, e.g. for Android apps,
  // this field contains the Android package name, and for web apps, it
  // contains the start URL.
  absl::optional<std::string> publisher_id;

  absl::optional<std::string> description;
  absl::optional<std::string> version;
  std::vector<std::string> additional_search_terms;

  absl::optional<IconKey> icon_key;

  absl::optional<base::Time> last_launch_time;
  absl::optional<base::Time> install_time;

  // This vector must be treated atomically, if there is a permission
  // change, the publisher must send through the entire list of permissions.
  // Should contain no duplicate IDs.
  // If empty during updates, Subscriber can assume no changes.
  // There is no guarantee that this is sorted by any criteria.
  Permissions permissions;

  // The main reason why this app is currently installed on the device (e.g.
  // because it is required by Policy). This may change over time and is not
  // necessarily the reason why the app was originally installed.
  InstallReason install_reason = InstallReason::kUnknown;

  // How installation of the app was triggered on this device. Either a UI
  // surface (e.g. Play Store), or a system component (e.g. Sync).
  InstallSource install_source = InstallSource::kUnknown;

  // IDs used for policy to identify the app.
  // For web apps, it contains the install URL(s).
  std::vector<std::string> policy_ids;

  // Whether the app is an extensions::Extensions where is_platform_app()
  // returns true.
  absl::optional<bool> is_platform_app;

  absl::optional<bool> recommendable;
  absl::optional<bool> searchable;
  absl::optional<bool> show_in_launcher;
  absl::optional<bool> show_in_shelf;
  absl::optional<bool> show_in_search;
  absl::optional<bool> show_in_management;

  // True if the app is able to handle intents and should be shown in intent
  // surfaces.
  absl::optional<bool> handles_intents;

  // Whether the app publisher allows the app to be uninstalled.
  absl::optional<bool> allow_uninstall;

  // Whether the app icon should add the notification badging.
  absl::optional<bool> has_badge;

  // Paused apps cannot be launched, and any running apps that become paused
  // will be stopped. This is independent of whether or not the app is ready to
  // be launched (defined by the Readiness field).
  absl::optional<bool> paused;

  // This vector stores all the intent filters defined in this app. Each
  // intent filter defines a matching criteria for whether an intent can
  // be handled by this app. One app can have multiple intent filters.
  IntentFilters intent_filters;

  // Whether the app can be free resized. If this is true, various resizing
  // operations will be restricted.
  absl::optional<bool> resize_locked;

  // Whether the app's display mode is in the browser or otherwise.
  WindowMode window_mode = WindowMode::kUnknown;

  // Whether the app runs on os login in a new window or not.
  absl::optional<RunOnOsLogin> run_on_os_login;

  // Whether the app can be closed by the user.
  absl::optional<bool> allow_close;

  // Storage space size for app and associated data.
  absl::optional<uint64_t> app_size_in_bytes;
  absl::optional<uint64_t> data_size_in_bytes;

  // App-specified supported locales.
  std::vector<std::string> supported_locales;
  // Currently selected locale, empty string means system language is used.
  // ARC-specific note: Based on Android implementation, `selected_locale`
  //  is not necessarily part of `supported_locales`.
  absl::optional<std::string> selected_locale;

  // The extra information used by the app platform(e.g. ARC, GuestOS) for an
  // app. `extra` needs to be modified as a whole, and we can't only modify part
  // of `extra`. AppService doesn't use the fields saved in `extra`. App
  // publishers modify the content saved in `extra`.
  absl::optional<base::Value::Dict> extra;

  // When adding new fields to the App type, the `Clone` function, the
  // `operator==` function, and the `AppUpdate` class should also be updated. If
  // the new fields should be saved, below functions should be updated:
  // `AppStorage::IsAppChanged`
  // `AppStorageFileHandler::ConvertAppsToValue`
  // `AppStorageFileHandler::ConvertValueToApps`
};

using AppPtr = std::unique_ptr<App>;

COMPONENT_EXPORT(APP_TYPES)
bool IsEqual(const std::vector<AppPtr>& source,
             const std::vector<AppPtr>& target);

COMPONENT_EXPORT(APP_TYPES)
ApplicationType ConvertAppTypeToProtoApplicationType(AppType app_type);

COMPONENT_EXPORT(APP_TYPES)
ApplicationInstallReason ConvertInstallReasonToProtoApplicationInstallReason(
    InstallReason install_reason);

COMPONENT_EXPORT(APP_TYPES)
ApplicationInstallSource ConvertInstallSourceToProtoApplicationInstallSource(
    InstallSource install_source);

COMPONENT_EXPORT(APP_TYPES)
ApplicationUninstallSource
ConvertUninstallSourceToProtoApplicationUninstallSource(
    UninstallSource uninstall_source);

}  // namespace apps

#endif  // COMPONENTS_SERVICES_APP_SERVICE_PUBLIC_CPP_APP_TYPES_H_
