/* SPDX-License-Identifier: MIT */
/*
 * Mali G615 MC6 — Emulation tuning layer
 *
 * Runtime configuration for Eden (Switch/Yuzu-fork), RPCS3 (PS3),
 * Vita3K (PS Vita) on MediaTek Dimensity 8300 devices.
 *
 * Use via environment variables or the pan_g615_emu_config API.
 */

#ifndef _MALI_G615_EMU_CONFIG_H_
#define _MALI_G615_EMU_CONFIG_H_

#include <stdint.h>
#include <stdbool.h>

/* ------------------------------------------------------------------ */
/* Emulator type flags (bitmask)                                        */
/* ------------------------------------------------------------------ */

#define G615_EMU_NONE       0x00#define G615_EMU_EDEN       0x01    /* Nintendo Switch — Eden (Yuzu fork) */
#define G615_EMU_RYUJINX    0x02    /* Nintendo Switch — Ryujinx \(kept for compat\) */
#define G615_EMU_RPCS3      0x04    /* PlayStation 3 */
#define G615_EMU_VITA3K     0x08    /* PlayStation Vita */

/* Eden is our primary Switch backend */
#define G615_EMU_SWITCH     G615_EMU_EDEN

/* ------------------------------------------------------------------ */
/* Feature tuning flags                                                 */
/* ------------------------------------------------------------------ */

#define G615_OPT_FP16_PROMOTE       (1u << 0)   /* FP32→FP16 shader promo */
#define G615_OPT_IDVS               (1u << 1)   /* Index-Driven VS */
#define G615_OPT_LARGE_PAGES        (1u << 2)   /* 2MB MMU pages for guest RAM */
#define G615_OPT_AFBC_RENDER        (1u << 3)   /* AFBC on render targets */
#define G615_OPT_PIPELINE_CACHE     (1u << 4)   /* Aggressive pipeline caching */
#define G615_OPT_ASYNC_COMPILE      (1u << 5)   /* Background shader compilation */
#define G615_OPT_COHERENT_GUEST_MEM (1u << 6)   /* PBHA coherency for guest RAM 
#define G615_OPT_PERF_BOOST         (1u << 7)   /* Max GPU freq during gameplay */
#define G615_OPT_STRIP_PRECISE      (1u << 8)   /* Remove precise qualifiers */
#define G615_OPT_BATCH_DESCRIPTORS  (1u << 9)   /* Batch descriptor set updates */
#define G615_OPT_THREADED_SUBMIT    (1u << 10)  /* Multi-threaded cmd submission */

/* Default optimization sets per emulator */
#define G615_OPT_EDEN_DEFAULT     (G615_OPT_FP16_PROMOTE     | \
                                    G615_OPT_IDVS             | \
                                    G615_OPT_LARGE_PAGES      | \
                                    G615_OPT_AFBC_RENDER      | \
                                    G615_OPT_PIPELINE_CACHE   | \
                                    G615_OPT_ASYNC_COMPILE    | \
                                    G615_OPT_PERF_BOOST       | \
                                    G615_OPT_BATCH_DESCRIPTORS | \
                                    G615_OPT_THREADED_SUBMIT)

struct mali_g615_emu_config {
    unsigned emulator;
    unsigned opt_flags;
    int      perf_level;
    const char *shader_cache_path;
    int      async_compile_threads;
    uint64_t guest_gpu_base;
    uint64_t guest_gpu_size;
    uint64_t guest_cpu_base;
    uint64_t guest_cpu_size;
    bool     log_shader_compile;
    bool     log_gpu_faults;
    bool     capture_renderdoc;
    bool     validate_descriptors;
};

int mali_g615_emu_config_from_env(struct mali_g615_emu_config *cfg, unsigned emulator);
int mali_g615_emu_config_apply(int drm_fd, const struct mali_g615_emu_config *cfg);
unsigned mali_g615_detect_emulator(void);
int mali_g615_set_perf_level(int level);

#define G615_ENV_EMU            "MALI_G615_EMU"
#define G615_ENV_PERF           "MALI_G615_PERF_LEVEL"
#define G615_DEFAULT_CACHE_PATH  "/data/local/tmp/mali_g615_shader_cache"

#endif /* _MALI_G615_EMU_CONFIG_H_ */
