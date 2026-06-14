/* SPDX-License-Identifier: MIT */
/*
 * mali_kbase_iface.h
 * Userspace interface to ARM's mali_kbase kernel driver (/dev/mali0).
 * Target: Mali G615 MC6 on MediaTek Dimensity 8300
 */

#ifndef _MALI_KBASE_IFACE_H_
#define _MALI_KBASE_IFACE_H_

#include <stdint.h>
#include <stdbool.h>
#include <sys/ioctl.h>

#define MALI_DEVICE_PATH        "/dev/mali0"
#define KBASE_IOCTL_TYPE        0x80

#define KBASE_IOCTL_VERSION_CHECK \
    _IOWR(KBASE_IOCTL_TYPE, 0, struct kbase_ioctl_version_check)
#define KBASE_IOCTL_SET_FLAGS \
    _IOW(KBASE_IOCTL_TYPE, 1, struct kbase_ioctl_set_flags)
#define KBASE_IOCTL_GET_GPUPROPS \
    _IOW(KBASE_IOCTL_TYPE, 3, struct kbase_ioctl_get_gpuprops)
#define KBASE_IOCTL_MEM_ALLOC \
    _IOWR(KBASE_IOCTL_TYPE, 3, struct kbase_ioctl_mem_alloc)
#define KBASE_IOCTL_MEM_FREE \
    _IOW(KBASE_IOCTL_TYPE, 7, struct kbase_ioctl_mem_free)
#define KBASE_IOCTL_MEM_QUERY XL_IOWR(KBASE_IOCTL_TYPE, 8, struct kbase_ioctl_mem_query)
#define KBASE_IOCTL_SYNC \
    _IOW(KBASE_IOCTL_TYPE, 9, struct kbase_ioctl_sync)
#define KBASE_IOCTL_MEM_IMPORT \
    _IOWR(KBASE_IOCTL_TYPE, 11, struct kbase_ioctl_mem_import)
#define KBASE_IOCTL_CS_QUEUE_ALLOC \
    _IOWR(KBASE_IOCTL_TYPE, 42, struct kbase_ioctl_cs_queue_alloc)
#define KBASE_IOCTL_CS_QUEUE_FREE \
    _IOW(KBASE_IOCTL_TYPE, 43, struct kbase_ioctl_cs_queue_free)
#define KBASE_IOCTL_CS_QUEUE_KICK RR_IOW(KBASE_IOCTL_TYPE, 44, struct kbase_ioctl_cs_queue_kick)
#define KBASE_IOCTL_CS_QUEUE_BIND \
    _IOWR(KBASE_IOCTL_TYPE, 45, struct kbase_ioctl_cs_queue_bind)
#define KBASE_IOCTL_CS_QUEUE_GROUP_CREATE _
    _IOWR(KBASE_IOCTL_TYPE, 58, struct kbase_ioctl_cs_queue_group_create)
#define KBASE_IOCTL_CS_QUEUE_GROUP_TERMINATE \
    _IOW(KBASE_IOCTL_TYPE, 47, struct kbase_ioctl_cs_queue_group_term)
#define KBASE_IOCTL_CS_TILER_HEAP_INIT \
    _IOWR(KBASE_IOCTL_TYPE, 52, struct kbase_ioctl_cs_tiler_heap_init)
#define KBASE_IOCTL_CS_TILER_HEAP_TERM V
  _IOW(KBASE_IOCTL_TYPE, 53, struct kbase_ioctl_cs_tiler_heap_term)

#define BASE_MEM_PROT_CPU_RD    (1U << 0)
#define BASE_MEM_PROT_CPU_WR    (1U << 1)
#define BASE_MEM_PROT_GPU_RD    (1U << 2)
#define BASE_MEM_PROT_GPU_WR    (1U << 3)
#define BASE_MEM_PROT_GPU_EX    (1U << 4)
#define BASE_MEM_GROW_ON_GPF    (1U << 9)
#define BASE_MEM_COHERENT_SYSTEM (1U << 11)
#define BASE_MEM_CACHED_CPU     (1U << 12)
#define BASE_MEM_SAME_VA        (1U << 17)
#define BASE_MEM_CSF_EVENT      (1U << 19)

#define MALI_MEM_GPU_RW         (BASE_MEM_PROT_GPU_RD | BASE_MEM_PROT_GPU_WR)
#define MALI_MEM_CPU_RW         (BASE_MEM_PROT_CPU_RD | BASE_MEM_PROT_CPU_WR)
#define MALI_MEM_BUFFER         (MALI_MEM_GPU_RW | MALI_MEM_CPU_RW | BASE_MEM_CACHED_CPU)
#define MALI_MEM_COHERENT       (MALI_MEM_BUFFER | BASE_MEM_COHERENT_SYSTEM)
#define MALI_MEM_CS_RING        (MALI_MEM_GPU_RW | MALI_MEM_CPU_RW | BASE_MEM_CSF_EVENT)

struct kbase_ioctl_version_check { uint16_t major; uint16_t minor; } __attribute__((packed));
struct kbase_ioctl_set_flags { uint32_t create_flags; } __attribute__((packed));
struct kbase_ioctl_get_gpuprops { uint64_t buffer; uint32_t size; uint32_t flags; } __attribute__((packed));
struct kbase_ioctl_mem_alloc {
    uint64_t va_pages; uint64_t commit_pages; uint64_t extent; uint64_t flags;
    uint64_t gpu_va; uint16_t va_alignment; uint8_t _pad[6];
} __attribute__((packed));
struct kbase_ioctl_mem_free { uint64_t gpu_addr; } __attribute__((packed));
struct kbase_ioctl_mem_query { uint64_t gpu_addr; uint64_t query; uint64_t value; } __attribute__((packed));
struct kbase_ioctl_sync { uint64_t handle; uint64_t user_addr; uint64_t size; uint8_t type; uint8_t _pad[7]; } __attribute__((packed));
struct kbase_ioctl_mem_import { uint64_t flags; uint64_t phandle; uint32_t type; uint32_t _pad; uint64_t gpu_va; uint64_t va_pages; } __attribute__((packed));
struct kbase_ioctl_cs_queue_alloc { uint32_t buffer_size; uint8_t priority; uint8_t _pad[3]; uint64_t buffer_gpu_addr; uint32_t doorbell_nr; uint32_t _pad2; } __attribute__((packed));
struct kbase_ioctl_cs_queue_free { uint64_t buffer_gpu_addr; } __attribute__((packed));
struct kbase_ioctl_cs_queue_kick { uint64_t buffer_gpu_addr; } __attribute__((packed));
struct kbase_ioctl_cs_queue_bind { uint64_t buffer_gpu_addr; uint8_t group_handle; uint8_t csi_index; uint8_t _pad[6]; uint64_t mmap_handle; } __attribute__((packed));
struct kbase_ioctl_cs_queue_group_create { uint64_t tiler_mask; uint64_t fragment_mask; uint64_t compute_mask; uint8_t cs_min; uint8_t priority; uint16_t _pad; uint32_t tiler_max; uint32_t fragment_max; uint32_t compute_max; uint8_t csi_handlers; uint8_t _pad2[3]; uint64_t reserved; uint8_t group_handle; uint8_t _pad3[7]; } __attribute__((packed));
struct kbase_ioctl_cs_queue_group_term { uint8_t group_handle; uint8_t _pad[7]; } __attribute__((packed));
struct kbase_ioctl_cs_tiler_heap_init { uint32_t chunk_size; uint32_t initial_chunks; uint32_t max_chunks; uint16_t target_in_flight; uint8_t _pad[2]; uint64_t gpu_heap_va; uint64_t first_hdr_va; } __attribute__((packed));
struct kbase_ioctl_cs_tiler_heap_term { uint64_t gpu_heap_va; } __attribute__((packed));

#define CSF_GLB_REQ_OFFSET          0x00
#define CSF_GLB_ACK_OFFSET          0x04
#define CSF_CSI_INSERT_OFFSET       0x20
#define CSF_CSI_EXTRACT_OFFSET      0x28
#define CSF_OP_FLUSH_CACHE      0x05
#define CSF_OP_SYNC_ADD         0x08
#define CSF_OP_SYNC_WAIT        0x0A
#define CSF_OP_DRAW_INDIRECT    0x20
#define CSF_OP_COMPUTE          0x21
#define CSF_FLUSH_FULL          (1U | 2U)

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

static inline uint64_t mali_page_count(uint64_t size) { return (size + PAGE_SIZE - 1) / PAGE_SIZE; }

#endif /* _MALI_KBASE_IFACE_H_ */
