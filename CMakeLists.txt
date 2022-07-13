cmake_minimum_required(VERSION 2.8.9)
project (CC1200)

#set( CMAKE_VERBOSE_MAKEFILE on )

option (BUILD_TESTS "Build test programs" ON)
#TODO SET TO OFF
option (BUILD_LOCAL_ONLY "Build program only local, without CC1200 libs" ON)
#TODO SET TO ON
option (BUILD_REAL_MINSTREL "Build program with real minstrel algorithm for productive system" OFF)

set (LOCAL_LIB "/usr/local/lib")
set(CMAKE_C_FLAGS "-pthread -DLOG_USE_COLOR")

add_compile_options("-g")

include_directories(src/controller)
include_directories(src/minstrel)
include_directories(src/cc1200)
include_directories(src/log.c/src)


file(GLOB SRC_CONTROLLER "src/controller/*.c")
#remove sender main
list(REMOVE_ITEM SRC_CONTROLLER "${CMAKE_CURRENT_SOURCE_DIR}/src/controller/main_sender.c")
#remove receiver main
list(REMOVE_ITEM SRC_CONTROLLER "${CMAKE_CURRENT_SOURCE_DIR}/src/controller/main_receiver.c")

file(GLOB SRC_CC1200 "src/cc1200/*.c")

file(GLOB SRC_LOG_LIB "src/log.c/src/log.c")

if (BUILD_LOCAL_ONLY)
    set (SPI_LIB "")
    set (PRUSS_LIBS "")
    list(REMOVE_ITEM SRC_CC1200 "${CMAKE_CURRENT_SOURCE_DIR}/src/cc1200/cc1200_rate.c")
else()
    set (PRUSS_LIBS "-Wl,-rpath=$(LOCAL_LIB) -L$(LOCAL_LIB) -lprussdrv")
    set (SPI_LIB "-L${LOCAL_LIB} -lspi")
    list(REMOVE_ITEM SRC_CC1200 "${CMAKE_CURRENT_SOURCE_DIR}/src/cc1200/cc1200_mockup.c") #REMOVE MOCKUOP
endif (BUILD_LOCAL_ONLY)


if (BUILD_REAL_MINSTREL)
    file(GLOB SRC_MINSTREL "src/minstrel/minstrel.c")
else ()
    file(GLOB SRC_MINSTREL "src/minstrel/minstrel_mockup.c")
endif (BUILD_REAL_MINSTREL)

set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SPI_LIB} ${PRUSS_LIBS} -lm -lncurses")
add_executable(main_sender ${SRC_LOG_LIB} ${SRC_CONTROLLER} ${SRC_MINSTREL} ${SRC_CC1200} "${CMAKE_CURRENT_SOURCE_DIR}/src/controller/main_sender.c")
add_executable(main_receiver ${SRC_LOG_LIB} ${SRC_CONTROLLER} ${SRC_MINSTREL} ${SRC_CC1200} "${CMAKE_CURRENT_SOURCE_DIR}/src/controller/main_receiver.c")

if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(testing)
endif (BUILD_TESTS)
