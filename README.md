ARDUINO MQTT NODE
=================

About
=====
- Simple implementation of the telemetry sensor for the Arduino
(http://arduino.cc/en/Main/arduinoBoardNano) board.
- The sensor uses MQTT (http://mqtt.org) protocol.
- Network communication is built via Serial interface.

Prepare environment
===================

Arduino IDE
-----------
1. Install official IDE from http://arduino.cc/en/Main/Software.
<br/>NOTE: In case of problems with Eclipse plugin please try with version `1.5.2` of the IDE.

Arduino MemoryFree Library
--------------------------
Official site is http://playground.arduino.cc/Code/AvailableMemory

1. Download the library from https://github.com/maniacbug/MemoryFree.
2. Install it to Arduino IDE (`Sketch -> Import Library -> Add Library`)

Arduino MQTT Paho Library
-------------------------
Official site is https://www.eclipse.org/paho

1. Download the Embedded MQTT C/C++ Client library from https://www.eclipse.org/downloads/download.php?file=/paho/arduino.zip
2. Install it to Arduino IDE (`Sketch -> Import Library -> Add Library`)

Eclipse (OPTIONAL)
------------------
1. Install Eclipse IDE for C/C++ Developers (With CDT plugin).
2. Use Eclipse market to install Arduino plugin
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
- The SoftwareSerial is used for the debug purpose.
- Please connect the external serial to the pins 10(RX) and 11(TX) and do not forget about ground pin.
- Port is configured to `4800 bit/s`.
- You can use command like `picocom -c -b 4800 /dev/tty.usbserial-A7036HPP` to get the output.

Network Connection
------------------
- Communication is done via HardwareSerial.
- RAW MQTT packets are sent/received to/from serial.
- If you connect the board to the PC you can use `socat` tool to connect with MQTT Broker.
- Port is configured to `57600 bit/s`.
- Example:`socat -d -d -d FILE:/dev/ttyUSB0,raw,echo=0,b57600,nonblock=1,cs8,clocal=1,cread=1 TCP:10.201.0.2:1883`

FYI: the server can close the TCP connection at any moment => need to restart the `socat` => You can use shell script:
```sh
!#/bin/sh
while [ 1 ]
do
  socat ...
  sleep 2
done
```
<br/>FYI: need to stop the `socat` if you are going to reload your Sketch to the board.

XBee ZigBee/802.15.4 Network Connection
---------------------------------------
You can connect you board to PC via wireless:
 - Connect your board HardwareSerial with XBee wireless module.
 - Connect another XBee to the PC => Any available usb-to-serial shield for XBee could be used.
 - Configure both XBee modules to work in one network and set their serial to `57600 bit/s`.
 - Start on the PC the `socat` tool as above to allow communication with MQTT Broker.

FYI: It is possible to flash the board over XBee connection => it works like wireless serial connection.
<br/> But in this case you need to click reset button on the board at the moment when the loading is started that is not easy...

Application loop behavior
=========================
- Establish connection with MQTT Broker If not available.
- Send telemetry data.
- Get the configuration If available.
