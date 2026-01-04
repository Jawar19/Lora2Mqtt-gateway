set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_AR arm-none-eabi-ar)
set(CMAKE_RANLIB arm-none-eabi-ranlib)

set(CMAKE_C_FLAGS_INIT "-mcpu=cortex-m33 -mthumb")
set(CMAKE_CXX_FLAGS_INIT "-mcpu=cortex-m33 -mthumb")
