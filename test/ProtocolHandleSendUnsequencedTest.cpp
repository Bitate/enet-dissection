#include <gtest/gtest.h>

#define ENET_IMPLEMENTATION
#include "../include/enet.h"

TEST(ENetTests, enet_protocol_handle_send_unsequenced_test)
{
	constexpr size_t DATA_LENGTH = 256;

	ENetHost* host = (ENetHost*)malloc(sizeof(ENetHost));
	host->maximumWaitingData = 10;

	ENetPeer* peer = (ENetPeer*)malloc(sizeof(ENetPeer));
	peer->host = host;
	peer->state = ENET_PEER_STATE_CONNECTED;
	peer->channelCount = 2;
	peer->incomingUnsequencedGroup = ENET_HOST_TO_NET_16(0);
	peer->totalWaitingData = 0;
	peer->channels = (ENetChannel*)malloc(sizeof(ENetChannel));

	ENetProtocol* command = (ENetProtocol*)malloc(sizeof(ENetProtocol));
	command->header.channelID = 0;
	command->sendUnsequenced.dataLength = DATA_LENGTH;
	command->sendUnsequenced.unsequencedGroup = ENET_HOST_TO_NET_16(1);
	command->header.command = ENET_PROTOCOL_COMMAND_SEND_UNSEQUENCED;

	enet_uint8* currentData = (enet_uint8*)malloc(sizeof(enet_uint8) * DATA_LENGTH);

	for (int i = 0; i < DATA_LENGTH; ++i)
	{
		currentData[i] = i;
		ASSERT_EQ(currentData[i], i) << "i is: " << i;
	}

	host->maximumPacketSize = ENET_HOST_DEFAULT_MAXIMUM_PACKET_SIZE;
	host->receivedData = currentData;
	host->receivedDataLength = DATA_LENGTH;

	enet_list_clear(&peer->dispatchedCommands);
	enet_list_clear(&peer->channels->incomingUnreliableCommands);
	enet_list_clear(&peer->channels->incomingReliableCommands);
	enet_list_clear(&peer->host->dispatchQueue);

	ASSERT_EQ(enet_protocol_handle_send_unsequenced(host, peer, command, &currentData), 0);
}