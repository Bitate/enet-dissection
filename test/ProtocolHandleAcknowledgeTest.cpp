#include <gtest/gtest.h>

#define ENET_IMPLEMENTATION
#include "../include/enet.h"

TEST(ProtocolHandleAcknowledgeTests, test)
{
	ENetHost* host = (ENetHost*)malloc(sizeof(ENetHost));
	host->serviceTime = 1000;

	ENetEvent* event = (ENetEvent*)malloc(sizeof(ENetEvent));

	ENetPeer* peer = (ENetPeer*)malloc(sizeof(ENetPeer));
	peer->state = ENET_PEER_STATE_CONNECTED;

	ENetProtocol* command = (ENetProtocol*)malloc(sizeof(ENetProtocol));
	command->acknowledge.receivedSentTime = ENET_HOST_TO_NET_16(1000);
	enet_protocol_handle_acknowledge(host, event, peer, command);
}