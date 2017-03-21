BUTLER-ARDUINO-LIBRARY
======================

About
=====
- Library to simplify telemetry sensors and IoT controllers development
- Based on [Arduino](https://www.arduino.cc) framework

Features
========

TBD

Examples
========

Sensors
-------

#### MQTT-XBEE-DHT-SENSOR for AVR

See example [README](examples/AvrSensorMqttXbeeDhtLpm/README.md).

Controllers
-----------

TODO

Development
===========

Eclipse IDE (PlatformIO Core)
-----------------------------

See [guide](http://docs.platformio.org/en/latest/ide/eclipse.html).

1. Install Eclipse IDE for C/C++ Developers (With CDT plugin).
2. Install `CDT Cross GCC Built-in Compiler Settings` package using
Eclipse `Install New Software`.
3. Go to project directory:

	```sh
	cd <path to sources>
	```
4. Use `pio` to generate project:

	```sh
	pio init --ide eclipse --board <ID>
	```
5. Import the project (`File -> Import -> General -> Existing Projects into Workspace`)
6. In case of problems with `C` files indexing like some types are not resolved
but the definition is definitely available in the included header Need to update
the `Language Mappings` in `Eclipse Preferences` -> `C/C++` -> `Language Mappings`.
Add next mappings:
	- `C Header file` to `C++`
	- `C Source file` to `C++`
