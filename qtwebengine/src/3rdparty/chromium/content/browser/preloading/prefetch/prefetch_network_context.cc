// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/preloading/prefetch/prefetch_network_context.h"

#include <optional>

#include "base/command_line.h"
#include "base/memory/scoped_refptr.h"
#include "content/browser/preloading/prefetch/prefetch_network_context_client.h"
#include "content/browser/preloading/prefetch/prefetch_proxy_configurator.h"
#include "content/browser/preloading/prefetch/prefetch_service.h"
#include "content/browser/preloading/prefetch/prefetch_type.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/child_process_host.h"
#include "content/public/browser/content_browser_client.h"
#include "content/public/browser/global_routing_id.h"
#include "content/public/browser/network_service_instance.h"
#include "content/public/browser/prefetch_service_delegate.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/common/content_client.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/user_agent.h"
#include "mojo/public/cpp/bindings/self_owned_receiver.h"
#include "net/base/isolation_info.h"
#include "services/cert_verifier/public/mojom/cert_verifier_service_factory.mojom.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"
#include "services/network/public/cpp/wrapper_shared_url_loader_factory.h"
#include "services/network/public/mojom/cookie_manager.mojom.h"
#include "services/network/public/mojom/network_context.mojom.h"
#include "services/network/public/mojom/url_loader_factory.mojom.h"

namespace content {

PrefetchNetworkContext::PrefetchNetworkContext(
    bool use_isolated_network_context,
    const PrefetchType& prefetch_type,
    const blink::mojom::Referrer& referrer_,
    const GlobalRenderFrameHostId& referring_render_frame_host_id)
    : use_isolated_network_context_(use_isolated_network_context),
      prefetch_type_(prefetch_type),
      referrer_(referrer_),
      referring_render_frame_host_id_(referring_render_frame_host_id) {}

PrefetchNetworkContext::~PrefetchNetworkContext() = default;

network::mojom::URLLoaderFactory* PrefetchNetworkContext::GetURLLoaderFactory(
    PrefetchService* service) {
  if (!url_loader_factory_) {
    if (use_isolated_network_context_) {
      CreateIsolatedURLLoaderFactory(service);
      CHECK(network_context_);
    } else {
      // Create new URL factory in the default network context.
      mojo::PendingRemote<network::mojom::URLLoaderFactory> url_factory_remote;
      CreateNewURLLoaderFactory(
          service->GetBrowserContext(),
          service->GetBrowserContext()
              ->GetDefaultStoragePartition()
              ->GetNetworkContext(),
          url_factory_remote.InitWithNewPipeAndPassReceiver(), std::nullopt);
      url_loader_factory_ = network::SharedURLLoaderFactory::Create(
          std::make_unique<network::WrapperPendingSharedURLLoaderFactory>(
              std::move(url_factory_remote)));
    }
  }
  CHECK(url_loader_factory_);
  return url_loader_factory_.get();
}

network::mojom::CookieManager* PrefetchNetworkContext::GetCookieManager() {
  CHECK(use_isolated_network_context_);
  CHECK(network_context_);
  if (!cookie_manager_)
    network_context_->GetCookieManager(
        cookie_manager_.BindNewPipeAndPassReceiver());

  return cookie_manager_.get();
}

void PrefetchNetworkContext::CloseIdleConnections() {
  if (network_context_)
    network_context_->CloseIdleConnections(base::DoNothing());
}

void PrefetchNetworkContext::CreateIsolatedURLLoaderFactory(
    PrefetchService* service) {
  CHECK(use_isolated_network_context_);

  network_context_.reset();
  url_loader_factory_.reset();

  PrefetchServiceDelegate* delegate = service->GetPrefetchServiceDelegate();

  auto context_params = network::mojom::NetworkContextParams::New();
  context_params->file_paths = network::mojom::NetworkContextFilePaths::New();
  context_params->user_agent =
      GetReducedUserAgent(base::CommandLine::ForCurrentProcess()->HasSwitch(
                              switches::kUseMobileUserAgent),
                          delegate ? delegate->GetMajorVersionNumber() : "");
  // The verifier created here does not have the same parameters as used in the
  // profile (where additional parameters are added in
  // chrome/browser/net/profile_network_context_service.h
  // ProfileNetworkContextService::ConfigureNetworkContextParamsInternal, as
  // well as updates in ProfileNetworkContextService::UpdateCertificatePolicy).
  //
  // Currently this does not cause problems as additional parameters only ensure
  // more requests validate, so the only harm is that prefetch requests will
  // fail and then later succeed when they are actually fetched. In the future
  // when additional parameters can cause validations to fail, this will cause
  // problems.
  //
  // TODO(crbug.com/1477317): figure out how to get this verifier in sync with
  // the profile verifier.
  context_params->cert_verifier_params = GetCertVerifierParams(
      cert_verifier::mojom::CertVerifierCreationParams::New());
  context_params->cors_exempt_header_list = {kCorsExemptPurposeHeaderName};
  context_params->cookie_manager_params =
      network::mojom::CookieManagerParams::New();

  if (delegate) {
    context_params->accept_language = delegate->GetAcceptLanguageHeader();
  }

  context_params->http_cache_enabled = true;
  CHECK(!context_params->file_paths->http_cache_directory);

  if (prefetch_type_.IsProxyRequiredWhenCrossOrigin() &&
      !prefetch_type_.IsProxyBypassedForTesting()) {
    PrefetchProxyConfigurator* prefetch_proxy_configurator =
        service->GetPrefetchProxyConfigurator();
    CHECK(prefetch_proxy_configurator);

    context_params->initial_custom_proxy_config =
        prefetch_proxy_configurator->CreateCustomProxyConfig();
    context_params->custom_proxy_connection_observer_remote =
        prefetch_proxy_configurator->NewProxyConnectionObserverRemote();

    // Register a client config receiver so that updates to the set of proxy
    // hosts or proxy headers will be updated.
    mojo::Remote<network::mojom::CustomProxyConfigClient> config_client;
    context_params->custom_proxy_config_client_receiver =
        config_client.BindNewPipeAndPassReceiver();
    prefetch_proxy_configurator->AddCustomProxyConfigClient(
        std::move(config_client), base::DoNothing());
  }

  // Explicitly disallow network service features which could cause a privacy
  // leak.
  context_params->enable_certificate_reporting = false;
  context_params->enable_domain_reliability = false;

  CreateNetworkContextInNetworkService(
      network_context_.BindNewPipeAndPassReceiver(), std::move(context_params));

  if (prefetch_type_.IsProxyRequiredWhenCrossOrigin() &&
      !prefetch_type_.IsProxyBypassedForTesting()) {
    // Configure a context client to ensure Web Reports and other privacy leak
    // surfaces won't be enabled.
    mojo::PendingRemote<network::mojom::NetworkContextClient> client_remote;
    mojo::MakeSelfOwnedReceiver(
        std::make_unique<PrefetchNetworkContextClient>(),
        client_remote.InitWithNewPipeAndPassReceiver());
    network_context_->SetClient(std::move(client_remote));
  }

  mojo::PendingRemote<network::mojom::URLLoaderFactory> isolated_factory_remote;

  CreateNewURLLoaderFactory(
      service->GetBrowserContext(), network_context_.get(),
      isolated_factory_remote.InitWithNewPipeAndPassReceiver(), std::nullopt);
  url_loader_factory_ = network::SharedURLLoaderFactory::Create(
      std::make_unique<network::WrapperPendingSharedURLLoaderFactory>(
          std::move(isolated_factory_remote)));
}

void PrefetchNetworkContext::CreateNewURLLoaderFactory(
    BrowserContext* browser_context,
    network::mojom::NetworkContext* network_context,
    mojo::PendingReceiver<network::mojom::URLLoaderFactory> pending_receiver,
    std::optional<net::IsolationInfo> isolation_info) {
  CHECK(network_context);

  auto factory_params = network::mojom::URLLoaderFactoryParams::New();
  factory_params->process_id = network::mojom::kBrowserProcessId;
  factory_params->is_trusted = true;
  factory_params->is_corb_enabled = false;
  if (isolation_info) {
    factory_params->isolation_info = *isolation_info;
  }

  // Prerender should not trigger any prefetch. This assumption is needed to
  // call GetPageUkmSourceId.
  RenderFrameHost* referring_render_frame_host =
      RenderFrameHost::FromID(referring_render_frame_host_id_);
  CHECK(!referring_render_frame_host->IsInLifecycleState(
      RenderFrameHost::LifecycleState::kPrerendering));

  // Call WillCreateURLLoaderFactory so that Extensions (and other features) can
  // proxy the URLLoaderFactory pipe.
  mojo::PendingRemote<network::mojom::TrustedURLLoaderHeaderClient>
      header_client;
  bool bypass_redirect_checks = false;
  GetContentClient()->browser()->WillCreateURLLoaderFactory(
      browser_context, referring_render_frame_host,
      referring_render_frame_host->GetProcess()->GetID(),
      ContentBrowserClient::URLLoaderFactoryType::kPrefetch,
      url::Origin::Create(referrer_.url),
      /*navigation_id=*/std::nullopt,
      ukm::SourceIdObj::FromInt64(
          referring_render_frame_host->GetPageUkmSourceId()),
      &pending_receiver, &header_client, &bypass_redirect_checks,
      /*disable_secure_dns=*/nullptr, /*factory_override=*/nullptr,
      /*navigation_response_task_runner=*/nullptr);

  if (header_client.is_valid()) {
    factory_params->header_client = std::move(header_client);
  }

  network_context->CreateURLLoaderFactory(std::move(pending_receiver),
                                          std::move(factory_params));
}

}  // namespace content
