# How to get libvulkan_g615.so on Windows (no Linux needed)

GitHub builds it for you FREE on their servers. Takes about 15 minutes.

---

## Step 1 — Make a free GitHub account
https://github.com/signup

---

## Step 2 — Create a new repository
1. Click the **+** in the top right → **New repository**
2. Name it: `mali-g615-driver`
3. Set to **Private** (keeps your work private)
4. Click **Create repository**

---

## Step 3 — Upload the driver files
In your new repo, click **uploading an existing file** and upload
this entire `mali_g615_driver` folder. Make sure the structure is:

```
mali_g615_driver/
├── .github/
│   └── workflows/
│       └── build.yml          ← REQUIRED
├── android/
│   ├── mali_kbase_iface.h
│   ├── mali_kbase_device.h
│   ├── mali_kbase_device.c
│   ├── pan_kmod_mali_kbase.h
│   ├── pan_kmod_mali_kbase.c
│   ├── vk_icd_g615.c
│   └── meta.json
├── mesa_src/
│   ├── pan_g615_props.c
│   └── pan_g615_shader.c
└── patches/
    ├── 0001-panfrost-add-mali-kbase-backend.patch
    ├── 0002-panfrost-add-g615-gpu-support.patch
    └── 0003-panvk-android-packaging.patch
```

---

## Step 4 — Run the build
1. Click **Actions** tab in your repo
2. Click **Build Mali G615 Vulkan Driver for Android**
3. Click **Run workflow** → **Run workflow**
4. Wait ~15 minutes (you can watch it live)

---

## Step 5 — Download your .so
1. When the build finishes (green checkmark ✓)
2. Click into the completed run
3. Scroll down to **Artifacts**
4. Click **mali-g615-vulkan-driver-eden** to download a zip
5. Unzip it — you get:
   ```
   libvulkan_g615.so
   meta.json
   ```

---

## Step 6 — Load into Eden

### Transfer to your device
Connect your device via USB and run in Command Prompt:
```bat
adb push libvulkan_g615.so  /sdcard/gpu_drivers/mali_g615/
adb push meta.json           /sdcard/gpu_drivers/mali_g615/
```
Or copy via File Explorer if your device mounts as USB storage.

### Select in Eden
1. Open Eden
2. **Settings → Graphics → GPU Driver**
3. Tap **Install** → navigate to `/sdcard/gpu_drivers/mali_g615/`
4. Select it
5. Restart Eden

---

## Verify it's working

In Command Prompt:
```bat
adb logcat -s MaliG615-VK
```

You should see:
```
Mali G615 MC6 Vulkan driver loaded (panvk/kbase)
Mali G615 MC6 device opened: 6 cores, 4 L2 slices
```

If you see that — your GPU is running the open driver.
