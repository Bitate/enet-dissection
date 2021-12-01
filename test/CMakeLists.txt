set(ENET_HEADER ../include/enet.h)

# simple client/server example
add_executable(enet_main ENetMain.cpp ${ENET_HEADER})

# peer tests
add_executable(peer_receive_test PeerReceiveTest.cpp ${ENET_HEADER})
target_link_libraries(peer_receive_test PRIVATE gtest_main)

add_executable(peer_queue_incoming_command_test PeerQueueIncomingCommandTest.cpp ${ENET_HEADER})
target_link_libraries(peer_queue_incoming_command_test PRIVATE gtest_main)

# protocol tests
add_executable(protocol_handle_send_unsequenced_test ProtocolHandleSendUnsequencedTest.cpp ${ENET_HEADER})
target_link_libraries(protocol_handle_send_unsequenced_test PRIVATE gtest_main)

add_executable(protocol_handle_incoming_command_test ProtocolHandleIncomingCommandTest.cpp ${ENET_HEADER})
target_link_libraries(protocol_handle_incoming_command_test PRIVATE gtest_main)

add_executable(protocol_handle_send_reliable_test ProtocolHandleSendReliableTest.cpp ${ENET_HEADER})
target_link_libraries(protocol_handle_send_reliable_test PRIVATE gtest_main)