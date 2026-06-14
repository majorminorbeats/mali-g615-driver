// SPDX-License-Identifier: GPL-2.0 OR MIT
/*
 * Mali G615 MC6 (Valhall Gen2 CSF) - Panthor driver implementation
 */

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/firmware.h>
#include <linux/pm_opp.h>
#include <drm/drm_print.h>
#include "panthor_device.h"
#include "panthor_gpu.h"
#include "panthor_fw.h"
#include "panthor_g615.h"

struct panthor_g615_data {
    struct panthor_device  *ptdev;
    u32 gpu_id;
    u32 shader_present;
    u32 tiler_present;
    u32 l2_present;
    u32 active_quirks;
    u32 active_features;
    int perf_level;
    bool fw_loaded;
};

bool panthor_g615_is_g615(u32 gpu_id)
{
    u32 prod_id = (gpu_id & GPU_ID2_PRODUCT_MODEL) >> 4;
    return prod_id == MALI_G615_PRODUCT_ID;
}
EXPORT_SYMBOL_GPL(panthor_g615_is_g615);

u64 panthor_g615_shader_present(struct panthor_device *ptdev)
{
    u64 lo = gpu_read(ptdev, GPU_SHADER_PRESENT_LO);
    u64 hi = gpu_read(ptdev, GPU_SHADER_PRESENT_HI);
    return lo | (hi << 32);
}
EXPORT_SYMBOL_GPL(panthor_g615_shader_present);

int panthor_g615_apply_quirks(struct panthor_device *ptdev)
{
    struct panthor_g615_data *g615 = ptdev->model_data;
    u32 val;
    if (g615->active_quirks & MALI_G615_QUIRK_TILER_OOM_RACE) {
        val = gpu_read(ptdev, GPU_FEATURES);
        val &= ~BIT(17);
        gpu_write(ptdev, GPU_FEATURES, val);
    }
    return 0;
}
EXPORT_SYMBOL_GPL(panthor_g615_apply_quirks);

int panthor_g615_fw_load(struct panthor_device *ptdev)
{
    const struct firmware *fw = NULL;
    int ret;
    ret = request_firmware(&fw, MALI_G615_FW_NAME, ptdev->base.dev);
    if (ret)
        ret = request_firmware(&fw, MALI_G615_FW_NAME_FALLBACK, ptdev->base.dev);
    if (ret) { drm_err(ptdev->base.dev, "G615: no CSF fw (%d)\n", ret); return ret; }
    ret = panthor_fw_init_from_data(ptdev, fw->data, fw->size);
    release_firmware(fw);
    return ret;
}
EXPORT_SYMBOL_GPL(panthor_g615_fw_load);

int panthor_g615_set_perf_level(struct panthor_device *ptdev, int level)
{
    if (level < 0 || level >= 8) return -EINVAL;
    return dev_pm_opp_set_rate(ptdev->base.dev, mali_g615_d8300_opp_table[level].freq_hz);
}
EXPORT_SYMBOL_GPL(panthor_g615_set_perf_level);

int panthor_g615_init(struct panthor_device *ptdev)
{
    struct panthor_g615_data *g615;
    u32 gpu_id;
    int ret;
    gpu_id = gpu_read(ptdev, GPU_ID);
    if (!panthor_g615_is_g615(gpu_id)) return -ENODEV;
    g615 = devm_kzalloc(ptdev->base.dev, sizeof(*g615), GFP_KERNEL);
    if (!g615) return -ENOMEM;
    g615->ptdev = ptdev;
    g615->gpu_id = gpu_id;
    g615->active_quirks = MALI_G615_ACTIVE_QUIRKS;
    g615->active_features = MALI_G615_ACTIVE_FEATURES;
    g615->perf_level = 5;
    ptdev->model_data = g615;
    g615->shader_present = (u32)panthor_g615_shader_present(ptdev);
    g615->tiler_present = gpu_read(ptdev, GPU_TILER_PRESENT_LO);
    g615->l2_present = gpu_read(ptdev, GPU_L2_PRESENT_LO);
    ret = panthor_g615_apply_quirks(ptdev);
    if (ret) goto err;
    ret = panthor_g615_fw_load(ptdev);
    if (ret) goto err;
    drm_info(ptdev->base.dev, "Mali G615 MC6 initialized: shaders=0x%x\n", g615->shader_present);
    return 0;
err:
    ptdev->model_data = NULL;
    return ret;
}
EXPORT_SYMBOL_GPL(panthor_g615_init);

void panthor_g615_fini(struct panthor_device *ptdev)
{
    if (ptdev->model_data) panthor_g615_set_perf_level(ptdev, 0);
    ptdev->model_data = NULL;
}
EXPORT_SYMBOL_GPL(panthor_g615_fini);

const struct panthor_model_ops g615_model_ops = {
    .init = panthor_g615_init,
    .fini = panthor_g615_fini,
    .apply_quirks = panthor_g615_apply_quirks,
    .fw_load = panthor_g615_fw_load,
};
EXPORT_SYMBOL_GPL(g615_model_ops);

MODULE_LICENSE("GPL v2");

