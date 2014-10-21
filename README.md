GOLIATH MQTT SENSOR
===================

About
=====
- Simple implementation of the telemetry sensor for the Arduino Nano
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
2. `Sketch -> Verify/Compile` (to build)
3. `File -> Upload` (load to the board)

Eclipse
-------
1. Import the project (`File -> Import -> General -> Existing Projects into Workspace`)
2. `Arduino -> verify` (to build)
3. `Arduino -> Upload Sketch` (load to the board)


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
- If you connect the board to you PC you can use `socat` tool to connect with MQTT Broker.
- Port is configured to `9600 bit/s`.
- Example: `socat -d -d -d /dev/tty.usbserial-A9MP9F7V,raw,echo=0,ispeed=9600,ospeed=9600,clocal=1,cs8,nonblock=1,ixoff=0,ixon=0,crtscts=0 TCP:10.201.0.2:1883`
<br/>FYI: the server can close the TCP connection at any moment => need to restart the `socat`.
<br/>FYI: need to stop the `socat` if you are going to reload your Sketch to the board.

Application loop behaviour
==========================
- Establish connection with MQTT Broker If not available.
- Send telemetry data.
- Get the configuration If available
