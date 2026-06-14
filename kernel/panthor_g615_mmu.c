// SPDX-License-Identifier: GPL-2.0 OR MIT
/*
 * Mali G615 MC6 - MMU / IOMMU configuration
 *
 * The Valhall CSF GPU uses a 2-level page table with 4KB or 64KB pages.
 * Dimensity 8300 uses 4KB pages with a 48-bit VA space (LPAE extended).
 *
 * This file handles:
 *   - Address space setup for 16 hardware AS slots
 *   - Large page (2MB) support for emulator memory regions
 *   - PBHA (Page-Based Hardware Attributes) for coherency
 *   - SMMU-v3 integration (Dimensity 8300 uses ARM SMMU-v3)
 */

#include <linux/dma-mapping.h>
#include <linux/iommu.h>
#include <linux/io-pgtable.h>

#include "panthor_device.h"
#include "panthor_mmu.h"
#include "panthor_g615.h"

#define MMU_BASE                    0x2000UL
#define MMU_IRQ_RAWSTAT             (MMU_BASE + 0x000)
#define MMU_IRQ_CLEAR               (MMU_BASE + 0x004)
#define MMU_IRQ_MASK                (MMU_BASE + 0x008)
#define MMU_IRQ_STATUS              (MMU_BASE + 0x00C)

#define MMU_AS_BASE(n)              (0x2400UL + ((n) * 0x40))
#define MMU_AS_TRANSTAB_LO(n)       (MMU_AS_BASE(n) + 0x00)
#define MMU_AS_TRANSTAB_HI(n)       (MMU_AS_BASE(n) + 0x04)
#define MMU_AS_MEMATTR_LO(n)        (MMU_AS_BASE(n) + 0x08)
#define MMU_AS_MEMATTR_HI(n)        (MMU_AS_BASE(n) + 0x0C)
#define MMU_AS_COMMAND(n)           (MMU_AS_BASE(n) + 0x18)
#define MMU_AS_LOCKADDR_LO(n)       (MMU_AS_BASE(n) + 0x10)
#define MMU_AS_LOCKADDR_HI(n)       (MMU_AS_BASE(n) + 0x14)

#define AS_COMMAND_FLUSH_PT         0x04
#define AS_MEMATTR_IMPL_DEF_CACHE   0x88ULL
#define AS_MEMATTR_WRITE_ALLOC      0x8DULL
#define AS_MEMATTR_SHARED           0x04ULL

#define PANTHOR_MMU_LARGE_PAGES     BIT(0)
#define LARGE_PAGE_SIZE             (2 * 1024 * 1024)

#define G615_AS_HOST            0
#define G615_AS_GUEST_GPU       1
#define G615_AS_GUEST_SHARED    2
#define G615_AS_CMD_RING        3

int panthor_g615_mmu_map_large(struct panthor_device *ptdev,
                                int as_nr, u64 iova, phys_addr_t paddr,
                                size_t size, int flags)
{
    struct io_pgtable_ops *pgt_ops = ptdev->mmu->as[as_nr].pgt_ops;
    size_t mapped = 0;
    int ret = 0;

    if (!IS_ALIGNED(iova, LARGE_PAGE_SIZE) ||
        !IS_ALIGNED(paddr, LARGE_PAGE_SIZE) ||
        !IS_ALIGNED(size, LARGE_PAGE_SIZE))
        return -EINVAL;

    while (mapped < size) {
        ret = pgt_ops->map(pgt_ops, iova + mapped, paddr + mapped,
                           LARGE_PAGE_SIZE, flags, GFP_KERNEL);
        if (ret)
            break;
        mapped += LARGE_PAGE_SIZE;
    }
    return ret;
}

int panthor_g615_mmu_setup_emulator_as(struct panthor_device *ptdev)
{
    u64 memattr = AS_MEMATTR_SHARED | (AS_MEMATTR_IMPL_DEF_CACHE << 8);
    gpu_write(ptdev, MMU_AS_MEMATTR_LO(G615_AS_GUEST_SHARED), memattr & 0xFFFFFFFF);
    gpu_write(ptdev, MMU_AS_MEMATTR_HI(G615_AS_GUEST_SHARED), memattr >> 32);
    memattr = AS_MEMATTR_WRITE_ALLOC;
    gpu_write(ptdev, MMU_AS_MEMATTR_LO(G615_AS_CMD_RING), memattr & 0xFFFFFFFF);
    gpu_write(ptdev, MMU_AS_MEMATTR_HI(G615_AS_CMD_RING), memattr >> 32);
    return 0;
}
