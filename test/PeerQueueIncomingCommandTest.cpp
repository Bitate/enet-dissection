#include <gtest/gtest.h>

#define ENET_IMPLEMENTATION
#include "../include/enet.h"

TEST(ENetTests,
     enet_peer_queue_incoming_command_test_reliable_packet_sequence_number_roll_over_mechanism)
{
	ENetPeer* peer;
	ENetProtocol* command;
	ENetIncomingCommand* dummy_command;

	peer = (ENetPeer*)malloc(sizeof(ENetPeer));
	command = (ENetProtocol*)malloc(sizeof(ENetProtocol));
	peer->channels = (ENetChannel*)malloc(sizeof(ENetChannel));
	command->header.channelID = 0;

	peer->state = ENET_PEER_STATE_DISCONNECT_LATER;
	dummy_command = enet_peer_queue_incoming_command(peer, command, nullptr, 0, 0, 0);

	// successfully update channel's incoming reliable sequence number based
	// upon command's reliable sequence number.
	peer->state = ENET_PEER_STATE_CONNECTED;
	peer->host = (ENetHost*)malloc(sizeof(ENetHost));
	peer->totalWaitingData = 0;
	peer->host->maximumWaitingData = 10;
	peer->channels->incomingReliableSequenceNumber = 65500;
	enet_list_clear(&peer->dispatchedCommands);
	enet_list_clear(&peer->channels->incomingUnreliableCommands);
	enet_list_clear(&peer->channels->incomingReliableCommands);
	enet_list_clear(&peer->host->dispatchQueue);
	command->header.reliableSequenceNumber = 65501;
	command->header.command = ENET_PROTOCOL_COMMAND_SEND_RELIABLE;
	EXPECT_NE(
	    enet_peer_queue_incoming_command(peer, command, nullptr, 0, ENET_PACKET_FLAG_RELIABLE, 0),
	    dummy_command);

	// discard duplicate packet
	command->header.reliableSequenceNumber = 65500;
	command->header.command = ENET_PROTOCOL_COMMAND_SEND_RELIABLE;
	EXPECT_EQ(
	    enet_peer_queue_incoming_command(peer, command, nullptr, 0, ENET_PACKET_FLAG_RELIABLE, 0),
	    dummy_command);
}