/* SPDX-License-Identifier: MIT */
/*
 * mali_kbase_device.h — Device context for /dev/mali0 connection
 */

#ifndef _MALI_KBASE_DEVICE_H_
#define _MALI_KBASE_DEVICE_H_

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include "mali_kbase_iface.h"

#define MALI_MAX_QUEUE_GROUPS   8
#define MALI_MAX_CS_PER_GROUP   8
#define MALI_RING_BUFFER_SIZE   (1 << 16)   /* 64KB ring buffer */
#define MALI_TILER_CHUNK_SIZE   (512 * 1024) /* 512KB tiler heap chunks */
#define MALI_TILER_INIT_CHUNKS  5
#define MALI_TILER_MAX_CHUNKS   64

/* ------------------------------------------------------------------ */
/* Memory object                                                        */
/* ------------------------------------------------------------------ */

struct mali_bo {
    uint64_t    gpu_va;     /* GPU virtual address */
    void       *cpu_addr;  /* CPU mmap'd address (NULL if not mapped) */
    uint64_t    size;       /* size in bytes */
    uint64_t    flags;      /* BASE_MEM_* flags used at alloc */
    bool        imported;   /* true if from dmabuf/external */
};

/* ------------------------------------------------------------------ */
/* Command Stream queue                                                 */
/* ------------------------------------------------------------------ */

struct mali_cs_queue {
    struct mali_bo  ring_buf;       /* ring buffer BO */
    volatile void  *doorbell;       /* mmap'd doorbell page */
    uint32_t        doorbell_nr;
    uint32_t        insert;         /* CPU write pointer (bytes) */
    uint8_t         group_handle;
    uint8_t         csi_index;
    bool            active;

    pthread_mutex_t lock;
};

/* ------------------------------------------------------------------ */
/* Queue group                                                          */
/* ------------------------------------------------------------------ */

struct mali_queue_group {
    uint8_t             handle;
    struct mali_cs_queue queues[MALI_MAX_CS_PER_GROUP];
    uint32_t            num_queues;
    struct mali_bo      tiler_heap;
    uint64_t            tiler_heap_va;
    bool                active;
};

/* ------------------------------------------------------------------ */
/* GPU properties (read once at open)                                  */
/* ------------------------------------------------------------------ */

struct mali_gpu_props {
    uint32_t    product_id;
    uint32_t    major_rev;
    uint32_t    minor_rev;
    uint64_t    shader_present;
    uint64_t    tiler_present;
    uint64_t    l2_present;
    uint32_t    num_cores;
    uint32_t    num_l2_slices;
    uint32_t    thread_max_threads;
    uint32_t    thread_max_workgroup_size;
    uint64_t    gpu_freq_khz_max;
    uint32_t    mmu_features;
    uint32_t    as_present;
    uint32_t    coherency_mode;
};

/* ------------------------------------------------------------------ */
/* Device context                                                       */
/* ------------------------------------------------------------------ */

struct mali_device {
    int                     fd;             /* /dev/mali0 file descriptor */
    struct mali_gpu_props   props;

    /* Queue groups */
    struct mali_queue_group groups[MALI_MAX_QUEUE_GROUPS];
    uint32_t                num_groups;

    /* Firmware shared memory */
    struct mali_bo          fw_mem;
    volatile uint8_t       *fw_base;        /* mmap'd firmware interface */

    /* Synchronization objects (GPU-CPU timeline) */
    struct mali_bo          sync_mem;
    volatile uint64_t      *sync_counters;
    uint64_t                next_seqno;

    pthread_mutex_t         lock;
    bool                    initialized;
};

/* ------------------------------------------------------------------ */
/* API                                                                  */
/* ------------------------------------------------------------------ */

/* Open /dev/mali0 and negotiate with kernel driver */
int mali_device_open(struct mali_device *dev);
void mali_device_close(struct mali_device *dev);

/* Read GPU hardware properties */
int mali_device_query_props(struct mali_device *dev);

/* Memory management */
int mali_bo_alloc(struct mali_device *dev, struct mali_bo *bo,
                  uint64_t size, uint64_t flags);
void mali_bo_free(struct mali_device *dev, struct mali_bo *bo);
void *mali_bo_map(struct mali_device *dev, struct mali_bo *bo);
void mali_bo_unmap(struct mali_bo *bo);
void mali_bo_sync(struct mali_device *dev, struct mali_bo *bo,
                  bool to_gpu);

/* Import external fd (dmabuf from Android gralloc) */
int mali_bo_import_dmabuf(struct mali_device *dev, struct mali_bo *bo,
                           int dmabuf_fd, uint64_t flags);

/* CSF queue group management */
int mali_queue_group_create(struct mali_device *dev,
                             struct mali_queue_group *grp,
                             uint32_t num_queues,
                             uint8_t priority);
void mali_queue_group_destroy(struct mali_device *dev,
                               struct mali_queue_group *grp);

/* Command submission */
int mali_cs_queue_init(struct mali_device *dev, struct mali_cs_queue *q,
                        uint8_t group_handle, uint8_t csi_index,
                        uint8_t priority);
void mali_cs_queue_fini(struct mali_device *dev, struct mali_cs_queue *q);
void mali_cs_emit(struct mali_cs_queue *q, const uint32_t *words,
                  uint32_t count);
void mali_cs_kick(struct mali_cs_queue *q);
int  mali_cs_wait(struct mali_device *dev, struct mali_cs_queue *q,
                  uint64_t seqno, uint64_t timeout_ns);

/* Tiler heap setup (required before rendering) */
int mali_tiler_heap_init(struct mali_device *dev,
                          struct mali_queue_group *grp);
void mali_tiler_heap_term(struct mali_device *dev,
                           struct mali_queue_group *grp);

/* Emit standard CSF commands */
void mali_cs_emit_nop(struct mali_cs_queue *q);
void mali_cs_emit_flush(struct mali_cs_queue *q, uint32_t flags);
void mali_cs_emit_sync_set(struct mali_cs_queue *q,
                            uint64_t sync_addr, uint32_t value);
void mali_cs_emit_sync_wait(struct mali_cs_queue *q,
                             uint64_t sync_addr, uint32_t value);

#endif /* _MALI_KBASE_DEVICE_H_ */
