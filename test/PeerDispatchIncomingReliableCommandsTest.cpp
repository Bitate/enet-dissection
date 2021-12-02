#include <gtest/gtest.h>

#define ENET_IMPLEMENTATION
#include "../include/enet.h"

TEST(PeerDispatchIncomingReliableCommandsTests, test_roll_over_channel_reliable_sequence_number)
{
	ENetPeer* peer = (ENetPeer*)malloc(sizeof(ENetPeer));
	peer->channels = (ENetChannel*)malloc(sizeof(ENetChannel));
	peer->channelCount = 1;
	peer->needsDispatch = 1;

	ENetIncomingCommand* queued_incoming_command =
	    (ENetIncomingCommand*)malloc(sizeof(ENetIncomingCommand));

	queued_incoming_command->reliableSequenceNumber = 0;
	queued_incoming_command->fragmentsRemaining = 0;
	queued_incoming_command->fragmentCount = 0;
	peer->channels->incomingReliableSequenceNumber = 65535;

	enet_list_clear(&peer->dispatchedCommands);
	enet_list_clear(&peer->channels->incomingReliableCommands);
	enet_list_clear(&peer->channels->incomingUnreliableCommands);
	enet_list_insert(enet_list_end(&peer->channels->incomingReliableCommands),
	                 queued_incoming_command);

	enet_peer_dispatch_incoming_reliable_commands(peer, peer->channels);

	EXPECT_EQ(peer->channels->incomingReliableSequenceNumber, 0);

	// This is the trick used in enet_peer_dispatch_incoming_reliable_commands() to handle sequence
	// number roll over scenario
	enet_uint16 max_sequence_number = 65535;
	EXPECT_EQ((enet_uint16)(max_sequence_number + 1), 0);
}