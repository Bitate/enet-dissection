#include <gtest/gtest.h>

#define ENET_IMPLEMENTATION
#include "../include/enet.h"

TEST(ENetTests, enet_protocol_handle_incoming_commands_test)
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

	ENetHost* host = (ENetHost*)malloc(sizeof(ENetHost));
	host->receivedData = packet;
	host->receivedDataLength = data_length;

	ENetEvent* event;
	enet_protocol_handle_incoming_commands(host, event);
}