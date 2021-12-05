set(ENET_HEADER ../include/enet.h)

# simple client/server example
add_executable(enet_main ENetMain.cpp ${ENET_HEADER})

# peer tests
add_executable(peer_receive_test PeerReceiveTest.cpp ${ENET_HEADER})
target_link_libraries(peer_receive_test PRIVATE gtest_main)

add_executable(peer_queue_incoming_command_test PeerQueueIncomingCommandTest.cpp ${ENET_HEADER})
target_link_libraries(peer_queue_incoming_command_test PRIVATE gtest_main)

add_executable(peer_dispatch_incoming_reliable_commands_test PeerDispatchIncomingReliableCommandsTest.cpp ${ENET_HEADER})
target_link_libraries(peer_dispatch_incoming_reliable_commands_test PRIVATE gtest_main)

add_executable(peer_throttle_test PeerThrottleTest.cpp ${ENET_HEADER})
target_link_libraries(peer_throttle_test PRIVATE gtest_main)

add_executable(peer_ping_test PeerPingTest.cpp ${ENET_HEADER})
target_link_libraries(peer_ping_test PRIVATE gtest_main)

# protocol tests
add_executable(protocol_handle_send_unsequenced_test ProtocolHandleSendUnsequencedTest.cpp ${ENET_HEADER})
target_link_libraries(protocol_handle_send_unsequenced_test PRIVATE gtest_main)

add_executable(protocol_handle_incoming_commands_test ProtocolHandleIncomingCommandsTest.cpp ${ENET_HEADER})
target_link_libraries(protocol_handle_incoming_commands_test PRIVATE gtest_main)

add_executable(protocol_handle_send_reliable_test ProtocolHandleSendReliableTest.cpp ${ENET_HEADER})
target_link_libraries(protocol_handle_send_reliable_test PRIVATE gtest_main)

add_executable(protocol_handle_acknowledge_test ProtocolHandleAcknowledgeTest.cpp ${ENET_HEADER})
target_link_libraries(protocol_handle_acknowledge_test PRIVATE gtest_main)
