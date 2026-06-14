# Build libvulkan_g615.so on Windows

## Step 1 — Install the tools (one time)

### Android NDK
1. Go to: https://developer.android.com/ndk/downloads
2. Download **"android-ndk-r26d-windows.zip"** (or latest r26)
3. Extract to `C:\android-ndk-r26d`

### CMake
1. Go to: https://cmake.org/download/
2. Download the Windows installer (cmake-3.x.x-windows-x86_64.msi)
3. Install it — check "Add CMake to system PATH"

### Ninja
1. Go to: https://github.com/ninja-build/ninja/releases
2. Download `ninja-win.zip`
3. Extract `ninja.exe` to `C:\ninja\`
4. Add `C:\ninja` to your system PATH:
   - Start → "Edit environment variables" → Path → New → `C:\ninja`

---

## Step 2 — Get the source files

Put all the files from the `android/` folder into one folder, e.g.:
```
C:\mali_g615\
  ├── mali_kbase_iface.h
  ├── mali_kbase_device.h
  ├── mali_kbase_device.c
  ├── vk_icd_g615.c
  ├── CMakeLists.txt
  ├── vulkan_g615.map
  └── meta.json
```

---

## Step 3 — Build

Open **Command Prompt** (not PowerShell) and run:

```bat
set NDK=C:\android-ndk-r26d

cd C:\mali_g615
mkdir build
cd build

cmake .. ^
  -DCMAKE_TOOLCHAIN_FILE=%NDK%\build\cmake\android.toolchain.cmake ^
  -DANDROID_ABI=arm64-v8a ^
  -DANDROID_PLATFORM=android-29 ^
  -DANDROID_NDK=%NDK% ^
  -DCMAKE_BUILD_TYPE=Release ^
  -G Ninja

cmake --build . --config Release
```

When it finishes you will have:
```
C:\mali_g615\build\libvulkan_g615.so
```

---

## Step 4 — Load into Eden

1. Copy both files to your phone (USB or wireless ADB):

```bat
adb push C:\mali_g615\build\libvulkan_g615.so /sdcard/gpu_drivers/mali_g615/
adb push C:\mali_g615\meta.json               /sdcard/gpu_drivers/mali_g615/
```

   Or just copy them manually via File Explorer / phone USB to:
   `/sdcard/gpu_drivers/mali_g615/`

2. Open **Eden**
3. **Settings → Graphics → GPU Driver → Install**
4. Navigate to the `mali_g615` folder
5. Select it and restart Eden

---

## Verify it worked

```bat
adb logcat -s MaliG615-VK
```

Look for:
```
Mali G615 MC6 Vulkan ICD loaded
Product ID : 0xa004
Cores      : 6
```

If you see that — the driver is talking to your GPU hardware.
