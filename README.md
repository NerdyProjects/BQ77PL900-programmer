# BQ77PL900 Programmer

This arduino tools programs a BQ77PL900 BMS chip for given parameters (overvoltage, undervoltage, overcurrent, short circuit, stand alone balancing).

It was used for a conti ebike battery and also supports reading out the fuel gauge BQ34Z100.

Warning: Use a 3.3V arduino (e.g. Arduino Pro Mini 3,3V). I2C pullups are internal to BQ77PL900, so they don't need to be (and should not) be provided in your circuit.

Pin 5 is used to connect to the EEPROM pin to actually fulfill the write.

The software in its current state does not detect when the EEPROM write failed as the shadow register is being read back: A failed write looks like a successful one until you power cycle the BQ77PL900.

Don't forget to turn the BQ77PL900 back on by supplying voltage on the PACK Pin after a power cycle.
