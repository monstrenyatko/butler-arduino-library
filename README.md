BUTLER-ARDUINO-SENSOR
=====================

About
=====
- `Arduino` (https://www.arduino.cc) based telemetry sensor for `BUTLER` smart house framework.
- Data is encoded in JSON format.
- MQTT (http://mqtt.org) protocol is used for network communication.
- Network communication is built via Serial interface.

Prepare environment
===================

Arduino IDE
-----------

Install official IDE from http://arduino.cc/en/Main/Software.
<br/>**Note**: In case of problems please try the version `1.6.9`.

#### SoftwareSerial Library

Official site is https://www.arduino.cc/en/Reference/SoftwareSerial.

1. Open Arduino IDE libraries manager (`Sketch -> Include Library -> Manage Libraries`)
2. Install the `SoftwareSerial` by Arduino

#### MemoryFree Library

Official site is http://playground.arduino.cc/Code/AvailableMemory.

1. Download the library from https://github.com/McNeight/MemoryFree
2. Install it to Arduino IDE (`Sketch -> Include Library -> Add ZIP Library`)

#### MQTT Client Library

Official site is https://github.com/monstrenyatko/ArduinoMqtt

1. Open Arduino IDE libraries manager (`Sketch -> Include Library -> Manage Libraries`)
2. Install the `ArduinoMqtt` by Oleg Kovalenko. Verified version is `1.0.1`

#### JSON Library

Official site is https://github.com/bblanchon/ArduinoJson

1. Open Arduino IDE libraries manager (`Sketch -> Include Library -> Manage Libraries`)
2. Install the `ArduinoJson` by Benoit Blanchon. Verified version is `5.8.0`

#### DHT Sensor Library

1. Open Arduino IDE libraries manager (`Sketch -> Include Library -> Manage Libraries`)
2. Install the `DHT Sensor Library` for DHT11, DHT22. Verified version is `1.3.0`
3. Install dependency library `Adafruit Unified Sensor`

Eclipse (OPTIONAL)
------------------
1. Install Eclipse IDE for C/C++ Developers (With CDT plugin).
2. Install `CDT Cross GCC Built-in Compiler Settings` package using
Eclipse `Install New Software`.
3. In case of problems with `C` files indexing like some types are not resolved
but the definition is definitely available in the included header Need to update
the `Language Mappings` in `Eclipse Preferences` -> `C/C++` -> `Language Mappings`.
Add next mappings:
	- `C Header file` to `C++`
	- `C Source file` to `C++`

PlatformIO Core (OPTIONAL)
--------------------------
See installation [guide](http://docs.platformio.org/en/latest/installation.html)
to setup working `pio` tool.

Building
========

Arduino IDE
-----------
1. Import the project (`File -> Open`)
2. Choose the board in `Tools -> Board` + `Tools -> Processor` + `Tools -> Port`
3. `Sketch -> Verify/Compile` (to build)
4. `Sketch -> Upload` (load to the board)

**Note**: Do not forget to remove all directories/files added by
`PlatformIO Core` if was used before.

Eclipse IDE (PlatformIO Core)
-----------------------------
See [guide](http://docs.platformio.org/en/latest/ide/eclipse.html)

1. Go to project directory:

	```sh
	cd <path to sources>
	```
2. Use `pio` to generate project:

	```sh
	pio init --ide eclipse --board <ID>
	```
3. Import the project (`File -> Import -> General -> Existing Projects into Workspace`)
4. `Project -> Build Targets -> Build -> PlatformIO: Build` (to build)

PlatformIO Core
---------------
1. Go to project directory:

	```sh
	cd <path to sources>
	```
2. Use `pio` to build for all available targets:

	```sh
	pio run
	```
3. Use `pio` to get `port name` list of connected boards:

	```sh
	pio device list
	```
4. Use `pio` to upload specific environment to specific board.
See `platformio.ini` file entries like `[env:<env name>]`.
<br/>**Note**: Add new environments/boards to the`platformio.ini` file if need.

	```sh
	pio run -t upload -e <env name> --upload-port <port name>
	```

Usage
=====

Debug output
------------
- The HardwareSerial is used for debug purpose.
- Serial speed is configured to `57600 bit/s`.
- You can use command like `picocom -r -c -b 57600 <path to serial>` to get the output
or `picocom -i -r -c -b 57600 <path to serial>` to avoid board reset (some times doesn't work).

<br/>FYI: You must stop `picocom` by command `C-a C-q` if you are going to reload your Sketch to the board.

Network Connection
------------------
- The SoftwareSerial is used for communication with network.
- Serial speed is configured to `9600 bit/s`.
- Please connect the network device to pins 10(RX) and 11(TX) and do not forget about ground pin.
- RAW MQTT packets are sent/received to/from serial.

#### XBee® ZigBee/802.15.4
- Wireless connection with encryption support.
- Support One-to-One, One-to-All, All-to-One communications.
- Works like wireless serial connection.

###### All-to-One
Allows to connect ANY quantity of sensors with server via one PC serial interface.
<br/>`butler-xbee-gateway` (https://github.com/monstrenyatko/butler-xbee-gateway) must be used.
- Connect XBee® ZigBee device as a network interface to the board (See `Network Connection`).
- Connect XBee® ZigBee Coordinator device to PC via serial.
- Devices and network configuration are explained on `butler-xbee-gateway` project page.

###### One-to-One 
Allows to connect ONE sensor with server via one PC serial interface.
- Connect XBee® ZigBee device as a network interface to the board (See `Network Connection`).
- Connect another XBee® ZigBee device to PC via serial.
- Configure both XBee® ZigBee devices to work in one network in transparent mode.
- Configure serial speed of XBee® ZigBee devices to `57600 bit/s`.
- Use `socat` tool to start communication between serial (XBee® ZigBee device) and server.
- Example:`socat -d -d -d FILE:<path to serial>,raw,echo=0,b57600,nonblock=1,cs8,clocal=1,cread=1 TCP:1<server address>:<port>`

<br/>FYI: on some OS the `socat` parameters could be different => please read manual.
<br/>FYI: server can close the TCP connection at any moment => need to restart the `socat` => Following shell script could be useful:
```sh
!#/bin/sh
while [ 1 ]
do
  socat ...
  sleep 2
done
```

Sensor Connection
-----------------

Information about sensor is available on https://learn.adafruit.com/dht

Default configuration:
- DHT11 sensor type
- Pin 2 is used to read data from sensor

Please follow the wiring instruction from https://learn.adafruit.com/dht/connecting-to-a-dhtxx-sensor

Application configuration
-------------------------

#### Identifier

Unique string identifier. Must be unique in whole sensor network.
Don't forget to update `SENSOR_ID` value.

#### Logger

Logging is disabled by default. Define `LOG_ENABLED` equal `1` to enable.

If you can't add the define using compiler options (in case of Arduino IDE) just
define it before including the library header:
```c++
#define LOG_ENABLED 1
#include "Logger.h"
```

#### Publishing delay

Delay between data publishing to the server.
Update `MQTT_PUBLISH_PERIOD_MS` to desired value. Value must be in milliseconds.

Application loop behavior
=========================
- Establish connection with MQTT Broker If not connected yet.
- Get configuration If available.
- Collect and send telemetry data.
- Sleep until time for next action.
