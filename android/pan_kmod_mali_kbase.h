/* SPDX-License-Identifier: MIT */
/*
 * pan_kmod_mali_kbase.h
 *
 * mali_kbase backend for Mesa's pan_kmod kernel abstraction layer.
 *
 * Mesa's panvk (Panfrost Vulkan driver) communicates with the kernel
 * through pan_kmod, which is a thin abstraction layer with two
 * existing backends:
 *   - panthor/   → talks to Panthor DRM (Linux mainline, open source)
 *   - kbase/     → talks to mali_kbase (Android, ARM proprietary kernel)
 *                  ← THIS FILE implements this backend
 *
 * By implementing pan_kmod with mali_kbase ioctls, we get the ENTIRE
 * panvk Vulkan stack (shader compiler, all vkCmd* functions, render
 * passes, compute, etc.) running on Android's existing Mali kernel driver.
 *
 * Drop into: src/panfrost/kmod/kbase/
 */

#ifndef _PAN_KMOD_MALI_KBASE_H_
#define _PAN_KMOD_MALI_KBASE_H_

#include "pan_kmod.h"
#include "../mali_kbase_iface.h"
#include "../mali_kbase_device.h"

/* ------------------------------------------------------------------ */
/* Extended device struct (embeds pan_kmod_dev)                        */
/* ------------------------------------------------------------------ */

struct pan_kmod_kbase_dev {
    struct pan_kmod_dev     base;       /* MUST be first */
    struct mali_device      mali;       /* /dev/mali0 connection */

    /* GPU timeline for synchronization */
    uint64_t                next_seqno;
    pthread_mutex_t         seqno_lock;
};

/* ------------------------------------------------------------------ */
/* Extended BO struct (embeds pan_kmod_bo)                             */
/* ------------------------------------------------------------------ */

struct pan_kmod_kbase_bo {
    struct pan_kmod_bo      base;       /* MUST be first */
    struct mali_bo          mali_bo;    /* underlying allocation */
    uint32_t                kbase_handle; /* kernel BO handle for GPU mapping */
};

/* ------------------------------------------------------------------ */
/* Extended VM struct (embeds pan_kmod_vm)                             */
/* ------------------------------------------------------------------ */

struct pan_kmod_kbase_vm {
    struct pan_kmod_vm      base;       /* MUST be first */
    /* mali_kbase manages VA space implicitly per-context,
     * so we just track our range here */
    uint64_t                va_start;
    uint64_t                va_end;
    struct mali_queue_group render_group;
    struct mali_queue_group compute_group;
};

/* ------------------------------------------------------------------ */
/* Helpers                                                             */
/* ------------------------------------------------------------------ */

static inline struct pan_kmod_kbase_dev *
to_kbase_dev(struct pan_kmod_dev *dev)
{
    return (struct pan_kmod_kbase_dev *)dev;
}

static inline struct pan_kmod_kbase_bo *
to_kbase_bo(struct pan_kmod_bo *bo)
{
    return (struct pan_kmod_kbase_bo *)bo;
}

static inline struct pan_kmod_kbase_vm *
to_kbase_vm(struct pan_kmod_vm *vm)
{
    return (struct pan_kmod_kbase_vm *)vm;
}

/* ------------------------------------------------------------------ */
/* Public entry point                                                   */
/* ------------------------------------------------------------------ */

/* Create a pan_kmod_dev backed by mali_kbase (/dev/mali0) */
struct pan_kmod_dev *
pan_kmod_kbase_dev_create(int fd,
                           uint32_t flags,
                           drmVersionPtr version,
                           const struct pan_kmod_allocator *allocator);

/* Backend ops table — registered with pan_kmod core */
extern const struct pan_kmod_dev_ops pan_kmod_kbase_ops;

#endif /* _PAN_KMOD_MALI_KBASE_H_ */
