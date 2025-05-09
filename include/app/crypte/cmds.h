#pragma once

#include <stdint.h>

enum class cmd_t : uint8_t {
	none,
	request_ask,
	request_err,

	ack = 5,
	
	remote_trpy = 10,
	remote_arm,
	
};
