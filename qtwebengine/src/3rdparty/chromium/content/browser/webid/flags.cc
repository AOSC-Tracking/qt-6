// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flags.h"

#include "base/command_line.h"
#include "base/metrics/field_trial_params.h"
#include "content/common/features.h"
#include "content/public/common/content_features.h"
#include "content/public/common/content_switches.h"

namespace content {

bool IsFedCmAuthzEnabled() {
  return base::FeatureList::IsEnabled(features::kFedCmAuthz);
}

bool IsFedCmMultipleIdentityProvidersEnabled() {
  return base::FeatureList::IsEnabled(
      features::kFedCmMultipleIdentityProviders);
}

FedCmIdpSigninStatusMode GetFedCmIdpSigninStatusFlag() {
  if (base::FeatureList::IsEnabled(features::kFedCmIdpSigninStatusEnabled)) {
    return FedCmIdpSigninStatusMode::ENABLED;
  }
  if (base::FeatureList::IsEnabled(features::kFedCmIdpSigninStatusMetrics)) {
    return FedCmIdpSigninStatusMode::METRICS_ONLY;
  }
  return FedCmIdpSigninStatusMode::DISABLED;
}

bool IsFedCmMetricsEndpointEnabled() {
  return base::FeatureList::IsEnabled(features::kFedCmMetricsEndpoint);
}

bool IsFedCmSelectiveDisclosureEnabled() {
  return base::FeatureList::IsEnabled(features::kFedCmSelectiveDisclosure);
}

bool IsFedCmSkipWellKnownForSameSiteEnabled() {
  return base::FeatureList::IsEnabled(features::kFedCmSkipWellKnownForSameSite);
}

bool IsFedCmIdPRegistrationEnabled() {
  return base::FeatureList::IsEnabled(features::kFedCmIdPRegistration);
}

bool IsFedCmWithoutWellKnownEnforcementEnabled() {
  return base::FeatureList::IsEnabled(
      features::kFedCmWithoutWellKnownEnforcement);
}

bool IsWebIdentityDigitalCredentialsEnabled() {
  return base::FeatureList::IsEnabled(features::kWebIdentityDigitalCredentials);
}

bool IsFedCmAutoSelectedFlagEnabled() {
  return base::FeatureList::IsEnabled(features::kFedCmAutoSelectedFlag);
}

bool IsFedCmDomainHintEnabled() {
  return base::FeatureList::IsEnabled(features::kFedCmDomainHint);
}

bool IsFedCmErrorEnabled() {
  return base::FeatureList::IsEnabled(features::kFedCmError);
}

bool IsFedCmDisconnectEnabled() {
  return base::FeatureList::IsEnabled(features::kFedCmDisconnect);
}

bool IsFedCmAddAccountEnabled() {
  return base::FeatureList::IsEnabled(features::kFedCmAddAccount);
}

bool IsFedCmExemptIdpWithThirdPartyCookiesEnabled() {
  return base::FeatureList::IsEnabled(
      features::kFedCmExemptIdpWithThirdPartyCookies);
}

bool IsFedCmButtonModeEnabled() {
  return base::FeatureList::IsEnabled(features::kFedCmButtonMode);
}

bool IsFedCmSameSiteLoginStatusEnabled() {
  return base::FeatureList::IsEnabled(features::kFedCmSameSiteLoginStatus);
}

}  // namespace content
