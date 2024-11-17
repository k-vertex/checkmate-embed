#include <SPI.h>
#include <Ethernet.h>
#include <string.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

char server[] = "www.emperorchang.store"; 

EthernetClient client;

#include <Wire.h>
#include <Adafruit_PN532.h>

#define PN532_SCK  (2)
#define PN532_MOSI (3)
#define PN532_SS   (4)
#define PN532_MISO (5)

#define PN532_IRQ   (2)
#define PN532_RESET (3)

Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);

#if defined(ARDUINO_ARCH_SAMD)
   #define Serial SerialUSB
#endif

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
 EthernetInit();
 nfcInit();
}

void EthernetInit() {
  if(Ethernet.begin(mac) == 0) {
    if(Ethernet.hardwareStatus() == EthernetNoHardware) {
      while (true) {
        delay(1); 
      }
    }
    if(Ethernet.linkStatus() != LinkOFF) {
      Serial.print("  IP: ");
      Serial.println(Ethernet.localIP());
    }
  delay(1000);
  }
}

void nfcInit() {
  nfc.begin();
    
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1);
  }

  nfc.SAMConfig();
}

void loop() {
  bool success;
  
  uint8_t responseLength = 32;

  success = nfc.inListPassiveTarget();

  if(success) {       
    uint8_t selectApdu[] = { 0x00, /* CLA */
                              0xA4, /* INS */
                              0x04, /* P1  */
                              0x00, /* P2  */
                              0x07, /* Length of AID  */
                              0xF0, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, /* AID defined on Android App */
                              0x00  /* Le  */ };
                              
    uint8_t response[32];  
     
    success = nfc.inDataExchange(selectApdu, sizeof(selectApdu), response, &responseLength);
    
    if(success) {
      
      nfc.PrintHexChar(response, responseLength);

      if(client.connect(server, 8888)) {
        String postData = "deviceToken=" + responseToString(response, responseLength);
        client.println("POST /attend HTTP/1.1");
        client.println("Host: emperorchang.store:8888");
        client.println("Content-Type: application/x-www-form-urlencoded");
        client.print("Content-Length: ");
        client.println(postData.length());
        client.println();
        client.println(postData);
        Serial.println("success");
        client.stop();
      }
      else
       Serial.println("failed");
    }
  }
  delay(1000);
}

String responseToString(uint8_t *response, uint8_t responseLength) {
  String respBuffer;
  String str;

  for(int i = 0; i < responseLength; i++) {
    if(response[i] < 0x10) 
      respBuffer = respBuffer + "0"; 
      respBuffer += String(response[i], HEX);                        
    }

    for(int i = 0; i < respBuffer.length(); i+=2) {
      String split = respBuffer.substring(i, i+2);
      char ch = strtol(split.c_str(), NULL, 16);
      str += ch;
    }

  return str;
}
