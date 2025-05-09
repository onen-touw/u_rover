#pragma once

#include "crtypes.h"
#include "cmds.h"
#include "crthelp.h"

namespace crt {


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
                // std::cout << "error\n";
				ufo::Trace_t::log("inc sz error\n");
				return;
			}
			if (sz != buf[cfg::it_sz])
			{
                // std::cout << "error: bad_packet\n";
				ufo::Trace_t::log("error: bad_packet\n");
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
				printf("error: bad_crc\n");
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
                // std::cout << "error: !not implemented\n";
				ufo::Trace_t::log("error: !not implemented\n");
			}
			else
			{
                // std::cout << "error: bad_packet\n";
				ufo::Trace_t::log("error: bad_packet\n");
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

			for (size_t i = cfg::it_data; i < sz - 1ull; i++)
			{
				printf("\t%u\n", buf[i]);
				// std::cout << "\t" << std::bitset<8>(_buf[i]) << '\n';
				if (i % 4 == 0)
				{
					// std::cout << "\n";
					printf("\n");
				}
			}
			printf("crc %u\n", buf[sz - 1ull]);
			// std::cout << "crc: " << static_cast<int>(_buf[sz - 1ull]) << '\n';
		}
	};

}
