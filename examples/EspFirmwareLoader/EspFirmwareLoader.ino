/* System Includes */
#include <Arduino.h>
/* Internal Includes */
#include <ButlerArduinoLibrary.h>
#include <ButlerArduinoLogger.hpp>
#include <ButlerArduinoStrings.hpp>
#include <ButlerArduinoJsonConfig.hpp>
#include <ButlerArduinoWiFiJsonConfig.hpp>
#include <ButlerArduinoEspManager.hpp>


////////// DECLARATION //////////

//// CONFIGURATION ////
struct Configuration: public Butler::Arduino::Config::JsonConfig<1> {
	//// CONSTANTS ////
	static constexpr uint32_t HW_UART_SPEED = 115200L;
	static constexpr uint8_t CFG_RESET_PIN = 13;
	static constexpr uint32_t CFG_RESET_DELAY_MS = (4*1000L);
	static constexpr uint32_t NET_CONNECT_TM_MS = (10*1000L);
	static constexpr uint32_t NET_CONNECT_ERROR_RETRY_TM_MS = (2*60*1000L);
	static constexpr uint32_t IDLE_TM_MS = (2*60*1000L);
	static constexpr const char* NAME_PREFIX = "BUTLER-";
	static constexpr const char* SERVER_ADDR = "192.168.1.185";
	static constexpr uint16_t SERVER_HTTP_PORT = 8000;

	//// PERSISTENCES ////
	Butler::Arduino::Config::WiFiJsonConfig				wifi;

	Configuration() {
		addNode(Butler::Arduino::Strings::WIFI, wifi);
	}
};

////////// OBJECTS //////////
Butler::Arduino::EspManager<Configuration>			manager;

////////// IMPLEMENTATION //////////
void setup() {
	manager.setup();
	LOG_PRINTFLN(manager.getContext(), "#################################");
	LOG_PRINTFLN(manager.getContext(), "###    Butler Firmware Loader");
	LOG_PRINTFLN(manager.getContext(), "#################################");
	////// INIT END //////
	manager.printState();
	manager.waitNetwork();
	manager.checkFirmwareUpdateNotSecure();
}

void loop() {
	manager.idle(manager.getConfig().IDLE_TM_MS);
}
