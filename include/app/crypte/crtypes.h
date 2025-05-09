#pragma once
#include <stdint.h>

namespace crt {

	class cfg {
	public:
		static constexpr uint16_t buf_sz = 127;

		static constexpr uint16_t it_in = 0;
		static constexpr uint16_t it_cmd = 1;
		static constexpr uint16_t it_sz = 2;
		static constexpr uint16_t it_cnt = 3;
		static constexpr uint16_t it_ty = 4;
		static constexpr uint16_t it_data = 5;
	
		enum arg_t : uint8_t {
			i = 0b01,
			f = 0b10,
			b = 0b11,
		};
	};

}