// DA9224 PMIC Control

#include <Wire.h> // I2C Library

#define I2C_WRITE_PAGE0_1    0xD0       // PAGE 0 & 1 Default Write Address 
#define I2C_READ_PAGE0_1     0xD1       // PAGE 0 & 1 Default Read Address
#define I2C_WRITE_PAGE2      0xD2       // PAGE 2 Default Write Address
#define I2C_READ_PAGE2       0xD3       // PAGE 2 Default Read Address

#define PAGE_CON             0x00       // 0x80 0X100 0x200
#define PAGE_0_1             0b10000000 // Selects Page 0/1 with Page Revert bit Set
#define PAGE_2               0b10000010 // Selects Page 2 with Page Revert bit Set


// Page 0 Register Addresses
#define STATUS_A             0x50
#define STATUS_B             0x51
#define EVENT_A              0x52
#define EVENT_B              0x53
#define MASK_A               0x54
#define MASK_B               0x55
#define CONTROL_A            0x56
#define GPI0_1               0x58
#define GPIO2_3              0x59
#define GPI4                 0x5A
#define BUCKA_CONT           0x5D
#define BUCKB_CONT           0x5E

// Page 1 Register Addresses
#define BUCK_ILIM            0xD0
#define BUCKA_CONF           0xD1
#define BUCKB_CONF           0xD2
#define BUCK_CONF            0xD3
#define VBUCKA_A             0xD7
#define VBUCKA_B             0xD8
#define VBUCKB_A             0xD9
#define VBUCKB_B             0xDA

// Page 2 Register Addresses
#define CONFIG_A             0x143
#define CONFIG_B             0x144
#define CONFIG_C             0x145
#define CONFIG_D             0x146
#define CONFIG_E             0x147
#define CONFIG_F             0x148

// Pin Definitions
#define IC_EN                1 // PD3
#define NIRQ                 0 // PD2


// I2C Write Function | I2C Address, Register Page, Register and Data variables
void da9224_write(uint8_t I2C_ADDR, uint8_t page, uint8_t reg, uint8_t data) 
{
  Wire.beginTransmission(I2C_WRITE_PAGE0_1 >> 1);   // Start condition > Slave Address > Write bit
  Wire.write(PAGE_CON);                             // Register Address
  Wire.write(page);                                 // Data
  Wire.endTransmission();                           // Stop Condition

  Wire.beginTransmission(I2C_ADDR >> 1);            // Start condition > Slave Address > Write bit
  Wire.write(reg);                                  // Register Address
  Wire.write(data);                                 // Data
  Wire.endTransmission();                           // Stop Condition
}


// I2C Read Function | I2C Address, Register Page and Register variables
uint8_t da9224_read(uint8_t I2C_ADDR, uint8_t page, uint8_t reg)
{
  Wire.beginTransmission(I2C_WRITE_PAGE0_1 >> 1);   // Start condition > Slave Address > Write bit
  Wire.write(PAGE_CON);                             // Register Address
  Wire.write(page);                                 // Data
  Wire.endTransmission();                           // Stop Condition

  Wire.beginTransmission(I2C_ADDR >> 1);            // Start condition > Slave Address > Read bit
  Wire.write(reg);                                  // Register Address
  Wire.endTransmission(false);                      // Repeated START condition
  Wire.requestFrom(I2C_ADDR >> 1, 1);               // Slave Address > Read bit > Stop Condition
  return Wire.read();                               // Returns Register Data
} 



void setup()
{
  pinMode(IC_EN, OUTPUT);
  pinMode(NIRQ, INPUT);

  digitalWrite(IC_EN, HIGH);

  Wire.begin();
  delay(10);

  da9224_write(I2C_WRITE_PAGE0_1, PAGE_0_1, MASK_A, 0b11111111);      // Mask all bits in EVENT_A
  da9224_write(I2C_WRITE_PAGE0_1, PAGE_0_1, MASK_B, 0b00000011);      // Mask PWRGOOD_B and PWRGOOD_A
  da9224_write(I2C_WRITE_PAGE0_1, PAGE_0_1, CONTROL_A, 0b00101010);   // V_LOCK off, slew rate set to 10mV per 2us & 1ms debounce time on GPIs
  da9224_write(I2C_WRITE_PAGE0_1, PAGE_0_1, GPI0_1, 0b01000100);      // BUCK ENABLE Signals Active HIGH
  da9224_write(I2C_WRITE_PAGE0_1, PAGE_0_1, GPIO2_3, 0b11111111);     // PGOOD Signals Active Level, Active HIGH & Push-Pull
  da9224_write(I2C_WRITE_PAGE0_1, PAGE_0_1, GPI4, 0b00000100);        // I2C ADDR SELECT Signal Active HIGH
  da9224_write(I2C_WRITE_PAGE0_1, PAGE_0_1, BUCKA_CONT, 0b00001010);  // BUCK0 option 1 voltage selected, Pulldown disabled & Buck enable GPI0 controlled
  da9224_write(I2C_WRITE_PAGE0_1, PAGE_0_1, BUCKB_CONT, 0b00001100);  // BUCK1 option 1 voltage selected, Pulldown disabled & Buck enable GPI1 controlled

  da9224_write(I2C_WRITE_PAGE0_1, PAGE_0_1, BUCK_ILIM, 0b10011001);   // Current limit per phase set to 5800mA
  da9224_write(I2C_WRITE_PAGE0_1, PAGE_0_1, BUCKA_CONF, 0b01001011);  // Buck0 power up/down slew rate set to 5mV per 1us - FORCE AUTO
  da9224_write(I2C_WRITE_PAGE0_1, PAGE_0_1, BUCKB_CONF, 0b01001011);  // Buck1 power up/down slew rate set to 5mV per 1us - FORCE AUTO
  da9224_write(I2C_WRITE_PAGE0_1, PAGE_0_1, BUCK_CONF, 0b00000101);   // Phase shedding disabled & 2 phase mode enabled for each buck
  da9224_write(I2C_WRITE_PAGE0_1, PAGE_0_1, VBUCKA_A, 0b11000110);    // Buck0 option 1 set to 1V - AUTO Mode
  da9224_write(I2C_WRITE_PAGE0_1, PAGE_0_1, VBUCKB_A, 0b11000110);    // Buck1 option 1 set to 1V - AUTO Mode
  da9224_write(I2C_WRITE_PAGE0_1, PAGE_0_1, VBUCKA_B, 0b00101101);    // Buck0 option 2 set to 0.75V - Force PWM Mode
  da9224_write(I2C_WRITE_PAGE0_1, PAGE_0_1, VBUCKB_B, 0b01111000);    // Buck1 option 2 set to 1.5V - Force PWM Mode
                                                                   
  // da9224_write(I2C_WRITE_PAGE2, PAGE_2, CONFIG_A, 0b00011010);        // Automatic I2C reset enabled, GPIs use VDDIO, Open Drain & Active LOW for nIRQ
  // da9224_write(I2C_WRITE_PAGE2, PAGE_2, CONFIG_B, 0b11101010);        // Disable UVLO for VDDIO, PGOOD Masked during DVC transitions, OCB Mask during DVC transitions (2us extra masking)
  // da9224_write(I2C_WRITE_PAGE2, PAGE_2, CONFIG_C, 0b00010011);        // GPI4, GPI1 and GPI0 pulldowns enabled
  // da9224_write(I2C_WRITE_PAGE2, PAGE_2, CONFIG_D, 0b01100000);        // BUCK0 PGOOD set to GPIO3, BUCK1 PGOOD set to GPIO2
  // da9224_write(I2C_WRITE_PAGE2, PAGE_2, CONFIG_E, 0b10000000);        // PMIC set to STAND_ALONE mode
  // da9224_write(I2C_WRITE_PAGE2, PAGE_2, CONFIG_F, 0b11010011);        // No difference for I2C alternate address (0xD0/1/2/3), GPI4 used to switch to alternate address.
// 
}


void loop()
{

}