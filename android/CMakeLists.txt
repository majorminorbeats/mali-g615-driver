cmake_minimum_required(VERSION 3.22)
project(mali_g615_vulkan_icd C)

# -----------------------------------------------------------------------
# Build: libvulkan_g615.so
# Target: Android arm64-v8a, API 29+ (Android 10+)
#
# Build command (from this directory):
#   mkdir build && cd build
#   cmake .. \
#     -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
#     -DANDROID_ABI=arm64-v8a \
#     -DANDROID_PLATFORM=android-29 \
#     -DANDROID_NDK=$ANDROID_NDK \
#     -DCMAKE_BUILD_TYPE=Release
#   make -j$(nproc)
#
# Output: build/libvulkan_g615.so
# -----------------------------------------------------------------------

set(CMAKE_C_STANDARD 11)

# Vulkan headers — use the ones bundled with the NDK or Vulkan SDK
# NDK path: $ANDROID_NDK/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/vulkan/
find_path(VULKAN_INCLUDE_DIR vulkan/vulkan.h
    HINTS
        $ENV{ANDROID_NDK}/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include
        $ENV{VULKAN_SDK}/include
    REQUIRED
)

# Source files
set(G615_SOURCES
    mali_kbase_device.c
    vk_icd_g615.c
    # Shader compiler stub (replace with full panvk compiler in production)
    pan_g615_shader_stub.c
)

add_library(vulkan_g615 SHARED ${G615_SOURCES})

target_include_directories(vulkan_g615 PRIVATE
    ${VULKAN_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../mesa      # pan_g615_props.h
)

# Android system libraries
target_link_libraries(vulkan_g615
    log          # __android_log_print
    android      # ANativeWindow etc
    m            # math
    dl           # dlopen (for panvk backend)
)

# Compiler flags optimized for Cortex-A720 (Dimensity 8300 big core)
target_compile_options(vulkan_g615 PRIVATE
    -O3
    -march=armv9-a+fp16+dotprod+i8mm
    -mtune=cortex-a720
    -fvisibility=hidden         # hide all symbols by default
    -ffast-math                 # safe for GPU driver math
    -DANDROID
    -DANDROID_API=29
    -DVK_USE_PLATFORM_ANDROID_KHR
    -DVK_NO_PROTOTYPES
    -DMALI_G615_MC6=1
)

# Only export the ICD entry points — everything else stays internal
set_target_properties(vulkan_g615 PROPERTIES
    OUTPUT_NAME "vulkan_g615"
    VERSION     "1.0.0"
    SOVERSION   "1"
)

# Export map — only these symbols are visible to the loader
target_link_options(vulkan_g615 PRIVATE
    -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/vulkan_g615.map
    -Wl,--strip-all
    -Wl,-z,defs
)
