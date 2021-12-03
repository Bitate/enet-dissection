#include <gtest/gtest.h>

#define ENET_IMPLEMENTATION
#include "../include/enet.h"

#include <vector>

ENetIncomingCommand* get_dummy_command()
{
	ENetPeer* peer;
	ENetProtocol* command;

	peer = (ENetPeer*)malloc(sizeof(ENetPeer));
	command = (ENetProtocol*)malloc(sizeof(ENetProtocol));
	peer->channels = (ENetChannel*)malloc(sizeof(ENetChannel));
	command->header.channelID = 0;

	peer->state = ENET_PEER_STATE_DISCONNECT_LATER;

	ENetIncomingCommand* dummy_command =
	    enet_peer_queue_incoming_command(peer, command, nullptr, 0, 0, 0);

	free(peer);
	free(command);

	return dummy_command;
}

TEST(PeerQueueIncomingCommandTests, reliable_packet_normal_sequence_number)
{
	ENetPeer* peer = (ENetPeer*)malloc(sizeof(ENetPeer));
	ENetProtocol* command = (ENetProtocol*)malloc(sizeof(ENetProtocol));
	peer->channels = (ENetChannel*)malloc(sizeof(ENetChannel));
	command->header.channelID = 0;

	// successfully update channel's incoming reliable sequence number based
	// upon command's reliable sequence number.
	peer->state = ENET_PEER_STATE_CONNECTED;
	peer->host = (ENetHost*)malloc(sizeof(ENetHost));
	peer->totalWaitingData = 0;
	peer->host->maximumWaitingData = 10;

	enet_list_clear(&peer->dispatchedCommands);
	enet_list_clear(&peer->channels->incomingUnreliableCommands);
	enet_list_clear(&peer->channels->incomingReliableCommands);
	enet_list_clear(&peer->host->dispatchQueue);

	command->header.reliableSequenceNumber = 65501;
	peer->channels->incomingReliableSequenceNumber = 65500;
	command->header.command = ENET_PROTOCOL_COMMAND_SEND_RELIABLE;

	ENetIncomingCommand* result =
	    enet_peer_queue_incoming_command(peer, command, nullptr, 0, ENET_PACKET_FLAG_RELIABLE, 0);

	EXPECT_NE(result, nullptr);
	EXPECT_NE(result, get_dummy_command());
}

TEST(PeerQueueIncomingCommandTests, reliable_packet_duplicate_sequence_number)
{
	ENetPeer* peer = (ENetPeer*)malloc(sizeof(ENetPeer));
	ENetProtocol* command = (ENetProtocol*)malloc(sizeof(ENetProtocol));
	peer->channels = (ENetChannel*)malloc(sizeof(ENetChannel));
	command->header.channelID = 0;

	// duplicate sequence number will be discarded
	command->header.reliableSequenceNumber = 65500;
	peer->channels->incomingReliableSequenceNumber = 65500;
	peer->state = ENET_PEER_STATE_CONNECTED;
	command->header.command = ENET_PROTOCOL_COMMAND_SEND_RELIABLE;
	EXPECT_EQ(
	    enet_peer_queue_incoming_command(peer, command, nullptr, 0, ENET_PACKET_FLAG_RELIABLE, 0),
	    get_dummy_command());
}

TEST(PeerQueueIncomingCommandTests, reliable_packet_old_sequence_number)
{
	ENetPeer* peer = (ENetPeer*)malloc(sizeof(ENetPeer));
	ENetProtocol* command = (ENetProtocol*)malloc(sizeof(ENetProtocol));
	peer->channels = (ENetChannel*)malloc(sizeof(ENetChannel));
	command->header.channelID = 0;

	// duplicate sequence number will be discarded
	command->header.reliableSequenceNumber = 60000;
	peer->channels->incomingReliableSequenceNumber = 65500;
	peer->state = ENET_PEER_STATE_CONNECTED;
	command->header.command = ENET_PROTOCOL_COMMAND_SEND_RELIABLE;
	EXPECT_EQ(
	    enet_peer_queue_incoming_command(peer, command, nullptr, 0, ENET_PACKET_FLAG_RELIABLE, 0),
	    get_dummy_command());
}

TEST(PeerQueueIncomingCommandTests, reliable_packet_roll_over_sequence_number)
{
	ENetPeer* peer = (ENetPeer*)malloc(sizeof(ENetPeer));
	ENetProtocol* command = (ENetProtocol*)malloc(sizeof(ENetProtocol));
	ENetHost* host = (ENetHost*)malloc(sizeof(ENetHost));
	peer->channels = (ENetChannel*)malloc(sizeof(ENetChannel));
	command->header.channelID = 0;
	peer->totalWaitingData = 0;
	peer->host = host;
	peer->host->maximumWaitingData = 10;

	enet_list_clear(&peer->dispatchedCommands);
	enet_list_clear(&peer->channels->incomingUnreliableCommands);
	enet_list_clear(&peer->channels->incomingReliableCommands);
	enet_list_clear(&peer->host->dispatchQueue);

	// duplicate sequence number will be discarded
	command->header.reliableSequenceNumber = 1;
	peer->channels->incomingReliableSequenceNumber = 65500;
	peer->state = ENET_PEER_STATE_CONNECTED;
	command->header.command = ENET_PROTOCOL_COMMAND_SEND_RELIABLE;

	ENetIncomingCommand* queued_incoming_command =
	    enet_peer_queue_incoming_command(peer, command, nullptr, 0, ENET_PACKET_FLAG_RELIABLE, 0);

	EXPECT_NE(queued_incoming_command, nullptr);
	EXPECT_NE(queued_incoming_command, get_dummy_command());

	EXPECT_EQ(queued_incoming_command->reliableSequenceNumber, 1);
	EXPECT_EQ(peer->channels->incomingReliableSequenceNumber, 65500);
}

TEST(PeerQueueIncomingCommandTests, insert_absent_command_in_incoming_commands_queue)
{
	ENetPeer* peer = (ENetPeer*)malloc(sizeof(ENetPeer));
	ENetHost* host = (ENetHost*)malloc(sizeof(ENetHost));
	peer->channels = (ENetChannel*)malloc(sizeof(ENetChannel));
	peer->totalWaitingData = 0;
	peer->host = host;
	peer->host->maximumWaitingData = 10;
	peer->channels->incomingReliableSequenceNumber = 0;
	peer->state = ENET_PEER_STATE_CONNECTED;

	enet_list_clear(&peer->dispatchedCommands);
	enet_list_clear(&peer->channels->incomingUnreliableCommands);
	enet_list_clear(&peer->channels->incomingReliableCommands);
	enet_list_clear(&peer->host->dispatchQueue);

	std::vector<int> existing_queued_commands_sequence_numbers = {1, 3, 4, 5, 6};

	for (int sequence_number : existing_queued_commands_sequence_numbers)
	{
		ENetIncomingCommand* command = (ENetIncomingCommand*)malloc(sizeof(ENetIncomingCommand));
		command->reliableSequenceNumber = sequence_number;
		enet_list_insert(enet_list_end(&peer->channels->incomingReliableCommands), command);
	}

	ENetProtocol* current_command = (ENetProtocol*)malloc(sizeof(ENetProtocol));
	current_command->header.command = ENET_PROTOCOL_COMMAND_SEND_RELIABLE;
	current_command->header.reliableSequenceNumber = 2;

	/**
	 * In this case, the current_command will be inserted into the queue at index of 1.
	 * Thus, the resulting incoming command queue is: 1 2 3 4 5 6
	 */
	ENetIncomingCommand* queued_incoming_command = enet_peer_queue_incoming_command(
	    peer, current_command, nullptr, 0, ENET_PACKET_FLAG_RELIABLE, 0);

	EXPECT_NE(queued_incoming_command, nullptr);
	EXPECT_NE(queued_incoming_command, get_dummy_command());

	EXPECT_EQ(peer->channels->incomingReliableSequenceNumber, 6);
}

// TEST: fragmented commands

TEST(PeerQueueIncomingCommandTests, roll_over_incoming_command_dispatched_by_the_incoming_queue)
{
	ENetPeer* peer = (ENetPeer*)malloc(sizeof(ENetPeer));
	ENetHost* host = (ENetHost*)malloc(sizeof(ENetHost));
	peer->channels = (ENetChannel*)malloc(sizeof(ENetChannel));
	peer->totalWaitingData = 0;
	peer->host = host;
	peer->host->maximumWaitingData = 10;
	peer->channels->incomingReliableSequenceNumber = 65535;
	peer->state = ENET_PEER_STATE_CONNECTED;

	enet_list_clear(&peer->dispatchedCommands);
	enet_list_clear(&peer->channels->incomingUnreliableCommands);
	enet_list_clear(&peer->channels->incomingReliableCommands);
	enet_list_clear(&peer->host->dispatchQueue);

	std::vector<int> existing_queued_commands_sequence_numbers = {1, 2, 3, 4, 5, 6};

	for (int sequence_number : existing_queued_commands_sequence_numbers)
	{
		ENetIncomingCommand* command = (ENetIncomingCommand*)malloc(sizeof(ENetIncomingCommand));
		command->reliableSequenceNumber = sequence_number;
		enet_list_insert(enet_list_end(&peer->channels->incomingReliableCommands), command);
	}

	ENetProtocol* current_command = (ENetProtocol*)malloc(sizeof(ENetProtocol));
	current_command->header.command = ENET_PROTOCOL_COMMAND_SEND_RELIABLE;
	current_command->header.reliableSequenceNumber = 0;

	/**
	 * In this case, the current_command will be inserted into the queue at index of 1.
	 * Thus, the resulting incoming command queue is: 1 2 3 4 5 6
	 */
	ENetIncomingCommand* queued_incoming_command = enet_peer_queue_incoming_command(
	    peer, current_command, nullptr, 0, ENET_PACKET_FLAG_RELIABLE, 0);

	EXPECT_NE(queued_incoming_command, nullptr);
	EXPECT_NE(queued_incoming_command, get_dummy_command());

	EXPECT_EQ(peer->channels->incomingReliableSequenceNumber, 6);

	std::vector<int> generated_queued_commands_sequence_numbers;

	EXPECT_EQ(enet_list_size(&peer->dispatchedCommands), 7);

	for (ENetListIterator current_command_iterator = enet_list_begin(&peer->dispatchedCommands);
	     current_command_iterator != enet_list_end(&peer->dispatchedCommands);
	     current_command_iterator = enet_list_next(current_command_iterator))
	{
		ENetIncomingCommand* current_incoming_command =
		    (ENetIncomingCommand*)(current_command_iterator);

		generated_queued_commands_sequence_numbers.push_back(
		    current_incoming_command->reliableSequenceNumber);
	}

	std::vector<int> dispatched_sequence_numbers = {0, 1, 2, 3, 4, 5, 6};
	EXPECT_EQ(dispatched_sequence_numbers, generated_queued_commands_sequence_numbers);
}