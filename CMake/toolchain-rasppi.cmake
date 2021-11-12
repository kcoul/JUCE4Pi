# -DCMAKE_TOOLCHAIN_FILE=${PROJECT_DIR}/toolchain-rasppi.cmake

# Define the host system
SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_SYSTEM_VERSION 1)

#Define the cross compiler locations
SET(CMAKE_C_COMPILER /usr/bin/arm-linux-gnueabihf-gcc)
SET(CMAKE_CXX_COMPILER /usr/bin/arm-linux-gnueabihf-gcc)

#Define the target environment
SET(CMAKE_FIND_ROOT_PATH /usr/lib/gcc-cross/arm-linux-gnueabihf)

SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)