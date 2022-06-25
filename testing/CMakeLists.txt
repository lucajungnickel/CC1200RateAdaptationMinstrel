
add_executable(unit_test "testing.c" "../src/controller/packet_minstrel.h" "../src/controller/packet_minstrel.c")

add_test(NAME packet.minstrel.build COMMAND unit_test packet.minstrel.build)
add_test(NAME packet.minstrel.destroy COMMAND unit_test packet.minstrel.destroy)
add_test(NAME packet.minstrel.checksum COMMAND unit_test packet.minstrel.checksum)
add_test(NAME packet.minstrel.serialize.normal COMMAND unit_test packet.minstrel.serialize.normal)
add_test(NAME packet.minstrel.serialize.zero COMMAND unit_test packet.minstrel.serialize.zero)
add_test(NAME packet.minstrel.serialize.maximum COMMAND unit_test packet.minstrel.serialize.maximum)
add_test(NAME packet.minstrel.deserialize.normal COMMAND unit_test packet.minstrel.deserialize.normal)
add_test(NAME packet.minstrel.deserialize.maximum COMMAND unit_test packet.minstrel.deserialize.maximum)