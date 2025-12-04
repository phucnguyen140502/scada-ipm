
#include <Arduino.h>
#include "WiFi.h"
#include "Modbus.h"

// Set to true to use simulated values, false to use real power meter
#define SIMULATE_POWER_METER true

class Power_meter
{

public:

unsigned long timer;
uint8_t mun_erro;
double simulated_total_energy = 0.0;

  // Add slight random fluctuation to a value (±percentage)
  double fluctuate(double base_value, double percent = 5.0) {
    double range = base_value * (percent / 100.0);
    double fluctuation = (random(-1000, 1001) / 1000.0) * range;
    return base_value + fluctuation;
  }

  void simulate_telemetry()
  {
    // Simulate telemetry based on toggle state
    if (JsonData["toggle"] == 1) {
      // Device is ON - simulate power consumption with fluctuations
      double voltage = fluctuate(5.0, 3.0);        // 5V ±3%
      double current = fluctuate(0.6, 5.0);        // 0.6A ±5%
      double power = voltage * current;             // Calculate actual power
      double pf = fluctuate(0.95, 2.0);            // 0.95 ±2%
      double freq = fluctuate(50.0, 0.5);          // 50Hz ±0.5%
      
      JsonData["voltage"] = voltage;
      JsonData["current"] = current;
      JsonData["power"] = power;
      JsonData["power_factor"] = pf;
      JsonData["frequency"] = freq;
      
      // Accumulate total energy based on actual power (per 10 seconds)
      simulated_total_energy += (power / 1000.0) / 360.0;  // kWh
      JsonData["total_energy"] = 300;
    } else {
      // Device is OFF - no power consumption
      JsonData["voltage"] = 0.0;
      JsonData["current"] = 0.0;
      JsonData["power"] = 0.0;
      JsonData["power_factor"] = 0.0;
      JsonData["frequency"] = 0.0;
      // Keep total_energy unchanged when off
      JsonData["total_energy"] = 300;
    }
  }

  void read(uint8_t key=0)
  {
    if ((millis() < timer)&(!key)) return;
    timer = millis() + 10000;  // Update every 10 seconds

#if SIMULATE_POWER_METER
    simulate_telemetry();
    return;
#endif

    modbus.setTimeout(300);
    if (modbus.requestFrom(0x01, 0x04, 0x00, 60) > 0)
    {
      double voltage = modbus.uint16(0) / 10.0;
      if (voltage > 0)
      {
        JsonData["total_energy"] = modbus.uint32(29) / 100.0;
        JsonData["total_energy_reverse"] = modbus.uint32(39) / 100.0;
        JsonData["total_energy_forward"] = modbus.uint32(49) / 100.0;
        JsonData["voltage"] = voltage;
        JsonData["current"] = modbus.int16(3) / 100.0;
        JsonData["power"] = modbus.int16(8) / 1.0;
        JsonData["power_factor"] = modbus.int16(20) / 1000.0;
        JsonData["frequency"] = modbus.int16(26) / 100.0;
      }
      //    SERIAL.printf("tong:      %4.2f kWh\r\n", meter.total);
      //    SERIAL.printf("tieu thu:  %4.2f kWh\r\n", meter.forward);
      //    SERIAL.printf("tai len:   %4.2f kWh\r\n", meter.reverse);
      //    SERIAL.printf("dien ap:   %4.2f V\r\n",   meter.voltage);
      //    SERIAL.printf("dong dien: %4.2f A\r\n",   meter.current);
      //    SERIAL.printf("cong suat: %4.2f W\r\n",   meter.wattage);
      //    SERIAL.printf("cos phi:   %4.2f \r\n",    meter.Pfactor);
      //    SERIAL.printf("tan so:    %4.2f Hz\r\n",  meter.frequency);
      //    SERIAL.println();
      mun_erro = 0;
    }
    else if (mun_erro > 100)
    {
      JsonData["voltage"] = 0;
      JsonData["current"] = 0;
      JsonData["power"] = 0;
      JsonData["power_factor"] = 0;
      JsonData["frequency"] = 0;
      cmd.println("erro reading");
    }
    else
    {
      mun_erro++;
    }
  }

  void begin()
  {
    modbus.init();
  }

  void loop()
  {
    read();
  }
};
Power_meter power_meter;
