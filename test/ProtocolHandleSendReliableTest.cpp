#include <gtest/gtest.h>

#define ENET_IMPLEMENTATION
#include "../include/enet.h"

TEST(ENetTests, enet_protocol_handle_send_reliable_test)
{
	// prerequisite
	enet_uint8* header = (enet_uint8*)"[header]";
	size_t header_length = strlen((const char*)header);

	enet_uint8* data = (enet_uint8*)"[data]";
	size_t data_length = strlen((const char*)data);

	size_t packet_length = (header_length + data_length) * 2;

	enet_uint8* packet = (enet_uint8*)malloc(packet_length);
	memcpy(packet, header, header_length);
	memcpy(packet + header_length, data, data_length);
	memcpy(packet + header_length + data_length, packet, packet_length / 2);

	// from
	ENetHost* host = (ENetHost*)malloc(sizeof(ENetHost));
	host->receivedData = packet;
	host->receivedDataLength = packet_length;
	host->maximumPacketSize = ENET_HOST_DEFAULT_MAXIMUM_PACKET_SIZE;

	// to
	ENetPeer* peer = (ENetPeer*)malloc(sizeof(ENetPeer));
	peer->state = ENET_PEER_STATE_CONNECTED;
	peer->channels = (ENetChannel*)malloc(sizeof(ENetChannel) * 2);
	peer->channelCount = 2;

	// handle data based upon command
	ENetProtocol* command = (ENetProtocol*)malloc(sizeof(ENetProtocol));
	command->header.channelID = 0;
	command->sendReliable.dataLength = ENET_HOST_TO_NET_16(data_length);

	EXPECT_EQ(enet_protocol_handle_send_reliable(host, peer, command, &packet), 0);
}