// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/trusted_vault/recovery_key_store_connection_impl.h"

#include <memory>

#include "base/notreached.h"
#include "base/time/time.h"
#include "components/signin/public/identity_manager/account_info.h"
#include "components/trusted_vault/proto/recovery_key_store.pb.h"
#include "components/trusted_vault/trusted_vault_access_token_fetcher.h"
#include "components/trusted_vault/trusted_vault_connection.h"
#include "components/trusted_vault/trusted_vault_request.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"
#include "url/gurl.h"

namespace trusted_vault {
namespace {

constexpr char kUpdateVaultUrl[] =
    "https://cryptauthvault.googleapis.com/v1/vaults/";

void ProcessUpdateVaultResponseResponse(
    RecoveryKeyStoreConnection::UpdateRecoveryKeyStoreCallback callback,
    TrustedVaultRequest::HttpStatus http_status,
    const std::string& response_body) {
  switch (http_status) {
    case TrustedVaultRequest::HttpStatus::kSuccess:
      std::move(callback).Run(UpdateRecoveryKeyStoreStatus::kSuccess);
      return;
    case TrustedVaultRequest::HttpStatus::kBadRequest:
    case TrustedVaultRequest::HttpStatus::kNotFound:
    case TrustedVaultRequest::HttpStatus::kConflict:
    case TrustedVaultRequest::HttpStatus::kOtherError:
      std::move(callback).Run(UpdateRecoveryKeyStoreStatus::kOtherError);
      return;
    case TrustedVaultRequest::HttpStatus::kNetworkError:
      std::move(callback).Run(UpdateRecoveryKeyStoreStatus::kNetworkError);
      return;
    case TrustedVaultRequest::HttpStatus::kTransientAccessTokenFetchError:
      std::move(callback).Run(
          UpdateRecoveryKeyStoreStatus::kTransientAccessTokenFetchError);
      return;
    case TrustedVaultRequest::HttpStatus::kPersistentAccessTokenFetchError:
      std::move(callback).Run(
          UpdateRecoveryKeyStoreStatus::kPersistentAccessTokenFetchError);
      return;
    case TrustedVaultRequest::HttpStatus::
        kPrimaryAccountChangeAccessTokenFetchError:
      std::move(callback).Run(UpdateRecoveryKeyStoreStatus::
                                  kPrimaryAccountChangeAccessTokenFetchError);
      return;
  }

  NOTREACHED_NORETURN();
}
}  // namespace

RecoveryKeyStoreConnectionImpl::RecoveryKeyStoreConnectionImpl(
    scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory,
    std::unique_ptr<TrustedVaultAccessTokenFetcher> access_token_fetcher)
    : url_loader_factory_(url_loader_factory),
      access_token_fetcher_(std::move(access_token_fetcher)) {}

RecoveryKeyStoreConnectionImpl::~RecoveryKeyStoreConnectionImpl() = default;

std::unique_ptr<RecoveryKeyStoreConnectionImpl::Request>
RecoveryKeyStoreConnectionImpl::UpdateRecoveryKeyStore(
    const CoreAccountInfo& account_info,
    const trusted_vault_pb::UpdateVaultRequest& update_vault_request,
    UpdateRecoveryKeyStoreCallback callback) {
  auto request = std::make_unique<TrustedVaultRequest>(
      account_info.account_id, TrustedVaultRequest::HttpMethod::kPatch,
      GURL(kUpdateVaultUrl), update_vault_request.SerializeAsString(),
      /*max_retry_duration=*/base::Seconds(0), url_loader_factory_,
      access_token_fetcher_->Clone(),
      // TODO(crbug.com/1495928): Add a separate metric for RecoveryKeyStore URL
      // fetches.
      TrustedVaultURLFetchReasonForUMA::kUnspecified);
  request->FetchAccessTokenAndSendRequest(
      base::BindOnce(&ProcessUpdateVaultResponseResponse, std::move(callback)));
  return request;
}

}  // namespace trusted_vault
