// My changes are commented with // pduck27
// TODO: Additional configurable parameters.
// TODO: Make "Use PIR" parameter a valid Checkbox in configuration page

/**
 * ESPTeamsPresence -- A standalone Microsoft Teams presence light 
 *   based on ESP32 and RGB neopixel LEDs.
 *   https://github.com/toblum/ESPTeamsPresence
 *
 * Copyright (C) 2020 Tobias Blum <make@tobiasblum.de>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <Arduino.h>
#include <IotWebConf.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include <WS2812FX.h>
#include <EEPROM.h>
#include "FS.h"
#include "SPIFFS.h"
#include "ESP32_RMT_Driver.h"


// Tool to get certs: https://projects.petrucci.ch/esp32/

// certificate for https://login.microsoftonline.com
// DigiCert Global Root CA, valid until Wed Mar 08 2023, size: 1647 bytes 
const char* rootCACertificateLogin = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIElDCCA3ygAwIBAgIQAf2j627KdciIQ4tyS8+8kTANBgkqhkiG9w0BAQsFADBh\n" \
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
"d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n" \
"QTAeFw0xMzAzMDgxMjAwMDBaFw0yMzAzMDgxMjAwMDBaME0xCzAJBgNVBAYTAlVT\n" \
"MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxJzAlBgNVBAMTHkRpZ2lDZXJ0IFNIQTIg\n" \
"U2VjdXJlIFNlcnZlciBDQTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB\n" \
"ANyuWJBNwcQwFZA1W248ghX1LFy949v/cUP6ZCWA1O4Yok3wZtAKc24RmDYXZK83\n" \
"nf36QYSvx6+M/hpzTc8zl5CilodTgyu5pnVILR1WN3vaMTIa16yrBvSqXUu3R0bd\n" \
"KpPDkC55gIDvEwRqFDu1m5K+wgdlTvza/P96rtxcflUxDOg5B6TXvi/TC2rSsd9f\n" \
"/ld0Uzs1gN2ujkSYs58O09rg1/RrKatEp0tYhG2SS4HD2nOLEpdIkARFdRrdNzGX\n" \
"kujNVA075ME/OV4uuPNcfhCOhkEAjUVmR7ChZc6gqikJTvOX6+guqw9ypzAO+sf0\n" \
"/RR3w6RbKFfCs/mC/bdFWJsCAwEAAaOCAVowggFWMBIGA1UdEwEB/wQIMAYBAf8C\n" \
"AQAwDgYDVR0PAQH/BAQDAgGGMDQGCCsGAQUFBwEBBCgwJjAkBggrBgEFBQcwAYYY\n" \
"aHR0cDovL29jc3AuZGlnaWNlcnQuY29tMHsGA1UdHwR0MHIwN6A1oDOGMWh0dHA6\n" \
"Ly9jcmwzLmRpZ2ljZXJ0LmNvbS9EaWdpQ2VydEdsb2JhbFJvb3RDQS5jcmwwN6A1\n" \
"oDOGMWh0dHA6Ly9jcmw0LmRpZ2ljZXJ0LmNvbS9EaWdpQ2VydEdsb2JhbFJvb3RD\n" \
"QS5jcmwwPQYDVR0gBDYwNDAyBgRVHSAAMCowKAYIKwYBBQUHAgEWHGh0dHBzOi8v\n" \
"d3d3LmRpZ2ljZXJ0LmNvbS9DUFMwHQYDVR0OBBYEFA+AYRyCMWHVLyjnjUY4tCzh\n" \
"xtniMB8GA1UdIwQYMBaAFAPeUDVW0Uy7ZvCj4hsbw5eyPdFVMA0GCSqGSIb3DQEB\n" \
"CwUAA4IBAQAjPt9L0jFCpbZ+QlwaRMxp0Wi0XUvgBCFsS+JtzLHgl4+mUwnNqipl\n" \
"5TlPHoOlblyYoiQm5vuh7ZPHLgLGTUq/sELfeNqzqPlt/yGFUzZgTHbO7Djc1lGA\n" \
"8MXW5dRNJ2Srm8c+cftIl7gzbckTB+6WohsYFfZcTEDts8Ls/3HB40f/1LkAtDdC\n" \
"2iDJ6m6K7hQGrn2iWZiIqBtvLfTyyRRfJs8sjX7tN8Cp1Tm5gr8ZDOo0rwAhaPit\n" \
"c+LJMto4JQtV05od8GiG7S5BNO98pVAdvzr508EIDObtHopYJeS4d60tbvVS3bR0\n" \
"j6tJLp07kzQoH3jOlOrHvdPJbRzeXDLz\n" \
"-----END CERTIFICATE-----\n" \
"";


// certificate for https://graph.microsoft.com
// DigiCert Global Root G2, valid until Fri Jun 28 2024, size: 2122 bytes 
const char* rootCACertificateGraph = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIF8zCCBNugAwIBAgIQCq+mxcpjxFFB6jvh98dTFzANBgkqhkiG9w0BAQwFADBh\n" \
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
"d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH\n" \
"MjAeFw0yMDA3MjkxMjMwMDBaFw0yNDA2MjcyMzU5NTlaMFkxCzAJBgNVBAYTAlVT\n" \
"MR4wHAYDVQQKExVNaWNyb3NvZnQgQ29ycG9yYXRpb24xKjAoBgNVBAMTIU1pY3Jv\n" \
"c29mdCBBenVyZSBUTFMgSXNzdWluZyBDQSAwMTCCAiIwDQYJKoZIhvcNAQEBBQAD\n" \
"ggIPADCCAgoCggIBAMedcDrkXufP7pxVm1FHLDNA9IjwHaMoaY8arqqZ4Gff4xyr\n" \
"RygnavXL7g12MPAx8Q6Dd9hfBzrfWxkF0Br2wIvlvkzW01naNVSkHp+OS3hL3W6n\n" \
"l/jYvZnVeJXjtsKYcXIf/6WtspcF5awlQ9LZJcjwaH7KoZuK+THpXCMtzD8XNVdm\n" \
"GW/JI0C/7U/E7evXn9XDio8SYkGSM63aLO5BtLCv092+1d4GGBSQYolRq+7Pd1kR\n" \
"EkWBPm0ywZ2Vb8GIS5DLrjelEkBnKCyy3B0yQud9dpVsiUeE7F5sY8Me96WVxQcb\n" \
"OyYdEY/j/9UpDlOG+vA+YgOvBhkKEjiqygVpP8EZoMMijephzg43b5Qi9r5UrvYo\n" \
"o19oR/8pf4HJNDPF0/FJwFVMW8PmCBLGstin3NE1+NeWTkGt0TzpHjgKyfaDP2tO\n" \
"4bCk1G7pP2kDFT7SYfc8xbgCkFQ2UCEXsaH/f5YmpLn4YPiNFCeeIida7xnfTvc4\n" \
"7IxyVccHHq1FzGygOqemrxEETKh8hvDR6eBdrBwmCHVgZrnAqnn93JtGyPLi6+cj\n" \
"WGVGtMZHwzVvX1HvSFG771sskcEjJxiQNQDQRWHEh3NxvNb7kFlAXnVdRkkvhjpR\n" \
"GchFhTAzqmwltdWhWDEyCMKC2x/mSZvZtlZGY+g37Y72qHzidwtyW7rBetZJAgMB\n" \
"AAGjggGtMIIBqTAdBgNVHQ4EFgQUDyBd16FXlduSzyvQx8J3BM5ygHYwHwYDVR0j\n" \
"BBgwFoAUTiJUIBiV5uNu5g/6+rkS7QYXjzkwDgYDVR0PAQH/BAQDAgGGMB0GA1Ud\n" \
"JQQWMBQGCCsGAQUFBwMBBggrBgEFBQcDAjASBgNVHRMBAf8ECDAGAQH/AgEAMHYG\n" \
"CCsGAQUFBwEBBGowaDAkBggrBgEFBQcwAYYYaHR0cDovL29jc3AuZGlnaWNlcnQu\n" \
"Y29tMEAGCCsGAQUFBzAChjRodHRwOi8vY2FjZXJ0cy5kaWdpY2VydC5jb20vRGln\n" \
"aUNlcnRHbG9iYWxSb290RzIuY3J0MHsGA1UdHwR0MHIwN6A1oDOGMWh0dHA6Ly9j\n" \
"cmwzLmRpZ2ljZXJ0LmNvbS9EaWdpQ2VydEdsb2JhbFJvb3RHMi5jcmwwN6A1oDOG\n" \
"MWh0dHA6Ly9jcmw0LmRpZ2ljZXJ0LmNvbS9EaWdpQ2VydEdsb2JhbFJvb3RHMi5j\n" \
"cmwwHQYDVR0gBBYwFDAIBgZngQwBAgEwCAYGZ4EMAQICMBAGCSsGAQQBgjcVAQQD\n" \
"AgEAMA0GCSqGSIb3DQEBDAUAA4IBAQAlFvNh7QgXVLAZSsNR2XRmIn9iS8OHFCBA\n" \
"WxKJoi8YYQafpMTkMqeuzoL3HWb1pYEipsDkhiMnrpfeYZEA7Lz7yqEEtfgHcEBs\n" \
"K9KcStQGGZRfmWU07hPXHnFz+5gTXqzCE2PBMlRgVUYJiA25mJPXfB00gDvGhtYa\n" \
"+mENwM9Bq1B9YYLyLjRtUz8cyGsdyTIG/bBM/Q9jcV8JGqMU/UjAdh1pFyTnnHEl\n" \
"Y59Npi7F87ZqYYJEHJM2LGD+le8VsHjgeWX2CJQko7klXvcizuZvUEDTjHaQcs2J\n" \
"+kPgfyMIOY1DMJ21NxOJ2xPRC/wAh/hzSBRVtoAnyuxtkZ4VjIOh\n" \
"-----END CERTIFICATE-----\n" \
"";


// Global settings
// #define NUMLEDS 16  							// Number of LEDs on the strip (if not set via build flags)
// #define DATAPIN 26							// GPIO pin used to drive the LED strip (20 == GPIO/D13) (if not set via build flags)
// #define STATUS_PIN LED_BUILTIN				// User builtin LED for status (if not set via build flags)
#define DEFAULT_POLLING_PRESENCE_INTERVAL "10"  // Default interval to poll for presence info (seconds) // pduck27 changed to 10
#define DEFAULT_ERROR_RETRY_INTERVAL 30			// pduck27 - Default interval to try again after errors
#define DEFAULT_LED_BRIGHTNESS "150" 			// pduck27 - Default LED brightness
#define DEFAULT_LED_BRIGHTNESS_ABSENCE "10"  	// pduck27 - Default LED brightness, when absent
#define DEFAULT_MOTION_INC "60"					// pduck27 - Time in secs added when motion is detected
#define DEFAULT_PIR_PIN "12"					// Default Pin for PIR data  
#define TOKEN_REFRESH_TIMEOUT 60	 			// Number of seconds until exPIRation before token gets refreshed
#define CONTEXT_FILE "/context.json"			// Filename of the context file
#define VERSION "0.14.0"						// Version of the software

#define DBG_PRINT(x) Serial.print(x)
#define DBG_PRINTLN(x) Serial.println(x)


// IotWebConf
// -- Initial name of the Thing. Used e.g. as SSID of the own Access Point.
const char thingName[] = "ESPTeamsPresence";
// -- Initial password to connect to the Thing, when it creates an own Access Point.
const char wifiInitialApPassword[] = "presence";

DNSServer dnsServer;
WebServer server(80);

IotWebConf iotWebConf(thingName, &dnsServer, &server, wifiInitialApPassword);

// Add parameter
#define STRING_LEN 64
#define INTEGER_LEN 16
#define BOOL_LEN 1
char paramClientIdValue[STRING_LEN];
char paramTenantValue[STRING_LEN];
char paramPollIntervalValue[INTEGER_LEN];
char paramNumLedsValue[INTEGER_LEN];
IotWebConfSeparator separator = IotWebConfSeparator();
IotWebConfParameter paramClientId = IotWebConfParameter("Client-ID (Generic ID: 3837bbf0-30fb-47ad-bce8-f460ba9880c3)", "clientId", paramClientIdValue, STRING_LEN, "text", "e.g. 3837bbf0-30fb-47ad-bce8-f460ba9880c3", "3837bbf0-30fb-47ad-bce8-f460ba9880c3");
IotWebConfParameter paramTenant = IotWebConfParameter("Tenant hostname / ID", "tenantId", paramTenantValue, STRING_LEN, "text", "e.g. contoso.onmicrosoft.com");
IotWebConfParameter paramPollInterval = IotWebConfParameter("Presence polling interval (sec) (default: 10)", "pollInterval", paramPollIntervalValue, INTEGER_LEN, "number", "10..300", DEFAULT_POLLING_PRESENCE_INTERVAL, "min='10' max='300' step='5'");  // pduck27 changed to 10 default
IotWebConfParameter paramNumLeds = IotWebConfParameter("Number of LEDs (default: 16)", "numLeds", paramNumLedsValue, INTEGER_LEN, "number", "1..500", "16", "min='1' max='500' step='1'");
byte lastIotWebConfState;
// pduck27 - Start
char paramLedBrightnessDefaultValue[INTEGER_LEN];
char paramLedBrightnessAbsenceValue[INTEGER_LEN];
char paramUsePIRValue[INTEGER_LEN];
char paramPIRPinValue[INTEGER_LEN];
char paramMotionIncValue[INTEGER_LEN];
IotWebConfParameter paramLedBrightnessDefault = IotWebConfParameter("Default LED Brightness (default: 150 [1-255])", "ledBrightnessDefault", paramLedBrightnessDefaultValue, INTEGER_LEN, "number", "1..255", DEFAULT_LED_BRIGHTNESS, "min='1' max='255' step='1'");
IotWebConfParameter paramLedBrightnessAbsence = IotWebConfParameter("Absence LED Brightness (default: 10 [0-255])", "ledBrightnessAbsence", paramLedBrightnessAbsenceValue, INTEGER_LEN, "number", "0..255", DEFAULT_LED_BRIGHTNESS_ABSENCE, "min='0' max='255' step='1'");
IotWebConfParameter paramUsePIR = IotWebConfParameter("Use PIR (0 = inactive and 1 = active)", "usePIR", paramUsePIRValue, INTEGER_LEN, "number", "0..1", false, "min='0' max='1' step='1'");
IotWebConfParameter paramPIRPin = IotWebConfParameter("PIR data Pin (default: 12 [1-255])", "PIRDataPin", paramPIRPinValue, INTEGER_LEN, "number", "1..255", DEFAULT_PIR_PIN, "min='1' max='255' step='1'");
IotWebConfParameter paramMotionInc = IotWebConfParameter("PIR motion increasement duration (default: 60 secs [1-1000])", "motionDuration", paramMotionIncValue, INTEGER_LEN, "number", "1..1000", DEFAULT_MOTION_INC, "min='1' max='1000' step='1'");
// pduck27 - End

// HTTP client
WiFiClientSecure client;

// WS2812FX
WS2812FX ws2812fx = WS2812FX(NUMLEDS, DATAPIN, NEO_GRB + NEO_KHZ800);
int numberLeds;

// OTA update
HTTPUpdateServer httpUpdater;

// Global variables
String user_code = "";
String device_code = "";
uint8_t interval = 5;

String access_token = "";
String refresh_token = "";
String id_token = "";
unsigned int exPIRes = 0;

String availability = "";
String activity = "";

// Statemachine
#define SMODEINITIAL 0               // Initial
#define SMODEWIFICONNECTING 1        // Wait for wifi connection
#define SMODEWIFICONNECTED 2         // Wifi connected
#define SMODEDEVICELOGINSTARTED 10   // Device login flow was started
#define SMODEDEVICELOGINFAILED 11    // Device login flow failed
#define SMODEAUTHREADY 20            // Authentication successful
#define SMODEPOLLPRESENCE 21         // Poll for presence
#define SMODEREFRESHTOKEN 22         // Access token needs refresh
#define SMODEPRESENCEREQUESTERROR 23 // Access token needs refresh
uint8_t state = SMODEINITIAL;
uint8_t laststate = SMODEINITIAL;
static unsigned long tsPolling = 0;
uint8_t retries = 0;

// Multicore
TaskHandle_t TaskNeopixel; 


// pduck27 Start - PIR variables
bool usePIR;							// Use PIR logic
int ledDefaultBrightness;				// Brightness for default state (1-255)
int ledAbsenceBrightness;				// Brightness for absent state (0-255)
int inputPinPIRSensor; 					// Pin of PIR data line
int nextPIRCheck;						// When next PIR check occurs (in general 1 sec)
int activePIRmotionUntil;  		    	// Time until absent state (will be increased each time a motion ccours)
int motionDurationInc;					// Increasement until absent state is reached in milli sec
bool detectedImportantActivity = true;  // Hinders absent mode for specific states
// pduck27 End

/**
 * Helper
 */
// Calculate token lifetime
int getTokenLifetime() {
	return (exPIRes - millis()) / 1000;
}

// Save context information to file in SPIFFS
void saveContext() {
	const size_t capacity = JSON_OBJECT_SIZE(3) + 5000;
	DynamicJsonDocument contextDoc(capacity);
	contextDoc["access_token"] = access_token.c_str();
	contextDoc["refresh_token"] = refresh_token.c_str();
	contextDoc["id_token"] = id_token.c_str();

	File contextFile = SPIFFS.open(CONTEXT_FILE, FILE_WRITE);
	size_t bytesWritten = serializeJsonPretty(contextDoc, contextFile);
	contextFile.close();
	DBG_PRINT(F("saveContext() - Success: "));
	DBG_PRINTLN(bytesWritten);
	// DBG_PRINTLN(contextDoc.as<String>());
}

boolean loadContext() {
	File file = SPIFFS.open(CONTEXT_FILE);
	boolean success = false;

	if (!file) {
		DBG_PRINTLN(F("loadContext() - No file found"));
	} else {
		size_t size = file.size();
		if (size == 0) {
			DBG_PRINTLN(F("loadContext() - File empty"));
		} else {
			const int capacity = JSON_OBJECT_SIZE(3) + 10000;
			DynamicJsonDocument contextDoc(capacity);
			DeserializationError err = deserializeJson(contextDoc, file);

			if (err) {
				DBG_PRINT(F("loadContext() - deserializeJson() failed with code: "));
				DBG_PRINTLN(err.c_str());
			} else {
				int numSettings = 0;
				if (!contextDoc["access_token"].isNull()) {
					access_token = contextDoc["access_token"].as<String>();
					numSettings++;
				}
				if (!contextDoc["refresh_token"].isNull()) {
					refresh_token = contextDoc["refresh_token"].as<String>();
					numSettings++;
				}
				if (!contextDoc["id_token"].isNull()){
					id_token = contextDoc["id_token"].as<String>();
					numSettings++;
				}
				if (numSettings == 3) {
					success = true;
					DBG_PRINTLN(F("loadContext() - Success"));
					if (strlen(paramClientIdValue) > 0 && strlen(paramTenantValue) > 0) {
						DBG_PRINTLN(F("loadContext() - Next: Refresh token."));
						state = SMODEREFRESHTOKEN;
					} else {
						DBG_PRINTLN(F("loadContext() - No client id or tenant setting found."));
					}
				} else {
					Serial.printf("loadContext() - ERROR Number of valid settings in file: %d, should be 3.\n", numSettings);
				}
				// DBG_PRINTLN(contextDoc.as<String>());
			}
		}
		file.close();
	}

	return success;
}

// Remove context information file in SPIFFS
void removeContext() {
	SPIFFS.remove(CONTEXT_FILE);
	DBG_PRINTLN(F("removeContext() - Success"));
}

void startMDNS() {
	DBG_PRINTLN("startMDNS()");
	// Set up mDNS responder
    if (!MDNS.begin(thingName)) {
        DBG_PRINTLN("Error setting up MDNS responder!");
        while(1) {
            delay(1000);
        }
    }
	// MDNS.addService("http", "tcp", 80);

    DBG_PRINT("mDNS responder started: ");
    DBG_PRINT(thingName);
    DBG_PRINTLN(".local");
}


#include "request_handler.h"
#include "spiffs_webserver.h"


// Neopixel control
void setAnimation(uint8_t segment, uint8_t mode = FX_MODE_STATIC, uint32_t color = RED, uint16_t speed = 3000, bool reverse = false) {
	uint16_t startLed, endLed = 0;	

	// Support only one segment for the moment
	if (segment == 0) {
		startLed = 0;
		endLed = numberLeds;
	}
	Serial.printf("setAnimation: %d, %d-%d, Mode: %d, Color: %d, Speed: %d\n", segment, startLed, endLed, mode, color, speed);
	ws2812fx.setSegment(segment, startLed, endLed, mode, color, speed, reverse);
}

void setPresenceAnimation() {
	// Activity: Available, Away, BeRightBack, Busy, DoNotDisturb, InACall, InAConferenceCall, Inactive, InAMeeting, Offline, OffWork, OutOfOffice, PresenceUnknown, Presenting, UrgentInterruptionsOnly

	detectedImportantActivity = false;  // pduck27

	if (activity.equals("Available")) {
		setAnimation(0, FX_MODE_STATIC, GREEN);
	}
	if (activity.equals("Away")) {
		setAnimation(0, FX_MODE_STATIC, YELLOW);
	}
	if (activity.equals("BeRightBack")) {
		setAnimation(0, FX_MODE_STATIC, ORANGE);
	}
	if (activity.equals("Busy")) {
		setAnimation(0, FX_MODE_STATIC, PURPLE);
	}
	if (activity.equals("DoNotDisturb") || activity.equals("UrgentInterruptionsOnly")) {
		setAnimation(0, FX_MODE_STATIC, PINK);
		detectedImportantActivity = true;  // pduck27
	}
	if (activity.equals("InACall")) {
		setAnimation(0, FX_MODE_BREATH, RED);
		detectedImportantActivity = true;  // pduck27
	}
	if (activity.equals("InAConferenceCall")) {
		setAnimation(0, FX_MODE_BREATH, RED, 9000);
		detectedImportantActivity = true;  // pduck27
	}
	if (activity.equals("Inactive")) {
		setAnimation(0, FX_MODE_BREATH, WHITE);
	}
	if (activity.equals("InAMeeting")) {
		setAnimation(0, FX_MODE_SCAN, RED);
	}	
	if (activity.equals("Offline") || activity.equals("OffWork") || activity.equals("OutOfOffice") || activity.equals("PresenceUnknown")) {
		setAnimation(0, FX_MODE_STATIC, BLACK);
	}
	if (activity.equals("Presenting")) {
		setAnimation(0, FX_MODE_COLOR_WIPE, RED);
		detectedImportantActivity = true;  // pduck27
	}
}


/**
 * Application logic
 */

// Handler: Wifi connected
void onWifiConnected() {
	state = SMODEWIFICONNECTED;
}

// Poll for access token
void pollForToken() {
	String payload = "client_id=" + String(paramClientIdValue) + "&grant_type=urn:ietf:params:oauth:grant-type:device_code&device_code=" + device_code;
	Serial.printf("pollForToken()\n");

	// const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(7) + 530; // Case 1: HTTP 400 error (not yet ready)
	const size_t capacity = JSON_OBJECT_SIZE(7) + 10000; // Case 2: Successful (bigger size of both variants, so take that one as capacity)
	DynamicJsonDocument responseDoc(capacity);
	boolean res = requestJsonApi(responseDoc, "https://login.microsoftonline.com/" + String(paramTenantValue) + "/oauth2/v2.0/token", payload, capacity);

	if (!res) {
		state = SMODEDEVICELOGINFAILED;
	} else if (responseDoc.containsKey("error")) {
		const char* _error = responseDoc["error"];
		const char* _error_description = responseDoc["error_description"];

		if (strcmp(_error, "authorization_pending") == 0) {
			Serial.printf("pollForToken() - Wating for authorization by user: %s\n\n", _error_description);
		} else {
			Serial.printf("pollForToken() - Unexpected error: %s, %s\n\n", _error, _error_description);
			state = SMODEDEVICELOGINFAILED;
		}
	} else {
		if (responseDoc.containsKey("access_token") && responseDoc.containsKey("refresh_token") && responseDoc.containsKey("id_token")) {
			// Save tokens and exPIRation
			access_token = responseDoc["access_token"].as<String>();
			refresh_token = responseDoc["refresh_token"].as<String>();
			id_token = responseDoc["id_token"].as<String>();
			unsigned int _exPIRes_in = responseDoc["exPIRes_in"].as<unsigned int>();
			exPIRes = millis() + (_exPIRes_in * 1000); // Calculate timestamp when token exPIRes

			// Set state
			state = SMODEAUTHREADY;
		} else {
			Serial.printf("pollForToken() - Unknown response: %s\n", responseDoc.as<const char*>());
		}
	}
}

// Get presence information
void pollPresence() {
	// See: https://github.com/microsoftgraph/microsoft-graph-docs/blob/ananya/api-reference/beta/resources/presence.md
	// const size_t capacity = JSON_OBJECT_SIZE(4) + 220;
	const size_t capacity = JSON_OBJECT_SIZE(4) + 500; // pduck27
	DynamicJsonDocument responseDoc(capacity);
	boolean res = requestJsonApi(responseDoc, "https://graph.microsoft.com/beta/me/presence", "", capacity, "GET", true);

	if (!res) {
		state = SMODEPRESENCEREQUESTERROR;
		retries++;
	} else if (responseDoc.containsKey("error")) {
		const char* _error_code = responseDoc["error"]["code"];
		if (strcmp(_error_code, "InvalidAuthenticationToken")) {
			DBG_PRINTLN(F("pollPresence() - Refresh needed"));
			tsPolling = millis();
			state = SMODEREFRESHTOKEN;
		} else {
			Serial.printf("pollPresence() - Error: %s\n", _error_code);
			state = SMODEPRESENCEREQUESTERROR;
			retries++;
		}
	} else {
		// Store presence info
		availability = responseDoc["availability"].as<String>();
		activity = responseDoc["activity"].as<String>();
		retries = 0;

		setPresenceAnimation();
	}
}

// Refresh the access token
boolean refreshToken() {
	boolean success = false;
	// See: https://docs.microsoft.com/de-de/azure/active-directory/develop/v1-protocols-oauth-code#refreshing-the-access-tokens
	String payload = "client_id=" + String(paramClientIdValue) + "&grant_type=refresh_token&refresh_token=" + refresh_token;
	DBG_PRINTLN(F("refreshToken()"));

	const size_t capacity = JSON_OBJECT_SIZE(7) + 10000;
	DynamicJsonDocument responseDoc(capacity);
	boolean res = requestJsonApi(responseDoc, "https://login.microsoftonline.com/" + String(paramTenantValue) + "/oauth2/v2.0/token", payload, capacity);

	// Replace tokens and exPIRation
	if (res && responseDoc.containsKey("access_token") && responseDoc.containsKey("refresh_token")) {
		if (!responseDoc["access_token"].isNull()) {
			access_token = responseDoc["access_token"].as<String>();
			success = true;
		}
		if (!responseDoc["refresh_token"].isNull()) {
			refresh_token = responseDoc["refresh_token"].as<String>();
			success = true;
		}
		if (!responseDoc["id_token"].isNull()) {
			id_token = responseDoc["id_token"].as<String>();
		}
		if (!responseDoc["exPIRes_in"].isNull()) {
			int _exPIRes_in = responseDoc["exPIRes_in"].as<unsigned int>();
			exPIRes = millis() + (_exPIRes_in * 1000); // Calculate timestamp when token exPIRes
		}

		DBG_PRINTLN(F("refreshToken() - Success"));
		state = SMODEPOLLPRESENCE;
	} else {
		DBG_PRINTLN(F("refreshToken() - Error:"));
		// Set retry after timeout
		tsPolling = millis() + (DEFAULT_ERROR_RETRY_INTERVAL * 1000);
	}
	return success;
}

// Implementation of a statemachine to handle the different application states
void statemachine() {

	// Statemachine: Check states of iotWebConf to detect AP mode and WiFi Connection attepmt
	byte iotWebConfState = iotWebConf.getState();
	if (iotWebConfState != lastIotWebConfState) {
		if (iotWebConfState == IOTWEBCONF_STATE_NOT_CONFIGURED || iotWebConfState == IOTWEBCONF_STATE_AP_MODE) {
			DBG_PRINTLN(F("Detected AP mode"));
			setAnimation(0, FX_MODE_THEATER_CHASE, WHITE);
		}
		if (iotWebConfState == IOTWEBCONF_STATE_CONNECTING) {
			DBG_PRINTLN(F("WiFi connecting"));
			state = SMODEWIFICONNECTING;
		}
	}
	lastIotWebConfState = iotWebConfState;

	// Statemachine: Wifi connection start
	if (state == SMODEWIFICONNECTING && laststate != SMODEWIFICONNECTING) {
		setAnimation(0, FX_MODE_THEATER_CHASE, BLUE);
	}

	// Statemachine: After wifi is connected
	if (state == SMODEWIFICONNECTED && laststate != SMODEWIFICONNECTED)
	{
		setAnimation(0, FX_MODE_THEATER_CHASE, GREEN);
		startMDNS();
		loadContext();
		// WiFi client
		DBG_PRINTLN(F("Wifi connected, waiting for requests ..."));
	}

	// Statemachine: Devicelogin started
	if (state == SMODEDEVICELOGINSTARTED) {
		if (laststate != SMODEDEVICELOGINSTARTED) {
			setAnimation(0, FX_MODE_THEATER_CHASE, PURPLE);
		}
		if (millis() >= tsPolling) {
			pollForToken();
			tsPolling = millis() + (interval * 1000);
		}
	}

	// Statemachine: Devicelogin failed
	if (state == SMODEDEVICELOGINFAILED) {
		DBG_PRINTLN(F("Device login failed"));
		state = SMODEWIFICONNECTED;	// Return back to initial mode
	}

	// Statemachine: Auth is ready, start polling for presence immediately
	if (state == SMODEAUTHREADY) {
		saveContext();
		state = SMODEPOLLPRESENCE;
		tsPolling = millis();
	}

	// Statemachine: Poll for presence information, even if there was a error before (handled below)
	if (state == SMODEPOLLPRESENCE) {
		if (millis() >= tsPolling) {
			DBG_PRINTLN(F("Polling presence info ..."));
			pollPresence();
			tsPolling = millis() + (atoi(paramPollIntervalValue) * 1000);
			Serial.printf("--> Availability: %s, Activity: %s\n\n", availability.c_str(), activity.c_str());
		}

		if (getTokenLifetime() < TOKEN_REFRESH_TIMEOUT) {
			Serial.printf("Token needs refresh, valid for %d s.\n", getTokenLifetime());
			state = SMODEREFRESHTOKEN;
		}
	}

	// Statemachine: Refresh token
	if (state == SMODEREFRESHTOKEN) {
		if (laststate != SMODEREFRESHTOKEN) {
			setAnimation(0, FX_MODE_THEATER_CHASE, RED);
		}
		if (millis() >= tsPolling) {
			boolean success = refreshToken();
			if (success) {
				saveContext();
			}
		}
	}

	// Statemachine: Polling presence failed
	if (state == SMODEPRESENCEREQUESTERROR) {
		if (laststate != SMODEPRESENCEREQUESTERROR) {
			retries = 0;
		}
		
		Serial.printf("Polling presence failed, retry #%d.\n", retries);
		if (retries >= 5) {
			// Try token refresh
			state = SMODEREFRESHTOKEN;
		} else {
			state = SMODEPOLLPRESENCE;
		}
	}

	// Update laststate
	if (laststate != state) {
		laststate = state;
		DBG_PRINTLN(F("======================================================================"));
	}
}

// pduck27 Start - PIR check function
void checkPIRstate() {	
	if (! usePIR){
		return;
	}
	
	if (millis() > nextPIRCheck) {		
		long state = 0;
		nextPIRCheck = millis() + 1000;
		
		// Check for force status
		if (detectedImportantActivity) {
			state = HIGH;
			DBG_PRINTLN(F("Motion detection forced because of important activity status."));
		} else {
			state = digitalRead(inputPinPIRSensor);
		}
		
		// If motion detected then increase time until absence mode. Else if absence time is over then decrease brightness.
		if (state == HIGH){			
			Serial.printf("Motion detected, set default brightness for %g s.\n", double(motionDurationInc / 1000));								
			ws2812fx.setBrightness(ledDefaultBrightness);									
			activePIRmotionUntil = millis() + motionDurationInc;		
			nextPIRCheck = millis() + 5000; // Additional delay because PIR sends up to 3 secs HIGH state after motion detection
		} else if (millis() > activePIRmotionUntil){		
			DBG_PRINTLN(F("Motion is absent, set absence brightness."));
			ws2812fx.setBrightness(ledAbsenceBrightness);    			
		}
		
	}
}
// pduck27 End

/**
 * Multicore
 */
void neopixelTask(void * parameter) {
	for (;;) {
		ws2812fx.service();
		vTaskDelay(10);
	}
}

void customShow(void) {
	uint8_t *pixels = ws2812fx.getPixels();
	// numBytes is one more then the size of the ws2812fx's *pixels array.
	// the extra byte is used by the driver to insert the LED reset pulse at the end.
	uint16_t numBytes = ws2812fx.getNumBytes() + 1;
	rmt_write_sample(RMT_CHANNEL_0, pixels, numBytes, false); // channel 0
}


/**
 * Main functions
 */
void setup()
{
	Serial.begin(115200);
	DBG_PRINTLN();
	DBG_PRINTLN(F("setup() Starting up..."));
	// Serial.setDebugOutput(true);

	// WS2812FX
	ws2812fx.init();
	rmt_tx_int(RMT_CHANNEL_0, ws2812fx.getPin());
	ws2812fx.start();
	setAnimation(0, FX_MODE_STATIC, WHITE);

	// iotWebConf - Initializing the configuration.
	#ifdef LED_BUILTIN
	iotWebConf.setStatusPin(LED_BUILTIN);
	#endif
	iotWebConf.setWifiConnectionTimeoutMs(5000);
	iotWebConf.addParameter(&separator);
	iotWebConf.addParameter(&paramClientId);
	iotWebConf.addParameter(&paramTenant);
	iotWebConf.addParameter(&paramPollInterval);
	iotWebConf.addParameter(&paramNumLeds);
	// pduck27 - Start
	iotWebConf.addParameter(&paramLedBrightnessDefault);
	iotWebConf.addParameter(&paramLedBrightnessAbsence);
	iotWebConf.addParameter(&paramUsePIR);
	iotWebConf.addParameter(&paramPIRPin);
	iotWebConf.addParameter(&paramMotionInc);	
	// pduck27 - End
	// iotWebConf.setFormValidator(&formValidator);
	// iotWebConf.getApTimeoutParameter()->visible = true;
	// iotWebConf.getApTimeoutParameter()->defaultValue = "10";
	iotWebConf.setWifiConnectionCallback(&onWifiConnected);
	iotWebConf.setConfigSavedCallback(&onConfigSaved);
	iotWebConf.setupUpdateServer(&httpUpdater);
	iotWebConf.skipApStartup();
	iotWebConf.init();

	// WS2812FX
	numberLeds = atoi(paramNumLedsValue);
	if (numberLeds < 1) {
		DBG_PRINTLN(F("Number of LEDs not given, using 16."));
		numberLeds = NUMLEDS;
	}
	ws2812fx.setLength(numberLeds);
	ws2812fx.setCustomShow(customShow);

	// HTTP server - Set up required URL handlers on the web server.
	server.on("/", HTTP_GET, handleRoot);
	server.on("/config", HTTP_GET, [] { iotWebConf.handleConfig(); });
	server.on("/config", HTTP_POST, [] { iotWebConf.handleConfig(); });
	server.on("/upload", HTTP_GET, [] { handleMinimalUpload(); });
	server.on("/api/startDevicelogin", HTTP_GET, [] { handleStartDevicelogin(); });
	server.on("/api/settings", HTTP_GET, [] { handleGetSettings(); });
	server.on("/api/clearSettings", HTTP_GET, [] { handleClearSettings(); });
	server.on("/fs/delete", HTTP_DELETE, handleFileDelete);
	server.on("/fs/list", HTTP_GET, handleFileList);
	server.on("/fs/upload", HTTP_POST, []() {
		server.send(200, "text/plain", "");
	}, handleFileUpload);

	// server.onNotFound([](){ iotWebConf.handleNotFound(); });
	server.onNotFound([]() {
		iotWebConf.handleNotFound();
		if (!handleFileRead(server.uri())) {
			server.send(404, "text/plain", "FileNotFound");
		}
	});

	DBG_PRINTLN(F("setup() ready..."));

	// SPIFFS.begin() - Format if mount failed
	DBG_PRINTLN(F("SPIFFS.begin() "));
	if(!SPIFFS.begin(true)) {
		DBG_PRINTLN("SPIFFS Mount Failed");
        return;
    }

	// Pin neopixel logic to core 0
	xTaskCreatePinnedToCore(
		neopixelTask,
		"Neopixels",
		1000,
		NULL,
		1,
		&TaskNeopixel,
		0);

	// pduck27 Start - PIR and brightness initialization		
	usePIR = atoi(paramUsePIRValue);
	if (usePIR) {
		DBG_PRINTLN("PIR mode is active.");

		// Set PIR Pin
		if (atoi(paramPIRPinValue) < 1 ){
			inputPinPIRSensor = atoi(DEFAULT_PIR_PIN);
			DBG_PRINTLN("PIR Pin not configured well. Must be greater than 0. Use default value instead.");
		} else {
			inputPinPIRSensor = atoi(paramPIRPinValue);
		}
		pinMode(inputPinPIRSensor, INPUT);	
		Serial.printf("PIR pin set to %d.\n", inputPinPIRSensor);

		// Set PIR motion duration increasement
		if ((atoi(paramMotionIncValue) < 1) || (atoi(paramMotionIncValue) > 1000)){
			motionDurationInc = atoi(DEFAULT_MOTION_INC);
			DBG_PRINTLN("PIR duration increasement not configured well. Must be between 1-1000. Use default value instead.");
		} else {
			motionDurationInc = atoi(paramMotionIncValue);
		}
		motionDurationInc *= 1000;
		Serial.printf("PIR motion duration increasement set to %d s.\n", motionDurationInc);

	} else{
		DBG_PRINTLN("PIR mode is not active.");
	}
	
	// Set brightness
	if (atoi(paramLedBrightnessDefaultValue) < 1){
		ledDefaultBrightness = atoi(DEFAULT_LED_BRIGHTNESS);
		ledAbsenceBrightness = atoi(DEFAULT_LED_BRIGHTNESS_ABSENCE);
		DBG_PRINTLN("Brightness is not configured well. Default must be greater than 0. Use defaults instead.");
	} else {
		ledDefaultBrightness = atoi(paramLedBrightnessDefaultValue);
		ledAbsenceBrightness = atoi(paramLedBrightnessAbsenceValue);	
	}
	Serial.printf("Default brightness set to %d and absence brightness set to %d.\n", ledDefaultBrightness, ledAbsenceBrightness);

	// Set default Brightness in general
	ws2812fx.setBrightness(ledDefaultBrightness);
	// pduck27 End
}

void loop()
{
	// iotWebConf - doLoop should be called as frequently as possible.
	iotWebConf.doLoop();

	statemachine();
	
	checkPIRstate(); // pduck27 - PIR integration
}
