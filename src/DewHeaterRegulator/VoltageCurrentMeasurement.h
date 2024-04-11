#pragma once

#include <Arduino.h>
#include <string>

#include "ADS1X15.h"

class VoltageCurrentMeasurement {
    public:
        VoltageCurrentMeasurement() = delete;

        VoltageCurrentMeasurement(ADS1115 *ads, uint8_t voltage_pin, uint8_t current_pin, float voltage_const, float current_const)   {
            m_voltage_pin = voltage_pin;
            m_current_pin = current_pin;
            m_voltage_const = voltage_const;
            m_current_const = current_const;
            m_ads1115 = ads;
        }

        void set_noise_level(float voltage_noise, float current_noise) {
            m_voltage_noise = voltage_noise;
            m_current_noise = current_noise;
        }

        void set_time_interval(int time_interval_min, int time_interval_max) {
            m_time_interval_min = time_interval_min;
            m_time_interval_max = time_interval_max;
        }

        void set_average_time(int average_time) {
            m_average_time = average_time;
        }

        void measure()  {
            const int32_t time_now = millis();
            if (time_now >= m_next_time_average && m_n_samples > 0) {
                m_average_voltage = m_voltage_sum/m_n_samples;
                m_average_current = m_current_sum/m_n_samples;
                m_total_charge += m_average_current*(time_now - m_time_last_average)/3600000.0;

                if (abs(m_average_voltage) < m_voltage_noise) {
                    m_average_voltage = 0;
                }
                if (abs(m_average_current) < m_current_noise) {
                    m_average_current = 0;
                }

                m_voltage_sum = 0;
                m_current_sum = 0;
                m_n_samples = 0;

                m_time_last_average = time_now;
                m_next_time_average = time_now + m_average_time;

            }
            if (time_now >= m_next_time_measurement) {
                const float voltage = m_voltage_const*m_ads1115->readADC(0);
                const float current = m_current_const*m_ads1115->readADC(1);

                m_voltage_sum += voltage;
                m_current_sum += current;
                m_n_samples++;

                m_next_time_measurement = time_now + m_time_interval_min + random()*(m_time_interval_max - m_time_interval_min);
            }
        }

        float get_voltage() const {
            return m_average_voltage;
        }

        float get_current() const {
            return m_average_current;
        }

        double get_total_charge() const {
            return m_total_charge;
        }


        static std::string get_message(float value, const std::string &unit) {
            if (value >= 0.8) {
                return std::to_string(value).substr(0,5) + " " + unit;
            }
            else  {
                return std::to_string(value*1000).substr(0,5) + " m" + unit;
            }
        }


    private:
        uint8_t m_voltage_pin;
        uint8_t m_current_pin;
        float m_voltage_const;
        float m_current_const;
        ADS1115 *m_ads1115 = nullptr;

        float m_voltage_noise = 0;
        float m_current_noise = 0;

        int32_t m_time_interval_min = 5;
        int32_t m_time_interval_max = 20;
        int32_t m_average_time = 1000;

        double   m_voltage_sum = 0;
        double   m_current_sum = 0;

        float   m_average_voltage = 0;
        float   m_average_current = 0;
        double  m_total_charge = 0;
        int32_t m_time_last_average = 0;
        int32_t m_n_samples = 0;

        int32_t m_next_time_measurement = 0;
        int32_t m_next_time_average     = 0;

        double random()  {
            return (double(rand())+1) / (double(RAND_MAX)+2);
        };

};