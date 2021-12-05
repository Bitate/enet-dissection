#include <gtest/gtest.h>

#define ENET_IMPLEMENTATION
#include "../include/enet.h"

TEST(PeerThrottleTests,
     peer_last_round_trip_time_less_than_or_equal_to_peer_last_round_trip_time_variance)
{
	ENetPeer* peer = (ENetPeer*)malloc(sizeof(ENetPeer));

	peer->lastRoundTripTime = 400;
	peer->lastRoundTripTimeVariance = 500;
	peer->packetThrottleLimit = ENET_PEER_PACKET_THROTTLE_SCALE;

	enet_peer_throttle(peer, 0);

	EXPECT_EQ(peer->packetThrottle, peer->packetThrottleLimit);
}

TEST(PeerThrottleTests, increase_packet_throttle)
{
	ENetPeer* peer = (ENetPeer*)malloc(sizeof(ENetPeer));

	peer->packetThrottleLimit = ENET_PEER_PACKET_THROTTLE_SCALE;
	peer->packetThrottleAcceleration = ENET_PEER_PACKET_THROTTLE_ACCELERATION;
	peer->packetThrottleDeceleration = ENET_PEER_PACKET_THROTTLE_DECELERATION;

	peer->lastRoundTripTime = 500;
	peer->lastRoundTripTimeVariance = 400;
	peer->packetThrottle = ENET_PEER_PACKET_THROTTLE_SCALE - ENET_PEER_PACKET_THROTTLE_ACCELERATION;
	enet_uint32 rtt = 400;

	enet_peer_throttle(peer, rtt);

	EXPECT_EQ(peer->packetThrottle, ENET_PEER_PACKET_THROTTLE_SCALE);
}

TEST(PeerThrottleTests, increase_packet_throttle_at_maximum_value)
{
	ENetPeer* peer = (ENetPeer*)malloc(sizeof(ENetPeer));

	peer->packetThrottleLimit = ENET_PEER_PACKET_THROTTLE_SCALE;
	peer->packetThrottleAcceleration = ENET_PEER_PACKET_THROTTLE_ACCELERATION;
	peer->packetThrottleDeceleration = ENET_PEER_PACKET_THROTTLE_DECELERATION;

	peer->lastRoundTripTime = 500;
	peer->lastRoundTripTimeVariance = 400;
	peer->packetThrottle = ENET_PEER_PACKET_THROTTLE_SCALE;
	enet_uint32 rtt = 400;

	enet_peer_throttle(peer, rtt);

	EXPECT_EQ(peer->packetThrottle, ENET_PEER_PACKET_THROTTLE_SCALE);
}

TEST(PeerThrottleTests, decrease_packet_throttle)
{
	ENetPeer* peer = (ENetPeer*)malloc(sizeof(ENetPeer));

	peer->packetThrottleLimit = ENET_PEER_PACKET_THROTTLE_SCALE;
	peer->packetThrottleAcceleration = ENET_PEER_PACKET_THROTTLE_ACCELERATION;
	peer->packetThrottleDeceleration = ENET_PEER_PACKET_THROTTLE_DECELERATION;

	peer->lastRoundTripTime = 500;
	peer->lastRoundTripTimeVariance = 400;
	peer->packetThrottle = ENET_PEER_PACKET_THROTTLE_SCALE;
	enet_uint32 rtt = 1400;

	enet_peer_throttle(peer, rtt);

	EXPECT_EQ(peer->packetThrottle,
	          ENET_PEER_PACKET_THROTTLE_SCALE - ENET_PEER_PACKET_THROTTLE_DECELERATION);
}