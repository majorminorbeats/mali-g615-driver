# Building libvulkan_g615.so for Eden on Android

## What this produces

`libvulkan_g615.so` — a Vulkan ICD that:
- Loads via Eden's custom GPU driver picker (same mechanism as Turnip)
- Talks directly to `/dev/mali0` (the existing mali_kbase kernel driver)
- Does NOT require a custom kernel or root
- Targets Mali G615 MC6 on MediaTek Dimensity 8300

---

## What you need on your build machine (Linux)

```bash
# Install Android NDK (r25c or newer)
# Download from: https://developer.android.com/ndk/downloads
export ANDROID_NDK=/path/to/android-ndk-r25c

# Install cmake and ninja
sudo apt install cmake ninja-build

# Vulkan headers (if not already in NDK)
sudo apt install vulkan-headers
# or: git clone https://github.com/KhronosGroup/Vulkan-Headers
```

---

## Build steps

```bash
cd mali_g615_driver/android

mkdir build && cd build

cmake .. \
  -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
  -DANDROID_ABI=arm64-v8a \
  -DANDROID_PLATFORM=android-29 \
  -DANDROID_NDK=$ANDROID_NDK \
  -DCMAKE_BUILD_TYPE=Release \
  -G Ninja

ninja

# Result:
ls -lh libvulkan_g615.so
```

---

## Packaging for Eden (driver folder structure)

Eden expects this exact layout in the folder you point it to:

```
gpu_driver/
├── meta.json               ← copy from android/meta.json
└── libvulkan_g615.so       ← your compiled output
```

### Deploy to device

```bash
# Create the driver folder on the device
adb shell mkdir -p /sdcard/gpu_drivers/mali_g615/

# Push the files
adb push build/libvulkan_g615.so  /sdcard/gpu_drivers/mali_g615/
adb push ../android/meta.json      /sdcard/gpu_drivers/mali_g615/
```

### Load in Eden

1. Open Eden
2. Go to **Settings → Graphics**
3. Tap **"GPU Driver"** (or "Custom Driver")
4. Tap **"Install"** and navigate to `/sdcard/gpu_drivers/mali_g615/`
5. Select the folder — Eden reads `meta.json` and loads `libvulkan_g615.so`
6. Restart Eden

---

## Verifying the driver loaded

Check Eden's log output (Settings → Logs or via adb):

```bash
adb logcat -s MaliG615 MaliG615-VK | head -30
```

Expected output:
```
MaliG615-VK: Mali G615 MC6 Vulkan ICD loaded
MaliG615-VK: Version: 1.0.0 | Target: MediaTek Dimensity 8300
MaliG615-VK: Mali G615 MC6 Vulkan ICD ready
MaliG615-VK:   Product ID : 0xa004
MaliG615-VK:   Cores      : 6
MaliG615-VK:   L2 slices  : 4
MaliG615-VK:   Max freq   : 760000 KHz
```

If you see `Product ID : 0xa004` — the driver opened the hardware successfully.

If `Product ID` is different, update `MALI_G615_PRODUCT_ID` in `panthor_g615.h`
to match the value reported here.

---

## Important notes

### `/dev/mali0` permissions
The driver needs read/write access to `/dev/mali0`.
On most Android devices this is already allowed for apps (the stock Mali
driver uses the same path). If you get permission denied:

```bash
# Check current permissions
adb shell ls -la /dev/mali0
# Should show: crw-rw-rw- or crw-rw---- with group 'gpu'

# If needed (rooted devices only):
adb shell su -c "chmod 666 /dev/mali0"
```

### Firmware (CSF firmware for G615)
The mali_kbase driver in Android already has the CSF firmware loaded —
you do not need to do anything for firmware. The existing Mali kernel
driver handles MCU boot before your app ever opens `/dev/mali0`.

### Full Vulkan implementation
`vk_icd_g615.c` currently implements the device enumeration and property
query layer. Full draw/compute command implementation requires integrating
Mesa's `panvk` (Panfrost Vulkan) as the shader compiler and command
encoder backend. The kernel communication layer (`mali_kbase_device.c`)
is complete and functional.

To integrate panvk:
1. Build Mesa with `-Dvulkan-drivers=panfrost` targeting Android
2. Link `libpanvk.a` into `libvulkan_g615.so`
3. Replace stub proc addresses in `vk_icd_g615.c` with panvk functions
4. Swap the kernel backend from Panthor DRM to mali_kbase (use
   `mali_kbase_device.c` as the implementation of panvk's `pan_kmod_*` API)
