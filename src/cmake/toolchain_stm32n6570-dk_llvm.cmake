cmake_minimum_required(VERSION 3.13)

set(TOOLCHAIN_TARGET_NAME stm32n6570-dk)
set(CMAKE_SYSTEM_NAME               Generic)
set(CMAKE_SYSTEM_PROCESSOR          arm)

# Try to find an STM32CubeIDE installation to use for the toolchain.
file(GLOB TOOLCHAIN_DIRECTORIES
	"/home/doc/Programmes*/ATfE-*-Linux-x86_64/bin"
)

message(STATUS "ESarc ${TOOLCHAIN_DIRECTORIES}")

find_program(COMPILER_PATH
	NAMES clang
	HINTS
		${TOOLCHAIN_DIRECTORIES}
	REQUIRED
	NO_SYSTEM_ENVIRONMENT_PATH
	NO_CMAKE_ENVIRONMENT_PATH
)
get_filename_component(TOOLCHAIN_DIRECTORY ${COMPILER_PATH} DIRECTORY)

if(WIN32)
	set(TOOLCHAIN_SUFFIX ".exe")
endif()

set(CMAKE_C_COMPILER                ${TOOLCHAIN_DIRECTORY}/clang${TOOLCHAIN_SUFFIX})
set(CMAKE_ASM_COMPILER              ${CMAKE_C_COMPILER})
set(CMAKE_CXX_COMPILER              ${TOOLCHAIN_DIRECTORY}/clang++${TOOLCHAIN_SUFFIX})
set(CMAKE_OBJCOPY                   ${TOOLCHAIN_DIRECTORY}/llvm-objcopy${TOOLCHAIN_SUFFIX})
set(CMAKE_OBJDUMP                   ${TOOLCHAIN_DIRECTORY}/llvm-objdump${TOOLCHAIN_SUFFIX})
set(CMAKE_SIZE                      ${TOOLCHAIN_DIRECTORY}/llvm-size${TOOLCHAIN_SUFFIX})
set(CMAKE_AR                        ${TOOLCHAIN_DIRECTORY}/llvm-ar${TOOLCHAIN_SUFFIX})
set(CMAKE_RANLIB                    ${TOOLCHAIN_DIRECTORY}/llvm-ranlib${TOOLCHAIN_SUFFIX})
set(CMAKE_NM                        ${TOOLCHAIN_DIRECTORY}/llvm-nm${TOOLCHAIN_SUFFIX})

set(CMAKE_EXECUTABLE_SUFFIX_C       ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_CXX     ".elf")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-static -Wl,--gc-sections")
# Add D__ARM_ARCH_8M_MAIN__ define to workaround https://github.com/ARM-software/CMSIS_5/issues/1210
set(CMAKE_ASM_FLAGS_INIT        "-D__ARM_ARCH_8M_MAIN__=1 --target=thumbv8.1m.main-unknown-none-eabihf -mcpu=cortex-m55 -mfloat-abi=hard -march=armv8.1-m.main+mve.fp+fp.dp -x assembler-with-cpp")
set(CMAKE_C_FLAGS_INIT          "-D__ARM_ARCH_8M_MAIN__=1 --target=thumbv8.1m.main-unknown-none-eabihf -mcpu=cortex-m55 -mfloat-abi=hard -mthumb -g3 -fdata-sections -ffunction-sections -fstack-usage -fstack-protector -mcmse")
set(CMAKE_CXX_FLAGS_INIT        "-D__ARM_ARCH_8M_MAIN__=1 --target=thumbv8.1m.main-unknown-none-eabihf -mcpu=cortex-m55 -mfloat-abi=hard -mthumb -g3 -fdata-sections -ffunction-sections -fstack-usage -fstack-protector -fno-rtti -fno-exceptions -mcmse")
set(CMAKE_C_FLAGS_DEBUG    "" CACHE STRING "Flags used by the C compiler during RELWITHDEBINFO builds.")
set(CMAKE_CXX_FLAGS_DEBUG  "" CACHE STRING "Flags used by the CXX compiler during DEBUG builds.")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -DNDEBUG" CACHE STRING "Flags used by the C compiler during RELWITHDEBINFO builds.")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -DNDEBUG" CACHE STRING "Flags used by the CXX compiler during RELWITHDEBINFO builds.")
set(CMAKE_C_FLAGS_RELEASE "-O3" CACHE STRING "Flags used by the C compiler during RELEASE builds.")
set(CMAKE_CXX_FLAGS_RELEASE "-O3" CACHE STRING "Flags used by the CXX compiler during RELEASE builds.")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Find signing tool for boot flash models
file(GLOB SIGNINGTOOL_DIRECTORIES
	"C:/ST/STM32CubeIDE_*/STM32CubeIDE/plugins/com.st.stm32cube.ide.mcu.externaltools.cubeprogrammer.*/tools/bin/"
	"/opt/st/stm32cubeide*/plugins/com.st.stm32cube.ide.mcu.externaltools.cubeprogrammer.*/tools/bin/"
	"/Applications/STM32CubeIDE.app/Contents/Eclipse/plugins/com.st.stm32cube.ide.mcu.externaltools.cubeprogrammer.*/tools/bin/"
)
find_program(STM32_SIGNINGTOOL_CLI
	NAMES STM32_SigningTool_CLI
	PATHS
		${SIGNINGTOOL_DIRECTORIES}
)