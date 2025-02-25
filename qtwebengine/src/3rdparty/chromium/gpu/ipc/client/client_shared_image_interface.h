// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_IPC_CLIENT_CLIENT_SHARED_IMAGE_INTERFACE_H_
#define GPU_IPC_CLIENT_CLIENT_SHARED_IMAGE_INTERFACE_H_

#include <set>

#include "base/containers/flat_map.h"
#include "base/memory/raw_ptr.h"
#include "base/synchronization/lock.h"
#include "base/thread_annotations.h"
#include "build/build_config.h"
#include "gpu/command_buffer/client/shared_image_interface.h"
#include "gpu/ipc/common/surface_handle.h"

namespace gpu {
class SharedImageInterfaceProxy;
class GpuChannelHost;

// Tracks shared images created by a single context and ensures they are deleted
// if the context is lost.
class GPU_EXPORT ClientSharedImageInterface : public SharedImageInterface {
 public:
  ClientSharedImageInterface(SharedImageInterfaceProxy* proxy,
                             scoped_refptr<gpu::GpuChannelHost> channel);
  ~ClientSharedImageInterface() override;

  // SharedImageInterface implementation.
  void UpdateSharedImage(const SyncToken& sync_token,
                         const Mailbox& mailbox) override;
  void UpdateSharedImage(const SyncToken& sync_token,
                         std::unique_ptr<gfx::GpuFence> acquire_fence,
                         const Mailbox& mailbox) override;
  void PresentSwapChain(const SyncToken& sync_token,
                        const Mailbox& mailbox) override;
#if BUILDFLAG(IS_FUCHSIA)
  void RegisterSysmemBufferCollection(zx::eventpair service_handle,
                                      zx::channel sysmem_token,
                                      gfx::BufferFormat format,
                                      gfx::BufferUsage usage,
                                      bool register_with_image_pipe) override;
#endif  // BUILDFLAG(IS_FUCHSIA)
  SyncToken GenUnverifiedSyncToken() override;
  SyncToken GenVerifiedSyncToken() override;
  void WaitSyncToken(const gpu::SyncToken& sync_token) override;
  void Flush() override;
  scoped_refptr<gfx::NativePixmap> GetNativePixmap(
      const Mailbox& mailbox) override;
  scoped_refptr<ClientSharedImage> CreateSharedImage(
      viz::SharedImageFormat format,
      const gfx::Size& size,
      const gfx::ColorSpace& color_space,
      GrSurfaceOrigin surface_origin,
      SkAlphaType alpha_type,
      uint32_t usage,
      base::StringPiece debug_label,
      gpu::SurfaceHandle surface_handle) override;
  scoped_refptr<ClientSharedImage> CreateSharedImage(
      viz::SharedImageFormat format,
      const gfx::Size& size,
      const gfx::ColorSpace& color_space,
      GrSurfaceOrigin surface_origin,
      SkAlphaType alpha_type,
      uint32_t usage,
      base::StringPiece debug_label,
      base::span<const uint8_t> pixel_data) override;
  scoped_refptr<ClientSharedImage> CreateSharedImage(
      viz::SharedImageFormat format,
      const gfx::Size& size,
      const gfx::ColorSpace& color_space,
      GrSurfaceOrigin surface_origin,
      SkAlphaType alpha_type,
      uint32_t usage,
      base::StringPiece debug_label,
      gpu::SurfaceHandle surface_handle,
      gfx::BufferUsage buffer_usage) override;
  scoped_refptr<ClientSharedImage> CreateSharedImage(
      viz::SharedImageFormat format,
      const gfx::Size& size,
      const gfx::ColorSpace& color_space,
      GrSurfaceOrigin surface_origin,
      SkAlphaType alpha_type,
      uint32_t usage,
      base::StringPiece debug_label,
      gpu::SurfaceHandle surface_handle,
      gfx::BufferUsage buffer_usage,
      gfx::GpuMemoryBufferHandle buffer_handle) override;
  scoped_refptr<ClientSharedImage> CreateSharedImage(
      viz::SharedImageFormat format,
      const gfx::Size& size,
      const gfx::ColorSpace& color_space,
      GrSurfaceOrigin surface_origin,
      SkAlphaType alpha_type,
      uint32_t usage,
      base::StringPiece debug_label,
      gfx::GpuMemoryBufferHandle buffer_handle) override;

  // Used by the software compositor only. |useage| must be
  // gpu::SHARED_IMAGE_USAGE_CPU_WRITE. Call client_shared_image->Map() later to
  // get the shared memory mapping.
  scoped_refptr<ClientSharedImage> CreateSharedImage(
      viz::SharedImageFormat format,
      const gfx::Size& size,
      const gfx::ColorSpace& color_space,
      GrSurfaceOrigin surface_origin,
      SkAlphaType alpha_type,
      uint32_t usage,
      base::StringPiece debug_label) override;

  // NOTE: The below method is DEPRECATED for `gpu_memory_buffer` only with
  // single planar eg. RGB BufferFormats. Please use the equivalent method above
  // taking in single planar SharedImageFormat with GpuMemoryBufferHandle.
  scoped_refptr<ClientSharedImage> CreateSharedImage(
      gfx::GpuMemoryBuffer* gpu_memory_buffer,
      GpuMemoryBufferManager* gpu_memory_buffer_manager,
      gfx::BufferPlane plane,
      const gfx::ColorSpace& color_space,
      GrSurfaceOrigin surface_origin,
      SkAlphaType alpha_type,
      uint32_t usage,
      base::StringPiece debug_label) override;
#if BUILDFLAG(IS_WIN)
  void CopyToGpuMemoryBuffer(const SyncToken& sync_token,
                             const Mailbox& mailbox) override;
  void UpdateSharedImage(const SyncToken& sync_token,
                         scoped_refptr<gfx::D3DSharedFence> d3d_shared_fence,
                         const Mailbox& mailbox) override;
#endif
  SwapChainSharedImages CreateSwapChain(viz::SharedImageFormat format,
                                        const gfx::Size& size,
                                        const gfx::ColorSpace& color_space,
                                        GrSurfaceOrigin surface_origin,
                                        SkAlphaType alpha_type,
                                        uint32_t usage) override;
  void DestroySharedImage(const SyncToken& sync_token,
                          const Mailbox& mailbox) override;
  void DestroySharedImage(
      const SyncToken& sync_token,
      scoped_refptr<ClientSharedImage> client_shared_image) override;
  uint32_t UsageForMailbox(const Mailbox& mailbox) override;
  scoped_refptr<ClientSharedImage> NotifyMailboxAdded(const Mailbox& mailbox,
                                                      uint32_t usage) override;

  scoped_refptr<ClientSharedImage> AddReferenceToSharedImage(
      const SyncToken& sync_token,
      const Mailbox& mailbox,
      uint32_t usage) override;

  const SharedImageCapabilities& GetCapabilities() override;

  gpu::GpuChannelHost* gpu_channel() { return gpu_channel_.get(); }

 private:
  scoped_refptr<gpu::GpuChannelHost> gpu_channel_;

  Mailbox AddMailbox(const Mailbox& mailbox);

  const raw_ptr<SharedImageInterfaceProxy> proxy_;

  base::Lock lock_;
  std::multiset<Mailbox> mailboxes_ GUARDED_BY(lock_);
};

}  // namespace gpu

#endif  // GPU_IPC_CLIENT_CLIENT_SHARED_IMAGE_INTERFACE_H_
