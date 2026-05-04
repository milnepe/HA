# Build with Arduino CLI

## Install libraries
pete@xps:ha$ arduino-cli
lib install "DallasTemperature"
Downloading DallasTemperature@4.0.6...
DallasTemperature@4.0.6 DallasTemperature@4.0.6 already downloaded
Installing DallasTemperature@4.0.6...
Installed DallasTemperature@4.0.6

## Compile
```
arduino-cli compile --fqbn 'arduino:samd:nano_33_iot' sensors/temp-sensor-basic
#arduino-cli compile --fqbn arduino:samd:mkrwifi1010 sensors/temp-sensor-sm

Sketch uses 36136 bytes (13%) of program storage space. Maximum is 262144 bytes.
Global variables use 4180 bytes (12%) of dynamic memory, leaving 28588 bytes for local variables. Maximum is 32768 bytes.

Used library      Version Path                                                                     
WiFiNINA          2.0.1   /home/pete/Arduino/libraries/WiFiNINA                                    
PubSubClient      2.8     /home/pete/Arduino/libraries/PubSubClient                                
OneWire           2.3.8   /home/pete/Arduino/libraries/OneWire                                     
DallasTemperature 4.0.6   /home/pete/Arduino/libraries/DallasTemperature                           
Arduino_SpiNINA   0.0.2   /home/pete/Arduino/libraries/Arduino_SpiNINA                             
SPI               1.0     /home/pete/.arduino15/packages/arduino/hardware/samd/1.8.14/libraries/SPI

Used platform Version Path                                                       
arduino:samd  1.8.14  /home/pete/.arduino15/packages/arduino/hardware/samd/1.8.14
```

## Flash
```
arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:samd:nano_33_iot sensors/temp-sensor-basic
Planes       : 1
Lock Regions : 16
Locked       : none
Security     : false
Boot Flash   : true
BOD          : true
BOR          : true
Arduino      : FAST_CHIP_ERASE
Arduino      : FAST_MULTI_PAGE_WRITE
Arduino      : CAN_CHECKSUM_MEMORY_BUFFER
Erase flash
done in 0.869 seconds

Write 36136 bytes to flash (565 pages)
[==============================] 100% (565/565 pages)
done in 0.210 seconds

Verify 36136 bytes of flash with checksum.
Verify successful
done in 0.032 seconds
CPU reset.
New upload port: /dev/ttyACM0 (serial)

```

## Monitor
```
arduino-cli monitor -p /dev/ttyACM0 -c baudrate=115200
Monitor port settings:
baudrate=115200
Connected to /dev/ttyACM0! Press CTRL-C to exit.
Temp: 21.50
Temp: 21.50
```

