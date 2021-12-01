#include <gtest/gtest.h>

#define ENET_IMPLEMENTATION
#include "../include/enet.h"

TEST(ENetTests, enet_peer_receive_test_zero_packet)
{
	ENetPeer* peer = (ENetPeer*)malloc(sizeof(ENetPeer));
	enet_uint8 channel_id;

	// peer's dispatched queue is empty
	enet_list_clear(&peer->dispatchedCommands);

	EXPECT_EQ(enet_peer_receive(peer, &channel_id), nullptr);

	free(peer);
}

TEST(ENetTests, enet_peer_receive_test_one_packet)
{
	ENetPeer* peer = (ENetPeer*)malloc(sizeof(ENetPeer));

	ENetIncomingCommand* dispatched_command =
	    (ENetIncomingCommand*)malloc(sizeof(ENetIncomingCommand));

	dispatched_command->packet = (ENetPacket*)malloc(sizeof(ENetPacket));
	dispatched_command->packet->referenceCount = 1;
	enet_uint8* data = (uint8_t*)("this is packet data");
	dispatched_command->packet->data = data;
	dispatched_command->packet->dataLength = strlen((const char*)data);
	dispatched_command->command.header.channelID = 2;

	enet_list_clear(&peer->dispatchedCommands);
	enet_list_insert(enet_list_end(&peer->dispatchedCommands), dispatched_command);

	enet_uint8 channel_id;
	ENetPacket* received_packet = enet_peer_receive(peer, &channel_id);

	EXPECT_EQ(channel_id, 2);
	EXPECT_EQ(received_packet, dispatched_command->packet);
	EXPECT_EQ(received_packet->referenceCount, 0);
	EXPECT_EQ(received_packet->data, data);
	EXPECT_EQ(received_packet->dataLength, dispatched_command->packet->dataLength);

	free(peer);
}