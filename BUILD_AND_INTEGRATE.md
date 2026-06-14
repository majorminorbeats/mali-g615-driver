# Mali G615 MC6 Driver — Build & Integration Guide
### MediaTek Dimensity 8300 | Valhall Gen2 CSF | Panthor and Panfrost

---

## Architecture Overview

```
Emulator (Ryujinx / RPCS3 / Vita3K)
         │
         ▼
  Vulkan / OpenGL API
         │
         ▼
  Mesa (Panfrost userspace)          ← pan_g615_props.c
  pan_g615_shader.c (NIR passes)     ← FP16 promote, IDVS, fixups
         │
         ▼
  DRM/KMS kernel interface
         │
         ▼
  Panthor kernel driver              ← panthor_g615.c / panthor_g615_mmu.c
         │
         ▼
  Mali G615 MC6 hardware
  (MediaTek Dimensity 8300)
```

---

## Step 1: Identify your GPU_ID

Update MACROS in kernel/panthor_g615.h if your product ID differs from 0xa004.

---

## Step 2: Build the Kernel Driver

Copy kernel/ files into drivers/gpu/drm/panthor/ and apply kernel/Makefile.patch.

---

## Step 3: Build Mesa

Use the GitHub Actions workflow in .github/workflows/build.yml for automated builds.

---

## Step 4: Load in Eden

See android/BUILD_FOR_EDEN-.md for deployment instructions.
