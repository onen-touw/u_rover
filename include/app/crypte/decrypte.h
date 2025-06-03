#pragma once

#include "crtypes.h"
#include "crthelp.h"

namespace crt {

	template <typename cmd_t, std::enable_if_t<std::is_enum_v<cmd_t> && sizeof(cmd_t) == sizeof(char), bool> = true> 
	class decrypte_t
	{
	public:
		using handle_t = void(*)(cmd_t, uint8_t*);

	public:

		decrypte_t() 
		{}

		~decrypte_t() {}

		static void unpack(uint8_t* buf, uint8_t sz, handle_t handle) {
	
			if (sz < cfg::it_data)
			{
				// ufo::Trace_t::log("inc sz error\n");
				printf("inc sz error\n");
				return;
			}
			if (sz != buf[cfg::it_sz])
			{
				// ufo::Trace_t::log("error: bad_packet\n");
				printf("error: bad_packet, len: %u %u\n", sz, buf[cfg::it_sz]);
				return;
			}

			uint16_t crc = crc16_t::crc16_arr(buf + 5, sz - 7);
			crc = crc16_t::crc16_byte(crc, buf[cfg::it_in]);
			crc = crc16_t::crc16_byte(crc, buf[cfg::it_cmd]);
			crc = crc16_t::crc16_byte(crc, buf[cfg::it_sz] - 2);
			crc = crc16_t::crc16_byte(crc, buf[cfg::it_cnt]);
			crc = crc16_t::crc16_byte(crc, buf[cfg::it_ty]);
			uint16_t q = buf[sz - 2] | (buf[sz - 1] << 8);

			if (q != crc)
			{
				// ufo::Trace_t::log("error: bad_crc\n");
				printf("error: bad_crc %u\n", q);
				return;
			}
			uint8_t cnt = buf[cfg::it_cnt];
			

			// parse in
			if (buf[cfg::it_in] == '>')
			{
				if (!cnt)
				{
					return;
				}
				
				cmd_t cmd = static_cast<cmd_t>(buf[cfg::it_cmd]);
				handle(cmd, buf);
			}
			else if (buf[cfg::it_in] == '!')
			{
				// ufo::Trace_t::log("error: !not implemented\n");
				printf("error: !not implemented\n");
			}
			else
			{
				// ufo::Trace_t::log("error: bad_packet\n");
				printf("error: bad_packet\n");
			}
		}

		static void log(uint8_t* buf, uint8_t sz)
		{
			uint16_t ssz = buf[cfg::it_sz];
			uint16_t cnt = buf[cfg::it_cnt];
			printf("in %u\n", buf[0]);
			printf("cmd %u\n", buf[1]);
			printf("sz %u\n", ssz);
			printf("cnt %u\n", cnt);
			// std::cout << _buf[0] << '\n';						  // in
			// std::cout << _buf[1] << '\n';						  // cmd
			// std::cout << "cnt: " << cnt << " sz: " << sz << '\n'; // cnt, sz
			printf("ty: %u %u %u %u\n", 
				(buf[cfg::it_ty] >> 6) & 0b11,
				(buf[cfg::it_ty] >> 4) & 0b11,
				(buf[cfg::it_ty] >> 2) & 0b11,
				buf[cfg::it_ty] & 0b11
			);

			if (ssz < cfg::it_data + 1)
			{
				printf("sz err\n");
				return;
			}

			for (size_t i = cfg::it_data; i < sz - 2ull; i++)
			{
				printf("\t%u\n", buf[i]);
				// std::cout << "\t" << std::bitset<8>(_buf[i]) << '\n';
				if (i % 4 == 0)
				{
					// std::cout << "\n";
					printf("\n");
				}
			}
			printf("crc %u, %u\n", buf[sz - 1ull], buf[sz - 2ull]);
			// std::cout << "crc: " << static_cast<int>(_buf[sz - 1ull]) << '\n';
		}
	};
}