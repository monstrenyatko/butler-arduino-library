/*
 *******************************************************************************
 *
 * Purpose: HTML page to request WiFi configuration.
 *
 *******************************************************************************
 * Copyright Oleg Kovalenko 2017.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

#ifndef BUTLER_ARDUINO_HTML_WIFI_CONFIG_H_
#define BUTLER_ARDUINO_HTML_WIFI_CONFIG_H_

/* System Includes */
/* Internal Includes */


namespace Butler {
namespace Arduino {

namespace Html {

const char WIFI_CONFIG_PAGE[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang='en'>
<head>
<title>Butler Device Configuration</title>
<meta charset='utf-8'>
<style>
	div,input{font-size:1rem}
	div{padding:0.3rem}
	input{
		width:94%;
		text-align:left;
		padding:0.3rem
	}
	button{
		width:100%;
		text-align:center;
		border:0;border-radius:0.3rem;
		background-color:#1fa3ec;color:#fff;
		line-height:2.4rem;
		font-size:1.2rem
	}
</style>
</head>
<body style='text-align:center;font-family:verdana'>
<div style='min-width:260px;display:inline-block'>
<form action='\wifiConfig' method='post'>
	SSID:<br>
	<input type='text' name='ssid'><br>
	Passphrase:<br>
	<input type='password' name='passphrase'><br>
	<div><button type='submit' value='Submit'>Apply</button></div>
</form>
</div>
</body>
</html>
)=====";

} // Html

}}

#endif // BUTLER_ARDUINO_HTML_WIFI_CONFIG_H_
