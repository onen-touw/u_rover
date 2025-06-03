#pragma once

#include "crtypes.h"
#include "crthelp.h"
#include <type_traits>

namespace crt
{
	// now support only four (4) arguments

	template <typename cmd_t, std::enable_if_t<std::is_enum_v<cmd_t> && sizeof(cmd_t) == sizeof(char), bool> = true> 
	class encrypte_t : protected crc16_t
	{
	public:
		using int_t = int32_t;
		using uint_t = uint32_t;

	private:
		uint8_t *_buf = nullptr;
		uint8_t _iter = 0;
		uint8_t _cnt = 0;

	public:
		encrypte_t()
		{
			_buf = new uint8_t[cfg::buf_sz]();
		}

		~encrypte_t()
		{
			if (_buf)
			{
				delete[] _buf;
			}
		}
		template <typename Ty, typename... Args>
		void pack(cmd_t cmd, Ty arg, Args... args)
		{
			if (!_cnt)
			{
				start_pack(cmd);
			}
			crypt_in_ufo(arg);
			pack(args...);
		}

		template <typename Ty>
		void pack(cmd_t cmd, Ty arg)
		{
			start_pack(cmd);
			crypt_in_ufo(arg);
			end_pack();
		}

		uint8_t *get()
		{
			return _buf;
		}

		uint8_t size() const
		{
			return _iter;
		}

		void reset(){
			_iter = 0;
		}

		void log()
		{
			printf("\nencrypte_t::log:\n");
			printf("No\t(dec)\t[val]\n");
			uint16_t c = 0;
			
			uint16_t ssz = _buf[cfg::it_sz];
			uint16_t cnt = _buf[cfg::it_cnt];
			printf("%u\tin(%u): [%c]\n", ++c, _buf[0], _buf[0]);
			printf("%u\tcmd(%u): [%c]\n", ++c, _buf[1], _buf[1]);
			printf("%u\tsz: [%u]\n", ++c, ssz);
			printf("%u\tcnt: [%u]\n", ++c, cnt);
			printf("%u\tty (%u): [%u] [%u] [%u] [%u]\n", 
				++c,
				_buf[cfg::it_ty],
				(_buf[cfg::it_ty] >> 6) & 0b11,
				(_buf[cfg::it_ty] >> 4) & 0b11,
				(_buf[cfg::it_ty] >> 2) & 0b11,
				_buf[cfg::it_ty] & 0b11
			);

			if (ssz < cfg::it_data + 1)
			{
				printf("sz err\n");
				return;
			}
			printf("\n");

			for (size_t i = cfg::it_data, ii = 0; i < ssz - 2ull; i++, ii++)
			{
				printf("%u\t\t%u [%u]\n", ++c, ii, _buf[i]);
				if (ii % 4 == 3)
				{
					printf("\n");
				}
			}
			printf("%u,%u\tcrc(%u, %u): [%u]\n", c+1, c+2, _buf[ssz - 2ull], _buf[ssz - 1ull], _buf[ssz - 2ull] | _buf[ssz - 1ull] << 8);
		}



	private:
		void start_pack(cmd_t cmd)
		{
			_iter = 0;
			_buf[_iter++] = '>';
			_buf[_iter++] = static_cast<uint8_t>(cmd);
			_buf[_iter++] = 0; // reserved for sz
			_buf[_iter++] = 0; // reserved for cnt
			_buf[_iter++] = 0; // reserved for types
		}

		void end_pack()
		{
			_buf[cfg::it_cnt] = _cnt; // cnt
			_buf[cfg::it_sz] = _iter; // sz		set it for crc calc

			crc(_buf[cfg::it_in]);
			crc(_buf[cfg::it_cmd]);
			crc(_buf[cfg::it_sz]);
			crc(_buf[cfg::it_cnt]);
			crc(_buf[cfg::it_ty]);

			_buf[_iter++] = crc_get() & 0xff;		 // add crc
			_buf[_iter++] = (crc_get() >> 8) & 0xff; // add crc

			_buf[cfg::it_sz] = _iter; // sz		update with crc
			_cnt = 0;
			crc_reset();
		}

		template <typename Ty, typename... Args>
		void pack(Ty arg, Args... args)
		{
			crypt_in_ufo(arg);
			pack(args...);
		}

		template <typename Ty>
		void pack(Ty arg)
		{
			crypt_in_ufo(arg);
			end_pack();
		}

		//	

		void crypt_in_ufo(int8_t arg)
		{
			crypt_in_ufo(static_cast<uint8_t>(arg));
		}

		void crypt_in_ufo(uint8_t arg)
		{
			_buf[cfg::it_ty] |= (cfg::arg_t::i8 << (_cnt * 2));
			_buf[_iter] = arg;
			crc(_buf[_iter]);

			++_iter;
			++_cnt;
		}

		void crypt_in_ufo(int16_t arg){
			crypt_in_ufo(static_cast<uint16_t>(arg));
		}

		void crypt_in_ufo(uint16_t arg)
		{
			_buf[cfg::it_ty] |= (cfg::arg_t::i16 << (_cnt * 2));
			for (uint_t i = 0; i < sizeof(uint16_t); i++)
			{
				_buf[_iter] = (arg >> i * 8) & 255;
				crc(_buf[_iter]);

				++_iter;
			}
			++_cnt;
		}

		void crypt_in_ufo(int32_t arg)
		{
			crypt_in_ufo(static_cast<uint32_t>(arg));
		}

		void crypt_in_ufo(uint32_t arg)
		{
			_buf[cfg::it_ty] |= (cfg::arg_t::i32 << (_cnt * 2));
			for (uint_t i = 0; i < sizeof(int32_t); i++)
			{
				_buf[_iter] = (arg >> i * 8) & 255;
				crc(_buf[_iter]);

				++_iter;
			}
			++_cnt;
		}

		void crypt_in_ufo(float arg)
		{
			_buf[cfg::it_ty] |= (cfg::arg_t::f32 << (_cnt * 2));

			int32_t in = ftob(arg);
			for (uint_t i = 0; i < 4; i++)
			{
				_buf[_iter] = (in >> i * 8) & 255;
				crc(_buf[_iter]);
				
				++_iter;
			}
			++_cnt;
		}
	};

}