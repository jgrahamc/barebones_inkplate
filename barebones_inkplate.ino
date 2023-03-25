// Barebones code for an Inkplate 10 that does the following:
//
// 1. Connects to a WiFi network
// 2. Gets the current IP address by making an HTTPS API call to 
//    a service on the Internet
// 3. Displays the IP address on screen
// 4. Goes into deep sleep for many seconds.
// 5. GOTO 1
//
// Copyright (c) 2023 John Graham-Cumming

#include <Inkplate.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <driver/rtc_io.h>

#include "params.h"

Inkplate display(INKPLATE_3BIT); 

void setup()
{
    display.begin();

    if (connectWiFi(wifi_network, wifi_password)) {
      showIP();
      disconnectWiFi();
    }

    deepSleep();
}

// Nothing here because the entire sketch will be woken up in setup(), do
// work and then go to sleep again.
void loop()
{
}

// connectWiFi connects to the passed in WiFi network and returns true
// if successful. 
bool connectWiFi(const char *ssid, const char *pass) {
  bool success = display.joinAP(ssid, pass);
  if (!success) {
    fatal("Failed to connect to WiFi " + String(ssid));
  }
  
  return success;
}

// disconnectWiFi cleans up the result of connecting via connectWiFi
void disconnectWiFi() {
  display.disconnect();
}

// CA certificate for the domain being connected to using TLS. Obtained
// using the openssl s_client as follows:
//
// openssl s_client -showcerts -servername api.ipify.org \
//     -connect api.ipify.org:443
//
// The certificate below is the last certificate output by openssl as it
// is the CA certificate of the domain above.
const char *cacert = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIEMjCCAxqgAwIBAgIBATANBgkqhkiG9w0BAQUFADB7MQswCQYDVQQGEwJHQjEb\n" \
"MBkGA1UECAwSR3JlYXRlciBNYW5jaGVzdGVyMRAwDgYDVQQHDAdTYWxmb3JkMRow\n" \
"GAYDVQQKDBFDb21vZG8gQ0EgTGltaXRlZDEhMB8GA1UEAwwYQUFBIENlcnRpZmlj\n" \
"YXRlIFNlcnZpY2VzMB4XDTA0MDEwMTAwMDAwMFoXDTI4MTIzMTIzNTk1OVowezEL\n" \
"MAkGA1UEBhMCR0IxGzAZBgNVBAgMEkdyZWF0ZXIgTWFuY2hlc3RlcjEQMA4GA1UE\n" \
"BwwHU2FsZm9yZDEaMBgGA1UECgwRQ29tb2RvIENBIExpbWl0ZWQxITAfBgNVBAMM\n" \
"GEFBQSBDZXJ0aWZpY2F0ZSBTZXJ2aWNlczCCASIwDQYJKoZIhvcNAQEBBQADggEP\n" \
"ADCCAQoCggEBAL5AnfRu4ep2hxxNRUSOvkbIgwadwSr+GB+O5AL686tdUIoWMQua\n" \
"BtDFcCLNSS1UY8y2bmhGC1Pqy0wkwLxyTurxFa70VJoSCsN6sjNg4tqJVfMiWPPe\n" \
"3M/vg4aijJRPn2jymJBGhCfHdr/jzDUsi14HZGWCwEiwqJH5YZ92IFCokcdmtet4\n" \
"YgNW8IoaE+oxox6gmf049vYnMlhvB/VruPsUK6+3qszWY19zjNoFmag4qMsXeDZR\n" \
"rOme9Hg6jc8P2ULimAyrL58OAd7vn5lJ8S3frHRNG5i1R8XlKdH5kBjHYpy+g8cm\n" \
"ez6KJcfA3Z3mNWgQIJ2P2N7Sw4ScDV7oL8kCAwEAAaOBwDCBvTAdBgNVHQ4EFgQU\n" \
"oBEKIz6W8Qfs4q8p74Klf9AwpLQwDgYDVR0PAQH/BAQDAgEGMA8GA1UdEwEB/wQF\n" \
"MAMBAf8wewYDVR0fBHQwcjA4oDagNIYyaHR0cDovL2NybC5jb21vZG9jYS5jb20v\n" \
"QUFBQ2VydGlmaWNhdGVTZXJ2aWNlcy5jcmwwNqA0oDKGMGh0dHA6Ly9jcmwuY29t\n" \
"b2RvLm5ldC9BQUFDZXJ0aWZpY2F0ZVNlcnZpY2VzLmNybDANBgkqhkiG9w0BAQUF\n" \
"AAOCAQEACFb8AvCb6P+k+tZ7xkSAzk/ExfYAWMymtrwUSWgEdujm7l3sAg9g1o1Q\n" \
"GE8mTgHj5rCl7r+8dFRBv/38ErjHT1r0iWAFf2C3BUrz9vHCv8S5dIa2LX1rzNLz\n" \
"Rt0vxuBqw8M0Ayx9lt1awg6nCpnBBYurDC/zXDrPbDdVCYfeU0BsWO/8tqtlbgT2\n" \
"G9w84FoVxp7Z8VlIMCFlA2zs6SFz7JsDoeA3raAVGI/6ugLOpyypEBMs1OUIJqsi\n" \
"l2D4kF501KKaU73yqWjgom7C12yxow+ev+to51byrvLjKzg6CYG1a4XXvi3tPxq3\n" \
"smPi9WIsgtRqAEFQ8TmDn5XpNpaYbg==\n" \
"-----END CERTIFICATE-----\n";

// showIP retrieves the IP address that the device is using to connect
// to the Internet and displays it on screen.
void showIP() {
  WiFiClientSecure tls;
  tls.setCACert(cacert); 
  HTTPClient http;
  
  if (http.begin(tls, "https://api.ipify.org/")) {
    int httpCode = http.GET();
    if (httpCode == 200) {
      text("My IP: " + http.getString() );
    } else {
      fatal("HTTP GET failed");
    }
  } else {
    fatal("http.begin failed");
  }
}

// clear clears the display
void clear() {
  display.clearDisplay();
  display.display();
}

// show displays whatever has been written to the display on the
// e-ink screen itself.
void show() {
  display.display();
}

// fatal is used to show a fatal error on screen
void fatal(String s) {
  clear();
  display.setTextColor(0, 7);
  display.setTextSize(8);
  display.setCursor(100, 100);
  display.print(s);
  show();
}

// text displays a message on screen.
void text(String s) {
  fatal(s);
}

// deepSleep puts the device into deep sleep mode for sleep_time
// seconds. When it wakes up setup() will be called.
void deepSleep() {
    esp_sleep_enable_timer_wakeup(sleep_time * 1000000);
    esp_deep_sleep_start();
}
