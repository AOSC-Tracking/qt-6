// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/enterprise/client_certificates/core/client_identity.h"

#include <utility>

#include "components/enterprise/client_certificates/core/private_key.h"
#include "net/cert/x509_certificate.h"

namespace client_certificates {

ClientIdentity::ClientIdentity(
    scoped_refptr<PrivateKey> private_key,
    scoped_refptr<net::X509Certificate> client_certificate)
    : private_key(std::move(private_key)),
      client_certificate(std::move(client_certificate)) {}

ClientIdentity::~ClientIdentity() = default;

}  // namespace client_certificates
