cmake_minimum_required(VERSION 2.8.9)
project (CC1200)

option (BUILD_TESTS "Build test programs" ON)
option (BUILD_LOCAL_ONLY "Build program only local, without CC1200 libs" ON)

set (LOCAL_LIB "/usr/local/lib")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SPI_LIB} ${PRUSS_LIBS} -lm")

add_compile_options("-g")

include_directories(src/controller)
include_directories(src/minstrel)
include_directories(src/cc1200)



file(GLOB SRC_CONTROLLER "src/controller/*.c")
file(GLOB SRC_MINSTREL "src/minstrel/*.c")

if (BUILD_LOCAL_ONLY) 
    set (SPI_LIB "")
    set (PRUSS_LIBS "")
    file(GLOB SRC_CC1200  "src/cc1200/cc1200_mockup.c")
else()
    set (PRUSS_LIBS "-Wl,-rpath=$(LOCAL_LIB) -L$(LOCAL_LIB) -lprussdrv")
    set (SPI_LIB "-L${LOCAL_LIB} -lspi")
    file(GLOB SRC_CC1200  "src/cc1200/cc1200_rate.c")
endif (BUILD_LOCAL_ONLY)


# gcc controller/*.c minstrel/*.c cc1200/cc1200_mockup.c

add_executable(MINSTREL ${SRC_CONTROLLER} ${SRC_MINSTREL} ${SRC_CC1200})

if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(testing)
endif (BUILD_TESTS)
