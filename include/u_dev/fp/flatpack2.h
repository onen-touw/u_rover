#pragma once

#include "u_drivers/can/candrv.h"
#include "u_sys/trace.h"
#include "u_sys/utils.h"
#include "fpdef.h"
// logout after about 5 sec after login if three are no requests(messages) to fp
// If communication is lost (logout)  the FP reverts back to the default voltage.

// 0x5014004 - normal voltage reached

namespace ufo
{
    namespace dev
    {
        class fp
        {
            class fp_cfg
            {
            public:
                static constexpr uint32_t cmd_login_base = 0x05004800;       // last bit is dev-id
                static constexpr uint32_t cmd_req_alert_base = 0x0500bffc;   // second bit is dev-id
                static constexpr uint32_t cmd_defaultVolt_base = 0x05009C00; // second bit is dev-id
                static constexpr uint32_t cmd_set_vals = 0x05004004;         // second bit is dev-id

                static constexpr uint32_t msg_lreq_strtup_base = 0x0504BFFC; // 0x05XXBFFC
                static constexpr uint32_t msg_alert = 0x0504BFFC;            // 0x05XXBFFC
                static constexpr uint32_t msg_hello = 0x05000000;            // 0x0500XXXX

                static constexpr float ovp = 57.50f; // todo
            };

        public:
            enum class fp_status : uint8_t
            {
                ConstantVoltage_Normal = 0x04,
                ConstantCurrent_Normal = 0x08,
                Alarm = 0x0C,
                RampUp = 0x10,
            };

            enum class fp_dev_status : uint8_t
            {
                idle,
                process,
                error,
                power_off,
            };
            enum class fp_flag1 : uint8_t
            {
                OVS_LockOut,        // ??
                Mod_Fail_Primary,   // ??
                Mod_Fail_Secondary, // ??
                High_Mains,         // ?
                Low_Mains,          // ?
                HighTemp,
                LowTemp, // ?
                CurrentLimit
            };
            enum class fp_flag2 : uint8_t
            {
                Internal_Voltage,
                ModuleFail,
                Mod_Fail_Secondary,
                Fan1_SpeedLow,
                Fan2_SpeedLow,
                SubMod1_Fail,
                Fan3_SpeedLow,
                Inner_Volt,
            };
            struct fp_alert
            {
                fp_alert_req _alert = fp_alert_req::warning;
                fp_flag1 _f1 = fp_flag1::OVS_LockOut;
                fp_flag2 _f2 = fp_flag2::Internal_Voltage;
            };

        private:
            using msg_t = ufo::drv::can_msg_t;
            using candrv_t = drv::can_drv_t; 
        private:
            fp_vals _vals;
            fp_alert _alert;
            fp_status _status = fp_status::ConstantVoltage_Normal;
            uint8_t _devID = 0;
            fp_dev_status _devStat = fp_dev_status::idle;
            uint8_t _serno[6] = {};

            candrv_t* _drv = nullptr;

        public:

            fp(candrv_t* candrv)
            {
                if (!candrv)
                {
                    // CEr
                    return;
                }
                if (!candrv->check_state())
                {
                    // CEr
                    return;
                }
                _drv = candrv;
                _devStat = fp_dev_status::idle;
            }

            // s_i is serial number i-byte
            void init(uint8_t s0, uint8_t s1,uint8_t s2,uint8_t s3,uint8_t s4,uint8_t s5, uint8_t id){
                if (id < 1 || id > 16)
                {
                    //CEr
                    _devID = 0;
                    return;
                }
                _serno[0] = s0;
                _serno[1] = s1;
                _serno[2] = s2;
                _serno[3] = s3;
                _serno[4] = s4;
                _serno[5] = s5;
                _devID = id;

                _devStat = fp_dev_status::process;
                cmdLogin();
            }

            void init(const uint8_t *serno, uint8_t id){
                if (id < 1 || id > 16)
                {
                    //CEr
                    _devID = 0;
                    return;
                }
                if (sizeof(serno) != 6)
                {
                    //CEr
                    return;
                }
                _devID = id;

                memcpy(_serno, serno, 6);
           
                _devStat = fp_dev_status::process;
                cmdLogin();
            }
            ~fp() {}
            
            void cmdLogin()
            {
                // if (_devStat == fp_dev_status::error && _devStat == fp_dev_status::power_off)
                // {
                //     return;
                // }
                Trace_t::log("login\n");
                msg_t msg;
                msg.identifier = fp_cfg::cmd_login_base | (_devID*4);
                msg.extd = 1;
                msg.data_length_code = 8;
                // memcpy(msg.data, _serno, 6);
                for (size_t i = 0; i < 6; i++)
                {
                    msg.data[i] = _serno[i];
                }
                msg.data[6] = 0x00;
                msg.data[7] = 0x00;
                _drv->write(msg);
                // log_raw(msg);
            }

            void full_msg_log(){
                Trace_t::log("\n======\n");
                Trace_t::log("full-log\n");

                msg_t msg = _drv->read();
                if (msg.data_length_code < 1)
                {
                    Trace_t::log("len=0\n");
                    return;
                }
                uint8_t devid = (msg.identifier >> 16) & 255;
                uint8_t sec_bit = (msg.identifier >> 8) & 255;
                printf("can id: 0x%lx\n", msg.identifier);
                printf("dev-id: 0x%x, dec:%u\n", devid, devid);
                printf("sec-bit: 0x%x\n", sec_bit);
                log_raw(msg);
            }

        
            // if external class chose this that msg.id==this->devID
            void update()
            {
                Trace_t::log("update::\n\t");

                if (_devStat != fp_dev_status::process)
                {
                    Trace_t::log("!handle\n");
                    return;
                }

                msg_t msg = _drv->read();
                if (msg.data_length_code < 1)
                {
                    return;
                }
                
                uint8_t devid = (msg.identifier >> 16) & 255;
                uint8_t sec_bit = (msg.identifier >> 8) & 255;
                printf("can resp id: 0x%lx\n", msg.identifier);
                printf("dev-id: 0x%x, dec: %u\n", devid, devid);
                printf("sec-bit: 0x%x\n", sec_bit);
                log_raw(msg);

                        cmdSet(20.f,48.f);
                // this is not neccessary
                if (devid != 0)
                {
                    if (sec_bit == 0x40)
                    {
                        Trace_t::log("status\n");
                        handleStatus(msg);
                    }
                    else if (sec_bit == 0x44)
                    {
                        Trace_t::log("start notif / login req\n"); 
                        cmdLogin(); // depend on which behavoir we want
        			    // cmdSet(10.f,45.f);

                    }
                    else if (sec_bit == 0xBF)
                    {
                        Trace_t::log("alert\n");
                        handleAlert(msg);
                    }
                    return;
                }
                if (msg.data[0] == 0x1b)
                {
                    Trace_t::log("hello pack\n");
                }
                else {
                    Trace_t::log("unknown\n");
                }
            }
            void cmdSet(float maxCurr, float destVolt, float ovp = fp_cfg::ovp)
            {
                msg_t msg;
                msg.identifier = fp_cfg::cmd_set_vals | (0xff << 16);
                Trace_t::log("cmd-limit: ");
                printf("0x%lx\n", msg.identifier);

                uint16_t amp = maxCurr * 10;
                uint16_t volt = destVolt * 100;
                uint16_t vovp = ovp * 100;
                msg.extd = 1;
                msg.data_length_code = 8;
                msg.data[0] = amp & 255;
                msg.data[1] = (amp >> 8) & 255;
                msg.data[2] = volt & 255;
                msg.data[3] = (volt >> 8) & 255;
                msg.data[4] = msg.data[2];
                msg.data[5] = msg.data[3];
                msg.data[6] = vovp & 255;
                msg.data[7] = (vovp >> 8) & 255;

                _drv->write(msg);
            }

            void cmdDefaultVolt(float def)
            {
                msg_t msg;
                msg.identifier = fp_cfg::cmd_defaultVolt_base | (_devID << 16);

                uint16_t volt = def * 100;
                msg.extd = 1;
                msg.data_length_code = 5;
                msg.data[0] = 0x29;
                msg.data[1] = 0x15;
                msg.data[2] = 0x00;
                msg.data[3] = volt & 255;
                msg.data[4] = (volt >> 8) & 255;

                _drv->write(msg);
            }

            void cmdRequestAlert(fp_alert_req alert)
            {
                msg_t msg;

                msg.extd = 1;
                msg.identifier = fp_cfg::cmd_req_alert_base | (_devID << 16);
                msg.data[0] = 0x08;
                msg.data[1] = static_cast<uint8_t>(alert);
                msg.data[2] = 0x00;
                msg.data_length_code = 3;

                _drv->write(msg);
            }
        private:
            void log_raw(const msg_t& msg) const {
                Trace_t::log("\traw:{");
                if (msg.data_length_code < 1)
                {
                    Trace_t::log("len = 0}\n");
                    return;
                }
                
                for (size_t i = 0; i < msg.data_length_code; i++)
                {
                    printf("0x%x, ",msg.data[i]);
                }
                Trace_t::log("}\n");
            }

            void handleStatus(msg_t &msg)
            {
                _status = static_cast<fp_status>(msg.identifier & 255); // ??

                _vals._t1 = msg.data[0];
                _vals._cur = 0.1f * (msg.data[1] | (msg.data[2] << 8));
                _vals._vout = 0.01f * (msg.data[3] | (msg.data[4] << 8));
                _vals._vin = msg.data[5] | (msg.data[6] << 8);
                _vals._t2 = msg.data[7];
                printf("vals: [ %u : %f : %f : %u : %u]\n", _vals._t1, _vals._cur, _vals._vout, _vals._vin, _vals._t2);
//Trace_t::log

                if (_status == fp_status::Alarm)
                {
                    // cmdRequestAlert(fp_alert_req::alarm);
                }
            }

            void handleAlert(msg_t &msg)
            {
                if (msg.data[0] != 0x0E)
                {
                    Trace_t::log("not alert\n");
                }
                _alert._alert = static_cast<fp_alert_req>(msg.data[1]);
                _alert._f1 = static_cast<fp_flag1>(msg.data[3]);
                _alert._f2 = static_cast<fp_flag2>(msg.data[4]);
            }
        };

    } // namespace dev

} // namespace ufo
