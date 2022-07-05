#add_compile_options("-lpthread -pthread --lpthread")
#set (PTHREAD_LIB "-L${LOCAL_LIB} -lpthread")

file (GLOB test_src "*.c")
file (GLOB controller_src "../src/controller/*.c")
file (GLOB log_src "../src/log.c/src/log.c")

#remove sender main
list(REMOVE_ITEM controller_src "${CMAKE_CURRENT_SOURCE_DIR}/../src/controller/main_sender.c")
#remove receiver main
list(REMOVE_ITEM controller_src "${CMAKE_CURRENT_SOURCE_DIR}/../src/controller/main_receiver.c")


#message("${controller_src}")

set (CMAKE_EXE_LINKER_FLAGS "-lpthread")
set(CMAKE_C_FLAGS "-pthread -DLOG_USE_COLOR")
add_executable(unit_test ${test_src} ${controller_src} ${log_src} "../src/minstrel/minstrel_mockup.c" "../src/cc1200/cc1200_mockup.c")

add_test(NAME packet.minstrel.build COMMAND unit_test packet.minstrel.build)
add_test(NAME packet.minstrel.destroy COMMAND unit_test packet.minstrel.destroy)
add_test(NAME packet.minstrel.checksum COMMAND unit_test packet.minstrel.checksum)
add_test(NAME packet.minstrel.serialize.normal COMMAND unit_test packet.minstrel.serialize.normal)
add_test(NAME packet.minstrel.serialize.zero COMMAND unit_test packet.minstrel.serialize.zero)
add_test(NAME packet.minstrel.serialize.maximum COMMAND unit_test packet.minstrel.serialize.maximum)
add_test(NAME packet.minstrel.deserialize.normal COMMAND unit_test packet.minstrel.deserialize.normal)
add_test(NAME packet.minstrel.deserialize.maximum COMMAND unit_test packet.minstrel.deserialize.maximum)
add_test(NAME communication.init COMMAND unit_test communication.init)
add_test(NAME communication.sendOK.rcvOK.small COMMAND unit_test communication.sendOK.rcvOK.small)
add_test(NAME communication.handshake.sendFAIL COMMAND unit_test communication.handshake.sendFAIL)
add_test(NAME communication.handshake.ackFAIL COMMAND unit_test communication.handshake.ackFAIL)
add_test(NAME communication.handshake.checksum.sendFAIL COMMAND unit_test communication.handshake.checksum.sendFAIL)
add_test(NAME communication.handshake.checksum.ackFAIL COMMAND unit_test communication.handshake.checksum.ackFAIL)
add_test(NAME communication.send.bigData COMMAND unit_test communication.send.bigData)

