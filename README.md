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
1. Install official IDE from http://arduino.cc/en/Main/Software.
<br/>NOTE: In case of problems with Eclipse plugin please try with version `1.6.5` of the IDE.

Arduino MemoryFree Library
--------------------------
Official site is http://playground.arduino.cc/Code/AvailableMemory

1. Download the library from https://github.com/maniacbug/MemoryFree.
2. Install it to Arduino IDE (`Sketch -> Include Library -> Add ZIP Library`)

Arduino MQTT Paho Library
-------------------------
Official site is https://www.eclipse.org/paho

1. Download the Embedded MQTT C/C++ Client library from https://www.eclipse.org/downloads/download.php?file=/paho/arduino.zip
2. Install it to Arduino IDE (`Sketch -> Include Library -> Add ZIP Library`)

Arduino JSON Library
--------------------
Official site is https://github.com/bblanchon/ArduinoJson

1. Download the library from https://github.com/bblanchon/ArduinoJson/releases
2. Install it to Arduino IDE (`Sketch -> Include Library -> Add ZIP Library`)

<br/> NOTE: In case of compilation problem of the library try to remove
`third-party` and `test` directories from the library.

Arduino DHT Sensor Library
--------------------------

1. Download the library with version `1.2.3` from https://github.com/adafruit/DHT-sensor-library/releases
2. Install it to Arduino IDE (`Sketch -> Include Library -> Add ZIP Library`)

Eclipse (OPTIONAL)
------------------
1. Install Eclipse IDE for C/C++ Developers (With CDT plugin).
2. Use Eclipse market to install `Arduino eclipse IDE` plugin
(http://marketplace.eclipse.org/content/arduino-eclipse-ide)
3. Configure the path to Arduino IDE (`Preferences -> Arduino`)

Building
========

Two options are available.

Arduino IDE
-----------
1. Import the project (`File -> Open`)
2. Choose the board in `Tool -> Board` + `Tool -> Processor` + `Tool -> Serial Port`
3. `Sketch -> Verify/Compile` (to build)
4. `File -> Upload` (load to the board)

Eclipse
-------
1. Import the project (`File -> Import -> General -> Existing Projects into Workspace`)
2. Choose the board in `Project -> Properties -> Arduino`
3. `Arduino -> verify` (to build)
4. `Arduino -> Upload Sketch` (load to the board)


Usage
=====

Debug output
------------
- The HardwareSerial is used for debug purpose.
- Serial speed is configured to `57600 bit/s`.
- You can use command like `picocom -c -b 57600 <path to serial>` to get the output.

<br/>FYI: You must stop `picocom` by command `C-a C-x` if you are going to reload your Sketch to the board.

Network Connection
------------------
- The SoftwareSerial is used for communication with network.
- Serial speed is configured to `57600 bit/s`.
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

Information abous sensor is available on https://learn.adafruit.com/dht

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

Change `LOG_ENABLED` to `0` in `Logger.h` to disable or to `1` for enabling.

#### Publishing delay

Delay between data publishing to the server.
Default publishing period is `15` seconds.
Update `MQTT_PUBLISH_PERIOD_MS` to desired value. Value must be in milliseconds.

Application loop behavior
=========================
- Establish connection with MQTT Broker If not connected yet.
- Get configuration If available.
- Collect and send telemetry data.
- Sleep until time for next iteration.
