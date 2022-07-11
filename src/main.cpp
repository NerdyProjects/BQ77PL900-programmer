#include <Arduino.h>
#include <Wire.h>

#define I2C_ADDR 0x10
#define I2C_ADDR_GAUGE 0x55

/* Pinout: SCL, SDA und GND mit Pinheader verbinden (von unten nach oben: GND, SCL, SDA).
Pin 5 mit Pinheader oben verbinden (EEPROM) für den Toggle zum programmieren. */

uint8_t read_reg(uint8_t reg) {
  uint8_t result = 255;
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(I2C_ADDR, 1, true);
  if(Wire.available()) {
    result = Wire.read();
  }
  return result;
}

void write_reg(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

/*
OV_CFG = 0xFA - 4,25V, Hysterese 0 mV (?), 2,25 Sekunden Ansprechzeit
UV_CFG = 0x1D - 2,7V, 0,4V Hysterese,
OC/UV_DELAY = 0x2A - 60 mV Overcurrent (-> 2 mOhm Shunt macht 30A); 3 Sekunden Ansprechzeit für Unterspannung (FS_BIT in Output Control ist per default 0)
OCD_CFG = 0xBF - 1,6 s Overcurrent Delay, Auto-Recover für OverCurrent nach 12.8 Sekunden ohne Anstecken des Ladegeräts, Balancing an
SCD_CFG = 0x74 - 80 mV (-> 40A) ShortCircuit detect mit 420µs delay
Alternative Settings dafür: OV_CFG auf 0xF8 für 4,15V, 2,25 Sekunden Ansprechzeit, Hysterese 0 (-> lädt die Zelle beispielsweise bei 2 A für 2,25 Sekunden
  über 4,15V, balanced dann mit 100mA also etwa 20 * 2,25 Sekunden, also ne knappe Minute, und lädt dann den Rest... Mit nem 42V Ladegerät sollte am Ende
  kaum noch Ladestrom fließen, natürlich "cycled" der dann trotzdem recht viel bei 4,15V herum)
  Erfahrung damit: Schaltet recht früh ab und balanced dann alle Zellen auf 4.07-4.09V -> Erhöhung auf 0xF9 für 4,2 statt 4,15V:
Erprobte Praxis mit 0xF9: Schaltet kurz vor 4,2V ab, startet aber Balancing in meinem Fall nicht, vermutlich weil Zellen zu gleich(?)
Mit 0xFA verhält es sich ganz gut, inklusive zügig wechselndem Laden/Balancing um die 4,2V.
*/

#define OV_CFG 0xFA
#define UV_CFG 0x1D
#define OC_UV_DELAY 0x2A
#define OCD_CFG 0xBF
#define SCD_CFG 0x74

void program_eeprom() {
  if((read_reg(0x06) == OV_CFG) &&
     (read_reg(0x07) == UV_CFG) &&
     (read_reg(0x08) == OC_UV_DELAY) &&
     (read_reg(0x09) == OCD_CFG) &&
     (read_reg(0x0A) == SCD_CFG)) {
        Serial.println("EEPROM match already");
        return;
      }
  write_reg(0x06, OV_CFG);
  write_reg(0x07, UV_CFG);
  write_reg(0x08, OC_UV_DELAY);
  write_reg(0x09, OCD_CFG);
  write_reg(0x0A, SCD_CFG);
  write_reg(0x0B, 0x62);
  if((read_reg(0x06) != OV_CFG) ||
     (read_reg(0x07) != UV_CFG) ||
     (read_reg(0x08) != OC_UV_DELAY) ||
     (read_reg(0x09) != OCD_CFG) ||
     (read_reg(0x0A) != SCD_CFG)) {
        Serial.println("EEPROM write fail");
        return;
      }
  write_reg(0x0B, 0x41);
  delay(1);
  if((read_reg(0x00) & 0x00) != 0x00) {
    Serial.println("VGOOD_FAIL");
    return;
  }
  digitalWrite(5, HIGH);
  delay(100);
  write_reg(0x0B, 0x00);
  digitalWrite(5, LOW);
  Serial.println("Done write");
}

uint16_t read_gauge_reg(uint8_t reg) {
  uint16_t result = 65535;
  Wire.beginTransmission(I2C_ADDR_GAUGE);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(I2C_ADDR_GAUGE, 2, true);
  if(Wire.available()) {
    result = Wire.read();
    result |= Wire.read() << 8;
  }
  return result;
}

void query_gauge() {
Serial.print("Voltage:");
Serial.println(read_gauge_reg(0x08), DEC);
Serial.print("Current:");
Serial.println((int16_t)read_gauge_reg(0x0A), DEC);
}

void setup() {
  Wire.begin();
  Serial.begin(9600);
  digitalWrite(5, LOW);
  pinMode(5, OUTPUT);
  delay(5000);
  program_eeprom();
}

void loop() {
  query_gauge();
  Serial.print("BALANCE 0x03:");
  Serial.println(read_reg(0x03), HEX);
  Serial.print("BALANCE 0x04:");
  Serial.println(read_reg(0x04), HEX);
  Serial.print("OV_CFG:");
  Serial.println(read_reg(0x06), HEX);
  Serial.print("UV_LEVEL:");
  Serial.println(read_reg(0x07), HEX);
  Serial.print("OCV:");
  Serial.println(read_reg(0x08), HEX);
  Serial.print("OCD:");
  Serial.println(read_reg(0x09), HEX);
  Serial.print("SCD:");
  Serial.println(read_reg(0x0A), HEX);
  Serial.print("status:");
  Serial.println(read_reg(0x00), HEX);
  delay(1000);
}