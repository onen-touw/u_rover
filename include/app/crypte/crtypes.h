#pragma once
#include <stdint.h>

namespace crt {

	enum arg_ind : uint8_t {
		arg_1,
		arg_2,
		arg_3,
		arg_4,
	};

	class cfg {
	public:
		static constexpr uint16_t buf_sz = 127;

		static constexpr uint16_t it_in = 0;
		static constexpr uint16_t it_cmd = 1;
		static constexpr uint16_t it_sz = 2;
		static constexpr uint16_t it_cnt = 3;
		static constexpr uint16_t it_ty = 4;
		static constexpr uint16_t it_data = 5;

		static constexpr uint16_t max_args = 4;

		enum arg_t : uint8_t {
			i8 = 0b00,
			i16 = 0b01,
			i32 = 0b10,
			f32 = 0b11,
		};
		
	};

}