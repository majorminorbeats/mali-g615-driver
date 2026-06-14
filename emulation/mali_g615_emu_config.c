/*
 * Mali G615 MC6 — Emulation config runtime implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "mali_g615_emu_config.h"

unsigned mali_g615_detect_emulator(void)
{
    char cmdline[4096] = {0};
    int fd, ret;
    fd = open("/proc/self/cmdline", O_RDONLY);
    if (fd < 0) return G615_EMU_NONE;
    ret = read(fd, cmdline, sizeof(cmdline) - 1);
    close(fd);
    if (ret <= 0) return G615_EMU_NONE;
    if (strstr(cmdline, "eden") || strstr(cmdline, "org.eden")) return G615_EMU_EDEN;
    if (strstr(cmdline, "ryujinx")) return G615_EMU_RYUJINX;
    if (strstr(cmdline, "rpcs3")) return G615_EMU_RPCS3;
    if (strstr(cmdline, "vita3k")) return G615_EMU_VITA3K;
    return G615_EMU_NONE;
}

static unsigned default_opts_for(unsigned emu)
{
    switch (emu) {
    case G615_EMU_EDEN:    return G615_OPT_EDEN_DEFAULT;
    default:               return 0;
    }
}

int mali_g615_emu_config_from_env(struct mali_g615_emu_config *cfg,
                                   unsigned emulator)
{
    memset(cfg, 0, sizeof(*cfg));
    cfg->emulator = emulator ? emulator : mali_g615_detect_emulator();
    cfg->opt_flags = default_opts_for(cfg->emulator);
    cfg->perf_level = 6;
    cfg->shader_cache_path = G615_DEFAULT_CACHE_PATH;
    switch (cfg->emulator) {
    case G615_EMU_EDEN:
        cfg->guest_gpu_base = EDEN_GUEST_GPU_BASE;
        cfg->guest_gpu_size = EDEN_GUEST_GPU_SIZE;
        cfg->guest_cpu_base = EDEN_GUEST_CPU_BASE;
        cfg->guest_cpu_size = EDEN_GUEST_CPU_SIZE;
        break;
    default: break;
    }
    return 0;
}

int mali_g615_emu_config_apply(int drm_fd, const struct mali_g615_emu_config *cfg)
{
    setenv("MESA_SHADER_CACHE_DIR", cfg->shader_cache_path, 1);
    return 0;
}

int mali_g615_set_perf_level(int level) { return 0; }
