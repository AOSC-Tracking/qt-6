// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/modules/xr/xr_plane.h"

#include "third_party/blink/renderer/bindings/core/v8/frozen_array.h"
#include "third_party/blink/renderer/core/dom/dom_exception.h"
#include "third_party/blink/renderer/modules/xr/vr_service_type_converters.h"
#include "third_party/blink/renderer/modules/xr/xr_object_space.h"
#include "third_party/blink/renderer/modules/xr/xr_reference_space.h"
#include "third_party/blink/renderer/modules/xr/xr_session.h"

namespace blink {

XRPlane::XRPlane(uint64_t id,
                 XRSession* session,
                 const device::mojom::blink::XRPlaneData& plane_data,
                 double timestamp)
    : XRPlane(id,
              session,
              mojo::TypeConverter<absl::optional<blink::XRPlane::Orientation>,
                                  device::mojom::blink::XRPlaneOrientation>::
                  Convert(plane_data.orientation),
              mojo::TypeConverter<
                  blink::HeapVector<blink::Member<blink::DOMPointReadOnly>>,
                  Vector<device::mojom::blink::XRPlanePointDataPtr>>::
                  Convert(plane_data.polygon),
              plane_data.mojo_from_plane,
              timestamp) {}

XRPlane::XRPlane(uint64_t id,
                 XRSession* session,
                 const absl::optional<Orientation>& orientation,
                 HeapVector<Member<DOMPointReadOnly>> polygon,
                 const absl::optional<device::Pose>& mojo_from_plane,
                 double timestamp)
    : id_(id),
      polygon_(MakeGarbageCollected<FrozenArray<DOMPointReadOnly>>(
          std::move(polygon))),
      orientation_(orientation),
      mojo_from_plane_(mojo_from_plane),
      session_(session),
      last_changed_time_(timestamp) {
  DVLOG(3) << __func__;
}

uint64_t XRPlane::id() const {
  return id_;
}

XRSpace* XRPlane::planeSpace() const {
  if (!plane_space_) {
    plane_space_ = MakeGarbageCollected<XRObjectSpace<XRPlane>>(session_, this);
  }

  return plane_space_.Get();
}

absl::optional<gfx::Transform> XRPlane::MojoFromObject() const {
  if (!mojo_from_plane_) {
    return absl::nullopt;
  }

  return mojo_from_plane_->ToTransform();
}

device::mojom::blink::XRNativeOriginInformationPtr XRPlane::NativeOrigin()
    const {
  return device::mojom::blink::XRNativeOriginInformation::NewPlaneId(
      this->id());
}

String XRPlane::orientation() const {
  if (orientation_) {
    switch (*orientation_) {
      case Orientation::kHorizontal:
        return "Horizontal";
      case Orientation::kVertical:
        return "Vertical";
    }
  }
  return "";
}

double XRPlane::lastChangedTime() const {
  return last_changed_time_;
}

const FrozenArray<DOMPointReadOnly>& XRPlane::polygon() const {
  return *polygon_.Get();
}

void XRPlane::Update(const device::mojom::blink::XRPlaneData& plane_data,
                     double timestamp) {
  DVLOG(3) << __func__;

  last_changed_time_ = timestamp;

  orientation_ =
      mojo::TypeConverter<absl::optional<blink::XRPlane::Orientation>,
                          device::mojom::blink::XRPlaneOrientation>::
          Convert(plane_data.orientation);

  mojo_from_plane_ = plane_data.mojo_from_plane;

  polygon_ = MakeGarbageCollected<FrozenArray<DOMPointReadOnly>>(
      mojo::TypeConverter<
          blink::HeapVector<blink::Member<blink::DOMPointReadOnly>>,
          Vector<device::mojom::blink::XRPlanePointDataPtr>>::
          Convert(plane_data.polygon));
}

void XRPlane::Trace(Visitor* visitor) const {
  visitor->Trace(polygon_);
  visitor->Trace(session_);
  visitor->Trace(plane_space_);
  ScriptWrappable::Trace(visitor);
}

}  // namespace blink
