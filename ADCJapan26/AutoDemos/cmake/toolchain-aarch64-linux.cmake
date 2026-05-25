set(CMAKE_SYSTEM_NAME      Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(ARCH_TRIPLET aarch64-linux-gnu)
set(CMAKE_LIBRARY_ARCHITECTURE ${ARCH_TRIPLET})

# ── Compilers ────────────────────────────────────────────────────────────────
set(CMAKE_C_COMPILER   /usr/bin/aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER /usr/bin/aarch64-linux-gnu-g++)

# ── Linker: prioritise arm64 lib directory ───────────────────────────────────
set(CMAKE_EXE_LINKER_FLAGS_INIT
    "-L/usr/lib/${ARCH_TRIPLET} -Wl,-rpath-link,/usr/lib/${ARCH_TRIPLET}")
set(CMAKE_SHARED_LINKER_FLAGS_INIT
    "-L/usr/lib/${ARCH_TRIPLET} -Wl,-rpath-link,/usr/lib/${ARCH_TRIPLET}")

# ── pkg-config: use the cross wrapper, restrict to arm64 .pc files ───────────
set(PKG_CONFIG_EXECUTABLE /usr/bin/aarch64-linux-gnu-pkg-config
    CACHE FILEPATH "pkg-config executable")
set(ENV{PKG_CONFIG_DIR}        "")
set(ENV{PKG_CONFIG_LIBDIR}     "/usr/lib/${ARCH_TRIPLET}/pkgconfig:/usr/share/pkgconfig")
set(ENV{PKG_CONFIG_SYSROOT_DIR} "/")
set(ENV{PKG_CONFIG_PATH}       "")

# ── Explicit arm64 library paths (stop CMake picking up host x86_64 libs) ────
set(ALSA_LIBRARY          "/usr/lib/${ARCH_TRIPLET}/libasound.so"    CACHE FILEPATH "" FORCE)
set(ALSA_INCLUDE_DIR      "/usr/include"                              CACHE PATH     "" FORCE)
set(ALSA_LIBRARIES        "/usr/lib/${ARCH_TRIPLET}/libasound.so"    CACHE FILEPATH "" FORCE)
set(FONTCONFIG_LIBRARIES  "/usr/lib/${ARCH_TRIPLET}/libfontconfig.so" CACHE FILEPATH "" FORCE)
set(FREETYPE_LIBRARIES    "/usr/lib/${ARCH_TRIPLET}/libfreetype.so"   CACHE FILEPATH "" FORCE)
set(OPENGL_gl_LIBRARY     "/usr/lib/${ARCH_TRIPLET}/libGL.so"        CACHE FILEPATH "" FORCE)

# ── HailoRT cross prefix ─────────────────────────────────────────────────────
# Extract the arm64 HailoRT .deb once:
#   sudo mkdir -p /opt/hailo-cross/arm64
#   sudo dpkg-deb -x hailort_<ver>_arm64.deb /opt/hailo-cross/arm64
set(HAILO_CROSS_PREFIX "/opt/hailo-cross/arm64" CACHE PATH
    "Root of extracted arm64 HailoRT .deb (for find_package(HailoRT))")

# ── CMake find root path ─────────────────────────────────────────────────────
# HAILO_CROSS_PREFIX first so its HailoRTConfig.cmake takes precedence over
# any host-architecture HailoRT that may also be installed.
set(CMAKE_FIND_ROOT_PATH ${HAILO_CROSS_PREFIX} /usr/lib/${ARCH_TRIPLET} /usr)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
