#include <gtest/gtest.h>

#define ENET_IMPLEMENTATION
#include "../include/enet.h"

#include <bitset>
#include <iostream>
#include <thread>

using namespace std;

TEST(TimeTests, get_time) { EXPECT_EQ(enet_time_get(), 1); }

TEST(TimeTests, cast_16_bit_time_to_32_bit_time)
{
	// 65535(0xFFFF in hexadecimal) seconds is 18 hours, 12 minutes and 15 seconds.
	enet_uint32 received_sent_time = 0xFFFF;

	// 0x12348678 is a randomly chosen number that is greater than 0x0000FFFF.
	enet_uint32 service_time = 0x12348678;

	// received_sent_time is assigned from a 16-bit unsigned number but service_time is 32-bit in
	// length. The two are not comparable bit by bit. Here we try to restore the 32-bit length
	// of received_sent_time by copy the upper 16-bit from service_time.
	received_sent_time |= service_time & 0xFFFF0000;

	EXPECT_EQ(received_sent_time, 0x1234FFFF);

	// because upper 16 bits are same. If the 16th bit is set in received_sent_time but not in
	// service_time. Then received_sent_time is greater than service_time. But, this is contrary
	// to the fact that received_sent_time must be less than service time.
	if ((received_sent_time & 0x8000) > (service_time & 0x8000))
	{
		// The 17th bit is the carray bit. Due to the casting from 32 bit ENet epoch time to 16 bit
		// ENet epoch time perfomred on sending host, the upper 16 bits (17th ~ 32th bits) are lost.
		// Here, we decreate the 17th bit by 1 to maintain the property that received_sent_time <=
		// service_time
		received_sent_time -= 0x10000;
	}

	EXPECT_EQ(received_sent_time, 0x1234FFFF);
	EXPECT_TRUE(ENET_TIME_GREATER(received_sent_time, service_time));

	service_time = 0x12347678;
	received_sent_time |= service_time & 0xFFFF0000;

	EXPECT_EQ(received_sent_time, 0x1234FFFF);

	if ((received_sent_time & 0x8000) > (service_time & 0x8000))
	{
		received_sent_time -= 0x10000;
	}

	EXPECT_EQ(received_sent_time, 0x1233FFFF);
	EXPECT_TRUE(ENET_TIME_LESS(received_sent_time, service_time));
}
