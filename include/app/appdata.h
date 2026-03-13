#pragma once

#include "apptypes.h"

namespace app
{
        class app_data_t
        {
        public:
            types::control_sig_rover_t _control_sig = {};
            types::bar_data_t _baro = {};
            types::imu_data_t _imu = {};

            types::rover_mot_t _rover = {};

            types::event_t _event = {};
            types::app_cmd_queue_t _queue = {};

        private:
            app_data_t(){}

        public:
            static app_data_t &get_instanse()
            {
                static app_data_t i;
                return i;
            }

            app_data_t(app_data_t &) = delete;
            app_data_t &operator=(app_data_t &) = delete;

            app_data_t &operator=(app_data_t &&) = default;
            app_data_t(app_data_t &&) = default;
        };

        static app_data_t& __global_app_data = app_data_t::get_instanse();
        
} // namespace app
