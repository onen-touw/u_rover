#pragma once

#include "u_sys/config.h"
#include "u_sys/list.h"
#include "u_sys/str.h"
#include "u_sys/vct.h"
#include "u_sys/thread.h"
#include "u_sys/mutex.h"
#include "u_sys/utils.h"
#include "u_sys/cns.h"

#include "u_drivers/drv.h"

#include "u_dev/m_pwm.h"

#include "crypte/decrypte.h"
#include "crypte/encrypte.h"

#include "net.h"
#include "appdata.h"

namespace app
{
    class app_t
    {
    public:

        app_t() {

        }
		app_t(app_t&&) = default;
		app_t(const app_t&) = delete;

        ~app_t() {}

        void task(ufo::token_t token){
            using namespace ufo;
			Trace_t::log("app task start\n");

			app_data_t &appd = app_data_t::get_instanse();

			net_t nett;
			nett.mk_sock(
				net_descriptors_t::sock_main,
				"192.168.0.64",
				net::uSocketType_t::UFO_SOCK_CLIENT,
				[](net::fast_sock::rcv_t *rcv)
				{
					// Trace_t::flog("rcv[%u] (%u): %s\n", ufo::utl::get_time_millis(), rcv->_len, rcv->_payload);

					crt::decrypte_t::unpack(
						reinterpret_cast<uint8_t *>(rcv->_payload),
						rcv->_len,
						[](cmd_t cmd, uint8_t *buf)
						{
							app_data_t &appd = app_data_t::get_instanse();
							if (cmd == cmd_t::remote_trpy)
							{
								ufo::lock_guard<mutex_t> _l(appd._remote._lock);

								appd._remote._throt = crt::get_arg<float>(0, buf);
								appd._remote._roll = crt::get_arg<float>(1, buf);
								appd._remote._pitch = crt::get_arg<float>(2, buf);
								appd._remote._yaw = crt::get_arg<float>(3, buf);
								appd._remote._mcmd = app::types::mot_cmd_t::mot_vals;
							}
							else if (cmd == cmd_t::remote_arm)
							{
								ufo::lock_guard<mutex_t> _l(appd._remote._lock);
								if (crt::get_arg<int32_t>(0, buf) > 0)
								{
									appd._remote._mcmd = app::types::mot_cmd_t::mot_set_arm;
								}
								else
								{
									appd._remote._mcmd = app::types::mot_cmd_t::mot_set_disarm;
								}
							}
						});
				});

			net_t::msg_block_t msg_block = nett.get_block(net_descriptors_t::sock_main);

			ufo::thread_cfg cfg_net;
			cfg_net._name = "net";
			cfg_net._core = 0;
			cfg_net._prio = 5;
			cfg_net._stackSize = 4096;
			ufo::thread_guard task_net(ufo::thread(cfg_net, &net_t::task, &nett));

			//======================== sensors-init ========================
			#pragma region // sensors
			#ifdef use_sens
						sens_t sens(&driver, msg_block);

						ufo::thread_cfg cfg_imu;
						cfg_imu._name = "imu";
						cfg_imu._core = 0;
						cfg_imu._prio = 5;
						cfg_imu._stackSize = 4096;
						ufo::thread_guard task_imu(ufo::thread(cfg_imu, &sens_t::task_imu, &sens));

						// ufo::thread_cfg cfg_bar;
						// cfg_bar._name = "bar";
						// cfg_bar._core = 0;
						// cfg_bar._prio = 5;
						// cfg_bar._stackSize = 4096;
						// ufo::thread_guard task_bar(ufo::thread(cfg_bar , &sens_t::task_bar, &sens));
			#endif
			#pragma endregion

			// send msg for a while
			// sens_t sens(&driver, nullptr);
			// ufo::thread_cfg cfg_imu;
			// cfg_imu._name = "imu";
			// cfg_imu._core = 0;
			// cfg_imu._prio = 5;
			// cfg_imu._stackSize = 4096;
			// ufo::thread_guard task_imu(ufo::thread(cfg_imu , &sens_t::task_imu, &sens));
			// ufo::thread_cfg cfg_bar;
			// cfg_bar._name = "bar";
			// cfg_bar._core = 0;
			// cfg_bar._prio = 5;
			// cfg_bar._stackSize = 4096;
			// ufo::thread_guard task_bar(ufo::thread(cfg_bar , &sens_t::task_bar, &sens));

			// lora lora(&port1);
			// UFO_LoraSettings conf = {};
			// conf._selfAddr._addh = 0;
			// conf._selfAddr._addl = 25;
			// conf._selfAddr._chan = 8;
			// conf._targAddr._addh = UFO_LORA_BROADCAST;
			// conf._targAddr._addl = UFO_LORA_BROADCAST;
			// conf._targAddr._chan = 10;
			// conf.adrt =  LORA_AIR_DATA_RATE_110_384;
			// lora.SetConfig(conf, [](lora::rcv_t* cll){
			// 	printf("rcv f10 %s\n", cll->_payload);
			// });
			// lora.Setup();
			// lora::msg_block_t lora_msg = lora.get_block();

			mpwm_t motors;

            while (token)
            {
				// function
				{
					ufo::lock_guard<mutex_t> _l(appd._remote._lock);

					app::types::mot_cmd_t mcmd = appd._remote._mcmd;

					if (mcmd != app::types::mot_cmd_t::mot_no)
					{
						if (mcmd == app::types::mot_cmd_t::mot_vals)
						{
							motors.update(
								appd._remote._pitch,
								appd._remote._roll
							);
						}
						else if (mcmd == app::types::mot_cmd_t::mot_valscns)
						{
							// use r, p like  m1 m2
							motors.target_write(
								appd._remote._roll,
								appd._remote._pitch
							);
							appd._remote._throt = 0;
							appd._remote._roll = 0;
							appd._remote._pitch = 0;
							appd._remote._yaw = 0;
						}
						appd._remote._mcmd = app::types::mot_cmd_t::mot_no;
					}
				}

				appd._rover._rr = motors.get_mot_throt_r();
				appd._rover._ll = motors.get_mot_throt_l();
				
				msg_block->fMsg("hello(iamRVR): %u", ufo::utl::get_time_millis());
				ufo::utl::sleep_for(50);
            }
        }


		void cns_init(ufo::cns::console_t & cns){
			using namespace ufo;

			cns.mk_blank(
				"rmt",
				"",
				[](cns::console_t::block_t block)
				{
					block->write("rmt was called\n");

					vector_t<string_t> &arg_list = block->get_buf();

					if (!arg_list.empty())
					{
						if (arg_list.size() > 1)
						{
							if (cns::get_obj_type(arg_list[1]) == cns::obj_t::flag)
							{
								char f = cns::get_flag(arg_list[1]);
								if (f == 'e')
								{
									uint16_t d = 50;
									if (arg_list.size() > 2)
									{
										if (cns::get_obj_type(arg_list[2]) == cns::obj_t::arg)
										{
											d = cns::get_arg<int>(arg_list[2]);
										}
									}
									block->fwrite("echo:\n\tdelay:%ums\n", d);

									app::app_data_t &_app = app::app_data_t::get_instanse();
									while (!block->is_read_out_signal())
									{
										{
											ufo::lock_guard<ufo::mutex_t> lock(_app._remote._lock);
											if (_app._remote._mcmd != app::types::mot_cmd_t::mot_no)
											{
												app::types::mot_cmd_t m = _app._remote._mcmd;
												block->fwrite("cmd: %u; payload:\n", static_cast<uint16_t>(m));
												switch (m)
												{
												case app::types::mot_cmd_t::mot_vals:
													block->fwrite(">t:%.3f\n>r:%.3f\n>p:%.3f\n>y:%.3f\n\n",
																  _app._remote._throt,
																  _app._remote._roll,
																  _app._remote._pitch,
																  _app._remote._yaw);
													break;
												case app::types::mot_cmd_t::mot_set_arm:
													block->fwrite("arm-up\n");
													break;
												case app::types::mot_cmd_t::mot_set_disarm:
													block->fwrite("arm-down\n");
													break;

												default:
													break;
												}
											}
										}
										utl::sleep_for(d);
									}
									block->write("echo stop\n");
								}
							}
						}
					}
				});

			cns.mk_blank(
				"imu",
				"",
				[](cns::console_t::block_t block)
				{
					block->write("imu was called\n");

					vector_t<string_t> &arg_list = block->get_buf();

					if (!arg_list.empty())
					{
						if (arg_list.size() > 1)
						{
							if (cns::get_obj_type(arg_list[1]) == cns::obj_t::flag)
							{
								char f = cns::get_flag(arg_list[1]);
								if (f == 'e')
								{
									uint16_t d = 50;
									if (arg_list.size() > 2)
									{
										if (cns::get_obj_type(arg_list[2]) == cns::obj_t::arg)
										{
											d = cns::get_arg<int>(arg_list[2]);
										}
									}
									block->fwrite("echo:\n\tdelay:%ums\n", d);

									app::app_data_t &_app = app::app_data_t::get_instanse();
									while (!block->is_read_out_signal())
									{
										{
											ufo::lock_guard<ufo::mutex_t> lock(_app._imu._lock);
											block->fwrite(">r:%.3f\n>p:%.3f\n>y:%.3f\n\n", 
												_app._imu._r, 
												_app._imu._p, 
												_app._imu._y
											);
										}
										utl::sleep_for(d);
									}
									block->write("echo stop\n");
								}
							}
						}
					}
				});

			cns.mk_blank(
				"bar",
				"",
				[](cns::console_t::block_t block)
				{
					block->write("imu was called\n");

					vector_t<string_t> &arg_list = block->get_buf();

					if (!arg_list.empty())
					{
						if (arg_list.size() > 1)
						{
							if (cns::get_obj_type(arg_list[1]) == cns::obj_t::flag)
							{
								char f = cns::get_flag(arg_list[1]);
								if (f == 'e')
								{
									uint16_t d = 50;
									if (arg_list.size() > 2)
									{
										if (cns::get_obj_type(arg_list[2]) == cns::obj_t::arg)
										{
											d = cns::get_arg<int>(arg_list[2]);
										}
									}
									block->fwrite("echo:\n\tdelay:%ums\n", d);

									app::app_data_t &_app = app::app_data_t::get_instanse();
									while (!block->is_read_out_signal())
									{
										{
											ufo::lock_guard<ufo::mutex_t> lock(_app._baro._lock);
											block->fwrite(">Pr:%.3f\n>Te:%.3f\n\n", _app._baro._p, _app._baro._t);
										}
										utl::sleep_for(d);
									}
									block->write("echo stop\n");
								}
							}
						}
					}
				});

			cns.mk_blank(
				"mmot",
				"",
				[](cns::console_t::block_t block)
				{
					block->write("mot was called\n");

					vector_t<string_t> &arg_list = block->get_buf();

					if (!arg_list.empty())
					{
						if (arg_list.size() > 1)
						{
							if (cns::get_obj_type(arg_list[1]) == cns::obj_t::flag)
							{
								char f = cns::get_flag(arg_list[1]);

								app::app_data_t &appd = app::app_data_t::get_instanse();

								app::types::mot_cmd_t mcmd = app::types::mot_cmd_t::mot_no;

								{
									lock_guard<mutex_t> _l(appd._remote._lock);
									mcmd = appd._remote._mcmd;
								}

								if (f == 'w')
								{
									// if (mcmd != app::types::mot_cmd_t::mot_no)
									// {
									// 	uint32_t t = utl::get_time_millis();
									// 	while (utl::get_time_millis() - t > 2000)
									// 	{
									// 		utl::sleep_for(10);
									// 		if (mcmd == app::types::mot_cmd_t::mot_no)
									// 		{
									// 			break;
									// 		}
									// 	}
									if (mcmd != app::types::mot_cmd_t::mot_no)
									{
										block->write("prev data was unhandled\n");
										return;
									}
									// }
									uint16_t sz = arg_list.size();
									block->fwrite("sz: %u\n", sz);
									if (sz > 3)
									{
										char m = '\0';
										float val = 0;

										// for (size_t i = 1; i < arg_list.size(); i++)
										// {
										// 	block->fwrite("%s\t", arg_list[i].c_str());
										// }
										// block->write("\n");

										if (cns::get_obj_type(arg_list[2]) == cns::obj_t::arg)
										{
											if (arg_list[2].size() > 0)
											{
												m = arg_list[2][0];
												block->fwrite("%c\n", m);
											}
											else
											{
												ufo::cns::log_incorrect_arg();
												return;
											}
										}

										if (cns::get_obj_type(arg_list[3]) == cns::obj_t::arg)
										{
											val = cns::get_arg<float>(arg_list[3]);
											if (val > 0.2f)
											{
												block->write("val will set to 20% (y/) for continue\n");

												// while (true)
												// {
												// 	string_t sss = block->read();
												// 	if (sss.size())
												// 	{
												// 		if (sss[0] == 'y')
												// 		{
												// 			break;
												// 		}
												// 		return;
												// 	}
												// 	utl::sleep_for(50);
												// }
											}
										}
										else
										{
											cns::log_incorrect_arg();
											return;
										}

										val = utl::constrain(static_cast<float>(val), 0.f, 0.3f);

										lock_guard<mutex_t> _l(appd._remote._lock);
										switch (m)
										{
										case 'q':
											appd._remote._roll = val;
											block->fwrite("set-m1 %.3f\n", val);
											appd._remote._mcmd = app::types::mot_cmd_t::mot_valscns;
											return;
										case 'w':
											appd._remote._pitch = val;
											block->fwrite("set-m2 %.3f\n", val);
											appd._remote._mcmd = app::types::mot_cmd_t::mot_valscns;
											return;
										default:
											// clear
											appd._remote._throt = 0.f;
											appd._remote._roll = 0.f;
											appd._remote._pitch = 0.f;
											appd._remote._yaw = 0.f;
											block->write("reset-all\n");
											break;
										}
										appd._remote._mcmd = app::types::mot_cmd_t::mot_vals;
										return;
									}
									cns::log_incorrect_arg();
									return;
								}
								else if (f == 'e')
								{
									while (!block->is_read_out_signal())
									{
										block->fwrite(">r:%.3f, l:%.3f\n",
											appd._rover._rr,
											appd._rover._ll);
										utl::sleep_for(50);
									}
									block->write("echo out\n");
								}

							}
						}
					}
				});
		}

    };

} // namespace ufo


