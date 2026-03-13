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
#include "model_uart_ctrl.h"

#include "appconsole.h"

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

			bazz_t bz;
			mpwm_t motors;
			crt::encrypte_t<remote_cmd_e> encripter;
			model::model_uart_ctrl_t ctrl(__global_system_data._drv._uart1.get(), net_callback);

			app::types::event_t event = {};
			event._app.set(app_event_e::idle);

			__global_system_data._cns.unlock();

			using qcmd_t = types::app_cmd_queue_t::cmd_t; 
			qcmd_t cmd = qcmd_t::null;

			while (token)
            {
				if (encripter.size())
				{
					ctrl.write(encripter.get(), encripter.size())
					encripter.reset();
				}
				
				if (event._app == app_event_e::control)
				{		
					{
						ufo::lock_guard<mutex_t> _l(__global_app_data._control_sig._lock);

						// motors.update(
						// 	utl::map(static_cast<float>(__global_app_data._control_sig._data._ang_sig._p), -1024.f, 1024.f, -1.f, 1.f),
						// 	utl::map(static_cast<float>(__global_app_data._control_sig._data._ang_sig._r), -1024.f, 1024.f, -1.f, 1.f)
						// );
						
						__global_app_data._rover._data._sig._rr = motors.get_mot_throt_r();
						__global_app_data._rover._data._sig._ll = motors.get_mot_throt_l();

						__global_app_data._rover._data._pwm._rpwm = motors.get_lpwm();
						__global_app_data._rover._data._pwm._lpwm = motors.get_rpwm();
					}

					ufo::utl::sleep_for(10);


					if (xQueueReceive(__global_app_data._queue._q, &cmd, 10))
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
					if (xQueueReceive(__global_app_data._queue._q, &cmd, 10))
					{
						// printf("\n");

						if (cmd == qcmd_t::arm)					// set from btns
						{
							printf( "rv::idle::arm\n");

							if (!__global_system_data._cns.get_state().get(ufo::types::cns_t::cns_state_t::started))
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

							if (xQueueReceive(__global_app_data._queue._q, &cmd, 50))
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

							if (cmd == remote_cmd_e::trpy)
							{
								ufo::lock_guard<mutex_t> _l(__global_app_data._control_sig._lock);
								__global_app_data._control_sig._throt = crt::gget_arg<crt::arg_1, uint16_t>(buf);
								__global_app_data._control_sig._roll = crt::gget_arg<crt::arg_2, uint16_t>(buf);
								__global_app_data._control_sig._pitch = crt::gget_arg<crt::arg_3, uint16_t>(buf);
								__global_app_data._control_sig._yaw = crt::gget_arg<crt::arg_4, uint16_t>(buf);
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
								xQueueSend(__global_app_data._queue._q, &qcmd, 10);
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
								xQueueSend(__global_app_data._queue._q, &qcmd, 10);
							}
						});
		}

		static void lora_test_callback(ufo::net::fsk_base::rcv_t *rcv){
			ufo::Trace_t::flog("rcv[%u] (%u): %s\n", ufo::utl::get_time_millis(), rcv->_len, rcv->_payload);
		}

		void cns_init(ufo::cns::console_t & cns){
			using namespace ufo;
			using namespace app::console;

			cns.mk_blank("echo", "write 'echo --<module>=f' where f(optioinal arg) is frequency", console_echo);
			cns.mk_blank("motors", "two modes of conrol are available: angle and throt", console_motors);
			cns.mk_blank("imu", "imu module control", console_imu);
		}
	};

} // namespace ufo


