#ifdef EMBEDDED
#warning asdf

#include "battery.h"
#include <stdint.h>
#include <Arduino.h>
#include <Wire.h>

uint16_t i2c_read_register16(uint8_t addr, uint8_t reg) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.endTransmission();

  Wire.requestFrom(addr, 2);
  uint16_t out = Wire.read();
  out |= (Wire.read() << 8);

  return out;
}

void i2c_write_register16(uint8_t addr, uint8_t reg, uint16_t val) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.write((uint8_t)((val >> 8) & 0xFF));
  Wire.write((uint8_t)(val & 0xFF));
  Wire.endTransmission();
}


void battery_setup() {
  uint16_t status_reg = i2c_read_register16(0x36, 0x00);

  Serial.printf("0x%x\r\n", status_reg);

  if (status_reg & 0x02) { // power on reset (POR) bit is set
    // do setup
    
    uint16_t fstat_reg = i2c_read_register16(0x36, 0x3D);
    if (fstat_reg & 1) { // DNR
      Serial.println("DNR flag set, skipping");
    } else {

      // get hibcfg value
      uint16_t hibcfg = i2c_read_register16(0x36, 0xBA);
      // wake up (3 steps, per manufacturer reference)
      i2c_write_register16(0x36, 0x60, 0x0090);
      i2c_write_register16(0x36, 0xBA, 0x0000);
      i2c_write_register16(0x36, 0x60, 0x0000);

      // designcap
      i2c_write_register16(0x36, 0x18, 1000); // 0.5mAh resolution * 500mAh = 1000

      // IchgTerm ; according to datasheet, this is 7.5% of Ireg, which is
      // 1000V/Rreg, which is 500mA, so IchgTerm = 37.5mA. At 156.25 uA
      // resolution, this gives a register value of 240
      i2c_write_register16(0x36, 0x1E, 240);

      // Vempty. 2.8V / 1.25mV resolution = 2240
      i2c_write_register16(0x36, 0x3A, 2240);

      // write model
      i2c_write_register16(0x36, 0xDB, 0x8000);
      
      i2c_write_register16(0x36, 0xBA, hibcfg);


      // clear POR bit
      Serial.println("Clearing POR flag");
      while (i2c_read_register16(0x36, 0x00) & 0x02) { i2c_write_register16(0x36, 0x00, status_reg & 0xFFFD); }
    }
  } else {
    uint16_t battery_pct = i2c_read_register16(0x36, 0x06);
    uint8_t bpct_l = battery_pct & 0xFF;
    uint8_t bpct_h = (battery_pct >> 8) & 0xFF;
    Serial.printf("%d %d (0x%x 0x%x)\r\n", bpct_h, bpct_l, bpct_h, bpct_l);
  }

}


#endif