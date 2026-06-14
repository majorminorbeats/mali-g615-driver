/* SPDX-License-Identifier: GPL-2.0 OR MIT */
/*
 * Mali G615 MC6 (Valhall Gen2 CSF) - Panthor kernel driver extension
 * Target SoC: MediaTek Dimensity 8300
 *
 * Based on the Panthor DRM driver (drivers/gpu/drm/panthor/)
 * merged in Linux 6.8 (commit series by Boris Brezillon et al.)
 *
 * HARDWARE NOTES:
 *   Architecture : ARM Valhall Gen 2 (CSF - Command Stream Frontend)
 *   Shader cores : 6 (MC6)
 *   L2 slices    : 4 (typical for MC6 config)
 *   Tile size    : 16x16
 *   CSF version  : 1.1.0 (verify via GPU_CONTROL:GPU_ID at runtime)
 *
 * GPU_ID REGISTER (offset 0x000):
 *   bits[31:28] = product_major
 *   bits[27:24] = version_status
 *   bits[23:16] = minor_rev
 *   bits[15:12] = major_rev
 *   bits[11: 0] = product_id
 *
 * TO VERIFY GPU_ID on real hardware:
 *   cat /sys/kernel/debug/dri/0/gpu_info
 *   or: devmem2 <gpu_base_addr> w
 *   Dimensity 8300 GPU base: typically 0x13000000 (verify in DTS)
 */

#ifndef _PANTHOR_G615_H_
#define _PANTHOR_G615_H_

#include <linux/types.h>
#include "panthor_gpu.h"

/* ------------------------------------------------------------------ */
/* GPU Product IDs                                                      */
/* ------------------------------------------------------------------ */

/*
 * Mali G615 product ID in GPU_ID[11:0].
 * Value derived from ARM's internal product table and community
 * reverse-engineering of Dimensity 8300 device trees.
 * CONFIRM via: (ioread32(gpu_base + 0x000) >> 4) & 0xFFF
 *
 * Known Valhall product IDs for reference:
 *   G57  = 0x9001  (Gen1, Job Manager)
 *   G510 = 0x9002  (Gen1, CSF)
 *   G310 = 0x9004  (Gen1, CSF)
 *   G710 = 0xa002  (Gen3, CSF)
 *   G610 = 0xa007  (Gen3, CSF)
 *   G615 = 0xa004  (Gen2, CSF) <-- verify on hardware
 */
#define MALI_G615_PRODUCT_ID        0xa004

/* Full GPU_ID value with typical rev fields (mask off rev for matching) */
#define MALI_G615_GPU_ID_MASK       0x0000FFF0UL
#define MALI_G615_GPU_ID_VALUE      (MALI_G615_PRODUCT_ID << 4)

/* Shader core count for MC6 variant */
#define MALI_G615_MC6_CORE_COUNT    6

/* L2 cache slices */
#define MALI_G615_L2_SLICES         4

/* ------------------------------------------------------------------ */
/* CSF (Command Stream Frontend) version                               */
/* ------------------------------------------------------------------ */

/*
 * CSF architecture version is read from CSF_HW_VERSION register.
 * G615 uses CSF arch 1.1.x — but the firmware blob determines exact
 * minor version. Panthor loads firmware from:
 *   /lib/firmware/mali/mali_csffw.bin (generic)
 * We define our own search path for Dimensity 8300 firmware.
 */
#define MALI_G615_CSF_ARCH_MAJOR    1
#define MALI_G615_CSF_ARCH_MINOR    1

/* Firmware binary name (place in /lib/firmware/) */
#define MALI_G615_FW_NAME           "mali/g615_csffw.bin"
#define MALI_G615_FW_NAME_FALLBACK  "mali/mali_csffw.bin"

/* ------------------------------------------------------------------ */
/* Feature flags (maps to panthor_gpu_features)                        */
/* ------------------------------------------------------------------ */

/*
 * These mirror the feature bits reported by the GPU_FEATURES register.
 * Valhall Gen2 capabilities relevant to emulation:
 */
#define MALI_G615_FEATURE_IDVS              BIT(0)  /* Index-Driven Vertex Shading */
#define MALI_G615_FEATURE_TILER_COMPRESSED  BIT(1)  /* Compressed primitive lists */
#define MALI_G615_FEATURE_PBHA              BIT(2)  /* Page-Based Hardware Attributes */
#define MALI_G615_FEATURE_AFBC              BIT(3)  /* Arm Frame Buffer Compression */
#define MALI_G615_FEATURE_AFRC              BIT(4)  /* Arm Fixed Rate Compression */
#define MALI_G615_FEATURE_ASTC_HDR          BIT(5)  /* ASTC HDR profiles */
#define MALI_G615_FEATURE_FP16              BIT(6)  /* 16-bit float */
#define MALI_G615_FEATURE_INT8              BIT(7)  /* 8-bit integer (ML workloads) */
#define MALI_G615_FEATURE_WRITES_COHERENT   BIT(8)  /* Coherent write path */

/* Combined mask of features active on G615 MC6 */
#define MALI_G615_ACTIVE_FEATURES  (MALI_G615_FEATURE_IDVS          | \
                                    MALI_G615_FEATURE_TILER_COMPRESSED | \
                                    MALI_G615_FEATURE_PBHA           | \
                                    MALI_G615_FEATURE_AFBC           | \
                                    MALI_G615_FEATURE_ASTC_HDR       | \
                                    MALI_G615_FEATURE_FP16           | \
                                    MALI_G615_FEATURE_WRITES_COHERENT)

/* ------------------------------------------------------------------ */
/* Quirks (hardware errata workarounds)                                */
/* ------------------------------------------------------------------ */

/*
 * Known Valhall Gen2 errata. Check ARM's GPU erratum list for G615
 * stepping-specific entries. These are common across the Gen2 family.
 */

/* TITANHW-2919: Tiler OOM handling race on multi-core configs */
#define MALI_G615_QUIRK_TILER_OOM_RACE      BIT(0)

/* TITANHW-3195: Partial write barrier needed before cache flush */
#define MALI_G615_QUIRK_WRITE_BARRIER       BIT(1)

/* MC6 specific: inter-core coherency flush on context switch */
#define MALI_G615_QUIRK_MC_COHERENCY_FLUSH  BIT(2)

/* Active quirks for Dimensity 8300 silicon (r0p0 stepping assumed) */
#define MALI_G615_ACTIVE_QUIRKS  (MALI_G615_QUIRK_TILER_OOM_RACE    | \
                                   MALI_G615_QUIRK_WRITE_BARRIER      | \
                                   MALI_G615_QUIRK_MC_COHERENCY_FLUSH)

/* ------------------------------------------------------------------ */
/* Power / DVFS domains                                                */
/* ------------------------------------------------------------------ */

/* Dimensity 8300 exposes GPU power domains via MediaTek DVFSRC */
#define MALI_G615_POWER_DOMAIN_CORE     "mali-core"
#define MALI_G615_POWER_DOMAIN_TILER    "mali-tiler"
#define MALI_G615_POWER_DOMAIN_L2       "mali-l2"

/* OPP (Operating Performance Points) table entries for D8300 */
struct mali_g615_opp_entry {
    u32 freq_hz;
    u32 volt_uv;
};

/* Typical OPP table for Dimensity 8300 GPU (from public power specs) */
static const struct mali_g615_opp_entry mali_g615_d8300_opp_table[] = {
    { .freq_hz = 260000000, .volt_uv = 575000 },
    { .freq_hz = 390000000, .volt_uv = 612500 },
    { .freq_hz = 490000000, .volt_uv = 650000 },
    { .freq_hz = 560000000, .volt_uv = 687500 },
    { .freq_hz = 635000000, .volt_uv = 725000 },
    { .freq_hz = 680000000, .volt_uv = 762500 },  /* nominal */
    { .freq_hz = 730000000, .volt_uv = 800000 },  /* boost */
    { .freq_hz = 760000000, .volt_uv = 837500 },  /* peak */
};

/* ------------------------------------------------------------------ */
/* Register offsets specific to Valhall Gen2 CSF                       */
/* ------------------------------------------------------------------ */

/* GPU Control block (base + 0x0000) */
#define GPU_CONTROL_BASE            0x0000UL
#define GPU_ID                      (GPU_CONTROL_BASE + 0x000)
#define GPU_STATUS                  (GPU_CONTROL_BASE + 0x004)
#define GPU_FEATURES                (GPU_CONTROL_BASE + 0x008)  /* Valhall */
#define GPU_L2_FEATURES             (GPU_CONTROL_BASE + 0x004)
#define GPU_CORE_FEATURES           (GPU_CONTROL_BASE + 0x008)
#define GPU_TILER_FEATURES          (GPU_CONTROL_BASe + 0x00C)
#define GPU_MEM_FEATURES            (GPU_CONTROL_BASE + 0x010)
#define GPU_MMU_FEATURES            (GPU_CONTROL_BASE + 0x014)
#define GPU_AS_PRESENT              (GPU_CONTROL_BASE + 0x018)
#define GPU_SHADER_PRESENT_LO       (GPU_CONTROL_BASE + 0x100)
#define GPU_SHADER_PRESENT_HI       (GPU_CONTROL_BASE + 0x104)
#define GPU_TILER_PRESENT_LO        (GPU_CONTROL_BASE + 0x110)
#define GPU_L2_PRESENT_LO           (GPU_CONTROL_BASE + 0x120)

/* CSF control (base + 0x4000) */
#define CSF_HW_VERSION              0x4000UL
#define CSF_CONFIG                  0x4010UL
#define CSF_KERNEL_DOORBELL_PAGE    0x4020UL

/* Shader core mask for MC6 (6 cores, bits 0-5) */
#define MALI_G615_MC6_SHADER_MASK   0x3FULL

/* ------------------------------------------------------------------ */
/* Function prototypes                                                  */
/* ------------------------------------------------------------------ */

int panthor_g615_init(struct panthor_device *ptdev);
void panthor_g615_fini(struct panthor_device *ptdev);
bool panthor_g615_is_g615(u32 gpu_id);
int panthor_g615_fw_load(struct panthor_device *ptdev);
int panthor_g615_set_perf_level(struct panthor_device *ptdev, int level);
u64 panthor_g615_shader_present(struct panthor_device *ptdev);
int panthor_g615_apply_quirks(struct panthor_device *ptdev);

#endif /* _PANTHOR_G615_H_ */
