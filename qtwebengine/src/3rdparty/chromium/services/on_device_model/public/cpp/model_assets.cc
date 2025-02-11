// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/on_device_model/public/cpp/model_assets.h"

#include <cstdint>
#include <string_view>

#include "base/files/file.h"
#include "build/build_config.h"

namespace on_device_model {

ModelAssetPaths::ModelAssetPaths() = default;

ModelAssetPaths::ModelAssetPaths(const ModelAssetPaths&) = default;

ModelAssetPaths::~ModelAssetPaths() = default;

ModelAssets::ModelAssets() = default;

ModelAssets::ModelAssets(ModelAssets&&) = default;

ModelAssets& ModelAssets::operator=(ModelAssets&&) = default;

ModelAssets::~ModelAssets() = default;

ModelAssets LoadModelAssets(const ModelAssetPaths& paths) {
  ModelAssets assets;
  assets.sp_model =
      base::File(paths.sp_model, base::File::FLAG_OPEN | base::File::FLAG_READ);
  assets.model =
      base::File(paths.model, base::File::FLAG_OPEN | base::File::FLAG_READ);

  if (paths.HasSafetyFiles()) {
    assets.ts_data = base::File(paths.ts_data,
                                base::File::FLAG_OPEN | base::File::FLAG_READ);
    assets.ts_sp_model = base::File(
        paths.ts_sp_model, base::File::FLAG_OPEN | base::File::FLAG_READ);
  }

  if (!paths.language_detection_model.empty()) {
    assets.language_detection_model =
        base::File(paths.language_detection_model,
                   base::File::FLAG_OPEN | base::File::FLAG_READ);
  }

  // NOTE: Weights ultimately need to be mapped copy-on-write, but Fuchsia
  // (due to an apparent bug?) doesn't seem to support copy-on-write mapping of
  // file objects which are not writable. So we open as writable on Fuchsia even
  // though nothing should write through to the file.
#if BUILDFLAG(IS_FUCHSIA)
  constexpr uint32_t kWeightsFlags =
      base::File::FLAG_OPEN | base::File::FLAG_READ | base::File::FLAG_WRITE;
#else
  constexpr uint32_t kWeightsFlags =
      base::File::FLAG_OPEN | base::File::FLAG_READ;
#endif
  assets.weights = base::File(paths.weights, kWeightsFlags);
  return assets;
}

}  // namespace on_device_model
