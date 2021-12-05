#include <gtest/gtest.h>

#define ENET_IMPLEMENTATION
#include "../include/enet.h"

#include <iostream>
#include <vector>

using namespace std;

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

	vector<int> existing_queued_commands_sequence_numbers = {1, 3, 4, 5, 6};

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

TEST(PeerQueueIncomingCommandTests, roll_over_reliable_sequence_number_insert_into_existing_queue)
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

	vector<int> existing_queued_commands_sequence_numbers = {1, 2, 3, 4, 5, 6};

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

	vector<int> generated_queued_commands_sequence_numbers;

	for (ENetListIterator current_command_iterator = enet_list_begin(&peer->dispatchedCommands);
	     current_command_iterator != enet_list_end(&peer->dispatchedCommands);
	     current_command_iterator = enet_list_next(current_command_iterator))
	{
		ENetIncomingCommand* current_incoming_command =
		    (ENetIncomingCommand*)(current_command_iterator);

		generated_queued_commands_sequence_numbers.push_back(
		    current_incoming_command->reliableSequenceNumber);
	}

	EXPECT_EQ(enet_list_size(&peer->dispatchedCommands), 7);
	vector<int> dispatched_sequence_numbers = {0, 1, 2, 3, 4, 5, 6};
	EXPECT_EQ(dispatched_sequence_numbers, generated_queued_commands_sequence_numbers);
}

TEST(PeerQueueIncomingCommandTests, roll_over_reliable_sequence_number_appended_to_the_queue)
{
	ENetPeer* peer = (ENetPeer*)malloc(sizeof(ENetPeer));
	ENetHost* host = (ENetHost*)malloc(sizeof(ENetHost));
	peer->channels = (ENetChannel*)malloc(sizeof(ENetChannel));
	peer->totalWaitingData = 0;
	peer->host = host;
	peer->host->maximumWaitingData = 10;
	peer->channels->incomingReliableSequenceNumber = 65530;
	peer->state = ENET_PEER_STATE_CONNECTED;

	enet_list_clear(&peer->dispatchedCommands);
	enet_list_clear(&peer->channels->incomingUnreliableCommands);
	enet_list_clear(&peer->channels->incomingReliableCommands);
	enet_list_clear(&peer->host->dispatchQueue);

	vector<int> existing_queued_commands_sequence_numbers = {65531, 65532, 65533, 65534, 65535};

	for (int sequence_number : existing_queued_commands_sequence_numbers)
	{
		ENetIncomingCommand* command = (ENetIncomingCommand*)malloc(sizeof(ENetIncomingCommand));
		command->reliableSequenceNumber = sequence_number;
		enet_list_insert(enet_list_end(&peer->channels->incomingReliableCommands), command);
	}

	ENetProtocol* current_command = (ENetProtocol*)malloc(sizeof(ENetProtocol));
	current_command->header.command = ENET_PROTOCOL_COMMAND_SEND_RELIABLE;
	current_command->header.reliableSequenceNumber = 0;

	ENetIncomingCommand* queued_incoming_command = enet_peer_queue_incoming_command(
	    peer, current_command, nullptr, 0, ENET_PACKET_FLAG_RELIABLE, 0);

	EXPECT_NE(queued_incoming_command, nullptr);
	EXPECT_NE(queued_incoming_command, get_dummy_command());

	EXPECT_EQ(peer->channels->incomingReliableSequenceNumber, 0);

	vector<int> generated_queued_commands_sequence_numbers;

	for (ENetListIterator current_command_iterator = enet_list_begin(&peer->dispatchedCommands);
	     current_command_iterator != enet_list_end(&peer->dispatchedCommands);
	     current_command_iterator = enet_list_next(current_command_iterator))
	{
		ENetIncomingCommand* current_incoming_command =
		    (ENetIncomingCommand*)(current_command_iterator);

		generated_queued_commands_sequence_numbers.push_back(
		    current_incoming_command->reliableSequenceNumber);
	}

	EXPECT_EQ(enet_list_size(&peer->dispatchedCommands), 6);
	vector<int> dispatched_sequence_numbers = {65531, 65532, 65533, 65534, 65535, 0};
	EXPECT_EQ(dispatched_sequence_numbers, generated_queued_commands_sequence_numbers);
}

TEST(PeerQueueIncomingCommandTests, maximum_size_of_incoming_commmands_queue)
{
	ENetPeer* peer = (ENetPeer*)malloc(sizeof(ENetPeer));
	ENetProtocol* command = (ENetProtocol*)malloc(sizeof(ENetProtocol));
	command->header.channelID = 0;
	command->header.command = ENET_PROTOCOL_COMMAND_SEND_RELIABLE;
	peer->channels = (ENetChannel*)malloc(sizeof(ENetChannel));
	peer->state = ENET_PEER_STATE_CONNECTED;
	peer->host = (ENetHost*)malloc(sizeof(ENetHost));
	peer->totalWaitingData = 0;
	peer->host->maximumWaitingData = 10;

	enet_list_clear(&peer->dispatchedCommands);
	enet_list_clear(&peer->channels->incomingUnreliableCommands);
	enet_list_clear(&peer->channels->incomingReliableCommands);
	enet_list_clear(&peer->host->dispatchQueue);

	ENetIncomingCommand* queued_command;
	ENetIncomingCommand* dummy_command = get_dummy_command();

	constexpr int WINDOW_15_STARTING_SEQUENCE_NUMBER =
	    (ENET_PEER_RELIABLE_WINDOWS - 1) * ENET_PEER_RELIABLE_WINDOW_SIZE;

	peer->channels->incomingReliableSequenceNumber = WINDOW_15_STARTING_SEQUENCE_NUMBER;

	// +2 is to prevent channel->incomingReliableSequenceNumber to update.
	for (int i = WINDOW_15_STARTING_SEQUENCE_NUMBER + 2;
	     i < ENET_PEER_RELIABLE_WINDOW_SIZE * ENET_PEER_RELIABLE_WINDOWS; ++i)
	{
		command->header.reliableSequenceNumber = i;
		queued_command = enet_peer_queue_incoming_command(peer, command, nullptr, 0,
		                                                  ENET_PACKET_FLAG_RELIABLE, 0);
		EXPECT_NE(queued_command, nullptr) << "valid command test failed at index: " << i;
		EXPECT_NE(queued_command, dummy_command) << "valid command test failed at index: " << i;
	}

	// if we swap the two for loop, this test case will consume about 3s.
	// TODO: there is performance issue here concerning the iteration of linked list.
	for (int i = 0; i < WINDOW_15_STARTING_SEQUENCE_NUMBER; ++i)
	{
		command->header.reliableSequenceNumber = i;
		queued_command = enet_peer_queue_incoming_command(peer, command, nullptr, 0,
		                                                  ENET_PACKET_FLAG_RELIABLE, 0);

		if (i < ENET_PEER_RELIABLE_WINDOW_SIZE * 6)
		{
			EXPECT_NE(queued_command, nullptr) << "valid command test failed at index: " << i;
			EXPECT_NE(queued_command, dummy_command) << "valid command test failed at index: " << i;
		}
		else
		{
			ASSERT_EQ(queued_command, dummy_command)
			    << "discard command test failed at index: " << i;
		}
	}

	EXPECT_EQ(peer->channels->incomingReliableSequenceNumber, WINDOW_15_STARTING_SEQUENCE_NUMBER);
	EXPECT_EQ(enet_list_size(&peer->dispatchedCommands), 0);

	// incomingReliableCommands contains at most (ENET_PEER_RELIABLE_WINDOW_SIZE * 7 - 2) commands
	EXPECT_EQ(enet_list_size(&peer->channels->incomingReliableCommands),
	          ENET_PEER_RELIABLE_WINDOW_SIZE * 7 - 2);
}