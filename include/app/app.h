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
#include "u_dev/lora/UFO_Lora.h"
#include "u_dev/bazz.h"
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
			sys_data_t& msys = sys_data_t::get_instanse();

			// net_t nett;
			// nett.mk_sock(
			// 	net_descriptors_t::sock_main,
			// 	"192.168.0.64",
			// 	net::uSocketType_t::client,
			// 	[](net::fast_sock::rcv_t *rcv)
			// 	{
			// 		// Trace_t::flog("rcv[%u] (%u): %s\n", ufo::utl::get_time_millis(), rcv->_len, rcv->_payload);
			// 		crt::decrypte_t::unpack(
			// 			reinterpret_cast<uint8_t *>(rcv->_payload),
			// 			rcv->_len,
			// 			[](cmd_t cmd, uint8_t *buf)
			// 			{
			// 				app_data_t &appd = app_data_t::get_instanse();
			// 				if (cmd == cmd_t::remote_trpy)
			// 				{
			// 					ufo::lock_guard<mutex_t> _l(appd._remote._lock);
			// 					appd._remote._throt = crt::get_arg<float>(0, buf);
			// 					appd._remote._roll = crt::get_arg<float>(1, buf);
			// 					appd._remote._pitch = crt::get_arg<float>(2, buf);
			// 					appd._remote._yaw = crt::get_arg<float>(3, buf);
			// 					appd._remote._mcmd = app::types::mot_cmd_t::mot_vals;
			// 				}
			// 				else if (cmd == cmd_t::remote_arm)
			// 				{
			// 					ufo::lock_guard<mutex_t> _l(appd._remote._lock);
			// 					if (crt::get_arg<int32_t>(0, buf) > 0)
			// 					{
			// 						appd._remote._mcmd = app::types::mot_cmd_t::mot_set_arm;
			// 					}
			// 					else
			// 					{
			// 						appd._remote._mcmd = app::types::mot_cmd_t::mot_set_disarm;
			// 					}
			// 				}
			// 			});
			// 	});
			// net_t::msg_block_t msg_block = nett.get_block(net_descriptors_t::sock_main);
			// ufo::thread_cfg cfg_net;
			// cfg_net._name = "net";
			// cfg_net._core = 0;
			// cfg_net._prio = 5;
			// cfg_net._stackSize = 4096;
			// ufo::thread_guard task_net(ufo::thread(cfg_net, &net_t::task, &nett));

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

			// lora llora(msys._drv._uart1.get());
			// UFO_LoraSettings conf = {};
			// conf._selfAddr._addh = 0;
			// conf._selfAddr._addl = 2;
			// conf._selfAddr._chan = 8;
			
			// conf._targAddr._addh = UFO_LORA_BROADCAST;
			// conf._targAddr._addl = UFO_LORA_BROADCAST;
			// conf._targAddr._chan = 10;
			// conf.adrt =  LORA_AIR_DATA_RATE_110_384;
			// llora.SetConfig(conf, [](lora::rcv_t* cll){
			// 	printf("rcv on RC from ROVER: %s\n", cll->_payload);
			// });
			// llora.Setup();
			// lora::msg_block_t lora_msg = llora.get_block();

			// ufo::thread_cfg cfg_lora;
			// cfg_lora._name = "lora";
			// cfg_lora._core = 1;
			// cfg_lora._prio = 5;
			// cfg_lora._stackSize = 4096;
			// ufo::thread_guard task_lora(ufo::thread(cfg_lora, [](lora* lr, token_t token){
			// 	while (token)
			// 	{
			// 		lr->Iteration();
			// 	}
			// }, &llora));

			nettt_t nettt;
			nettt_t::desc_t sock = 0;
			// nettt_t::desc_t lrr = 0;
			nettt_t::msg_block_t sock_msg = nettt.mk(
				sock,	
				std::make_unique<nettt_t::sock_t>(
					"192.168.0.64", 
					nettt_t::sock_t::sockt_t::server, 
					net_callback)
				);

			bazz_t bz;
				
			mpwm_t motors;
			crt::encrypte_t<remote_cmd_e> encripter;
			app::types::event_t event = {};

			event._app.set(app_event_e::idle);
			msys._cns.unlock();

			using qcmd_t = types::app_cmd_queue_t::cmd_t; 
			qcmd_t cmd = qcmd_t::null;

			while (token)
            {
				if (encripter.size())
				{
					sock_msg->Msg(encripter.get(), encripter.size());
					encripter.reset();
				}
				// lora_msg->Msg(3, "cntRV:228\n",11);
				// sock_msg->fMsg("hello %lu", ufo::utl::get_time_millis());
				if (event._app == app_event_e::control)
				{		
					{
						ufo::lock_guard<mutex_t> _l(appd._control_sig._lock);

						motors.update(
							utl::map(static_cast<float>(appd._control_sig._pitch), -1024.f, 1024.f, -1.f, 1.f),
							utl::map(static_cast<float>(appd._control_sig._roll), -1024.f, 1024.f, -1.f, 1.f)
						);
						appd._rover._rr = motors.get_mot_throt_r();
						appd._rover._ll = motors.get_mot_throt_l();

						appd._rover._rpwm = motors.get_lpwm();
						appd._rover._lpwm = motors.get_rpwm();
					}

					ufo::utl::sleep_for(10);


					if (xQueueReceive(appd._queue._q, &cmd, 10))
					{
						// printf( "\n");

						if (cmd == qcmd_t::disarm)
						{
							printf("rv::control::disarm\n");
							motors.update(0.f, 0.f);

							// msys._cns.unlock();
							
							// encripter.pack(remote_cmd_e::arm, uint8_t(0u));

							// after checks
							event._app.set(app_event_e::idle);
						}
						else if (cmd == qcmd_t::req)
						{
							// printf( "rc::control::req_ask\n");
							encripter.pack(remote_cmd_e::ask, utl::get_time_millis());
						}
						else {
							printf("rv::control::~\n");
						}
						continue;	// ?? if we handle trpy outside this scope (like it did now)
					}
					// printf( "\rrc::control::");

					// check connection (sometimes)
					utl::sleep_for(20);
				}
				else if (event._app == app_event_e::idle)
				{
					if (xQueueReceive(appd._queue._q, &cmd, 10))
					{
						// printf("\n");

						if (cmd == qcmd_t::arm)					// set from btns
						{
							printf( "rv::idle::arm\n");

							if (!msys._cns.get_state().get(ufo::types::cns_t::cns_state_t::started))
							{
								// todo!!! check in block and if !cns.block() do warning!
								// msys._cns.block();

								// todo:: send req (confirm-arm and time) -> wait ans -> good ? set(control) : send -> ! (bad) 
								event._app.set(app_event_e::control);
							}
						}
						else if (cmd == qcmd_t::find_on)			// set from btns
						{
							// printf( "rc::idle::find_mode\n");

							event._app.set(app_event_e::alarm);
							event._alarm.set(app_event_alarm_e::find_mode);
						}
						else if (cmd == qcmd_t::req)
						{
							// printf("rc::idle::req_ask\n");
							encripter.pack(remote_cmd_e::ask, utl::get_time_millis());
						}
						else {
							printf("rv::no such cmd\n");

							event._app.set(app_event_e::alarm);
							event._alarm.set(app_event_alarm_e::warning);							
						}
						continue;		
					}
					// printf( "\rrc::idle::");
					utl::sleep_for(10);
					// check connection
				}
				else if (event._app == app_event_e::alarm)
				{
					if (event._alarm.get() == app_event_alarm_e::disconn)
					{
						printf("rv::alarm::disconn\n");
						for (size_t i = 0; i < 3; i++)
						{
							bz.update(524);
							utl::sleep_for(25);
							bz.update(0);
							utl::sleep_for(15);
						}
						event._app.back();
					}
					else if (event._alarm.get() == app_event_alarm_e::find_mode)
					{
						printf("rv::alarm::find-find::\n\twait tubm-find-off\n");

						while (true)
						{
							printf(".");
							for (size_t i = 0; i < 2; i++)
							{
								bz.update(0);
								utl::sleep_for(25);
								bz.update(524);
								utl::sleep_for(15);
							}
							bz.update(0);

							if (xQueueReceive(appd._queue._q, &cmd, 50))
							{
								if (cmd == qcmd_t::find_off)
								{
									printf("\n");
									bz.update(0);
									utl::sleep_for(15);
									bz.update(0);
									utl::sleep_for(15);

									event._app.back();
									break;
								}
							}
						}
					}
					else if (event._alarm.get() == app_event_alarm_e::warning){
						printf("rv::alarm::warning\n");
						// bp
						// bip
						bz.update(524);
						utl::sleep_for(25);
						bz.update(0);
						
						event._app.back();
					}
					else if (event._alarm.get() == app_event_alarm_e::battery)
					{
						printf("rv::alarm::bat low\n");
						// bip
						bz.update(524);
						utl::sleep_for(50);
						bz.update(0);
						
						event._app.back();
					}
					else if (event._alarm.get() == app_event_alarm_e::battery_crit)
					{
						//bip-bibiiiiip
						bz.update(524);
						utl::sleep_for(500);
						bz.update(0);
						
						printf("rv::alarm::bat low\n\tpower-off\n");
						break;
					}
					else
					{
						// crit 
						for (uint16_t i = 0; i < 10; i++)
						{
							// bip-bip-bibibip
							printf("rv::alarm::critical\n");
							//bip-bibiiiiip
							bz.update(524);
							utl::sleep_for(500);
							bz.update(0);
							ufo::utl::sleep_for(500);
						}
						break;
					}
				}
				else
				{
					printf("rv::app::null\n");
					event._app.set(app_event_e::alarm);
					event._alarm.set(app_event_alarm_e::critical);
				}
				ufo::utl::sleep_for(1);
            }
        }

		static void net_callback(ufo::net::fsk_base::rcv_t *rcv){
			// ufo::Trace_t::flog("rcv[%u] (%u): %s\n", ufo::utl::get_time_millis(), rcv->_len, rcv->_payload);
			using namespace ufo;

			crt::decrypte_t<remote_cmd_e>::unpack(
						reinterpret_cast<uint8_t*>(rcv->_payload),
						rcv->_len,
						[](remote_cmd_e cmd, uint8_t *buf)
						{
							using qcmd_t = types::app_cmd_queue_t::cmd_t; 
							qcmd_t qcmd = qcmd_t::null;
							app_data_t &appd = app_data_t::get_instanse();

							if (cmd == remote_cmd_e::trpy)
							{
								ufo::lock_guard<mutex_t> _l(appd._control_sig._lock);
								appd._control_sig._throt = crt::gget_arg<crt::arg_1, uint16_t>(buf);
								appd._control_sig._roll = crt::gget_arg<crt::arg_2, uint16_t>(buf);
								appd._control_sig._pitch = crt::gget_arg<crt::arg_3, uint16_t>(buf);
								appd._control_sig._yaw = crt::gget_arg<crt::arg_4, uint16_t>(buf);
							}
							else if (cmd == remote_cmd_e::arm)
							{
								if (crt::gget_arg<crt::arg_1, int8_t>(buf) > 0)
								{
									qcmd = qcmd_t::arm;
								}
								else
								{
									qcmd = qcmd_t::disarm;
								}
								xQueueSend(appd._queue._q, &qcmd, 10);
							}
							else if (cmd == remote_cmd_e::find_mode)
							{
								if (crt::gget_arg<crt::arg_1, uint8_t>(buf) > 0)
								{
									qcmd = qcmd_t::find_on;
								}
								else
								{
									qcmd = qcmd_t::find_off;
								}
								xQueueSend(appd._queue._q, &qcmd, 10);
							}
						});
			
		}

		void cns_init(ufo::cns::console_t & cns){
			using namespace ufo;

			cns.mk_blank(
				"rmt",
				"",
				[](cns::console_t::block_t block)
				{
					// block->write("rmt was called\n");
					vector_t<string_t> &arg_list = block->get_buf();
					if (!arg_list.empty())
					{
						if (arg_list.size() > 1)
						{
							cns::opt_t opt(arg_list[1]);
							if (opt == 'e' || opt == "echo")
							{

								uint16_t d = 50;
								if (opt.arg_count() == 1)
								{
									d = opt.get_arg<uint16_t>(0);
									if (!d)
									{
										d = 50;
									}
									block->fwrite("change freq to %ums\n", d);
								}

								app::app_data_t &_app = app::app_data_t::get_instanse();
								while (!block->is_read_out_signal())
								{
									{
										ufo::lock_guard<ufo::mutex_t> lock(_app._control_sig._lock);
										// block->fwrite(">t:%.3f\n>r:%.3f\n>p:%.3f\n>y:%.3f\n\n",
											block->fwrite(">t:%d\n>r:%d\n>p:%d\n>y:%d\n\n",
														  _app._control_sig._throt,
														  _app._control_sig._roll,
														  _app._control_sig._pitch,
														  _app._control_sig._yaw);
									}
									utl::sleep_for(d);
								}
								block->write("echo stop\n");
								return;
							}
						}
					}
					block->log_incorrect_arg();
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
							cns::opt_t opt(arg_list[1]);
							if (opt == 'e' || opt == "echo")
							{
								uint16_t d = 50;
								if (opt.arg_count() == 1)
								{
									d = opt.get_arg<uint16_t>(0);
									if (!d)
									{
										d = 50;
									}
									block->fwrite("change freq to %ums\n", d);
								}

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
				});

			cns.mk_blank(
				"bar",
				"",
				[](cns::console_t::block_t block)
				{
					// block->write("imu was called\n");
					vector_t<string_t> &arg_list = block->get_buf();
					if (!arg_list.empty())
					{
						if (arg_list.size() > 1)
						{
							cns::opt_t opt(arg_list[1]);
							if (opt == 'e' || opt == "echo")
							{
								uint16_t d = 50;
								if (opt.arg_count() == 1)
								{
									d = opt.get_arg<uint16_t>(0);
									if (!d)
									{
										d = 50;
									}
									block->fwrite("change freq to %ums\n", d);
								}

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
				});

			cns.mk_blank(
				"mmot",
				"",
				[](cns::console_t::block_t block)
				{
					vector_t<string_t> &arg_list = block->get_buf();
					if (!arg_list.empty())
					{
						if (arg_list.size() > 1)
						{
							cns::opt_t opt(arg_list[1]);
							if (opt == 'p' || opt == "param")
							{
								app::app_data_t &appd = app::app_data_t::get_instanse();
								// app::types::mot_cmd_t mcmd = app::types::mot_cmd_t::mot_no;
								// {	// get current command
								// 	lock_guard<mutex_t> _l(appd._remote._lock);
								// 	mcmd = appd._remote._mcmd;
								// }
								// if (mcmd != app::types::mot_cmd_t::mot_no)
								// {
								// 	block->write("prev data was unhandled\n");
								// 	return;
								// }
								if (arg_list.size() > 2)
								{
									cns::opt_t mot_opt(arg_list[2]);
									char m = '\0'; // get char: q/w/s/a or r/p/y
									float val = 0.f;
									if (mot_opt == 'q' || mot_opt == "left")
									{
										m = 'q';
									}
									else if (mot_opt == 'w' || mot_opt == "right")
									{
										m = 'w';
									}
									else if (mot_opt == 'a' || mot_opt == "all")
									{
										m ='a';
									}
									else if (mot_opt == 'r' || mot_opt == "rst")
									{
										m ='r';
									}
									else
									{
										block->log_incorrect_arg();
										return;
									}
									if (mot_opt.arg_count() != 1 )
									{
										if (m != 'r' && mot_opt.arg_count() != 0)
										{
											block->log_incorrect_arg();
											return;
										}
									}
									val = mot_opt.get_arg<float>(0);
									if (val < 0.f)
									{
										block->log_incorrect_arg();
										block->write("val<0.f!\n");
										return;
									}
									else if (val > 0.2f)
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
									val = utl::constrain(static_cast<float>(val), 0.f, 0.3f);
									lock_guard<mutex_t> _l(appd._control_sig._lock);
									switch (m)
									{
									case 'q':
									block->fwrite("set-m1 %.3f\n", val);
										appd._control_sig._roll = val;
										appd._control_sig._pitch = 0.f;
										break;
									case 'w':
									block->fwrite("set-m2 %.3f\n", val);
										appd._control_sig._pitch = val;
										appd._control_sig._roll = 0.f;
										break;
									case 'a':
									block->fwrite("set-all %.3f\n", val);
										appd._control_sig._pitch = val;
										appd._control_sig._roll = val;
										break;
									default:
										// clear
										block->write("reset-all\n");
										appd._control_sig._throt = 0.f;
										appd._control_sig._roll = 0.f;
										appd._control_sig._pitch = 0.f;
										appd._control_sig._yaw = 0.f;
										break;
									}
									return;
								}
							}
							else if (opt == 'e' || opt == "echo")
							{
								uint16_t d = 50;
								if (opt.arg_count() == 1)
								{
									d = opt.get_arg<uint16_t>(0);
									if (!d)
									{
										d = 50;
									}
									block->fwrite("change freq to %ums\n", d);
								}
								app::app_data_t &_app = app::app_data_t::get_instanse();
								while (!block->is_read_out_signal())
								{
									block->fwrite(">L:%.3f\t>Lp:%d\n>R:%.3f\t>Lr:%d\n\n", 
										_app._rover._ll, 
										_app._rover._lpwm, 
										_app._rover._rr, 
										_app._rover._rpwm);
									utl::sleep_for(d);
								}
								block->write("echo stop\n");
								return;
							}
							else if (opt == 'h' || opt == "help")
							{
								block->write("-p/--param ");
								block->write("-q/--left=0.1 or -w/--right=0.1 ");
								block->write("will set 10'%' of thror to left or right motor resp\n");
								return;
							}	
						}
					}
					block->log_incorrect_arg();
				});
		}
	};

} // namespace ufo


