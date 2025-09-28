/*
  Static receiver node.

  Based on the Ping-Pong example from RadioLib library. Details can be found below:
  
  For default module settings, see the wiki page
  https://github.com/jgromes/RadioLib/wiki/Default-configuration#sx126x---lora-modem

  For full API reference, see the GitHub Pages
  https://jgromes.github.io/RadioLib/
*/

// include the library
#include <RadioLib.h>
#include <AESLib.h>

//decryption
AESLib aesLib;
const uint8_t aes_key[16] = {
  //insert key here. this must match the key of the mobile node.
};

#define INITIATING_NODE

// SX1262 has the following connections:
// NSS pin:   10
// DIO1 pin:  2
// NRST pin:  3
// BUSY pin:  9
SX1262 radio = new Module(41, 39, 42, 40);


// save transmission states between loops
int transmissionState = RADIOLIB_ERR_NONE;

// flag to indicate transmission or reception state
bool transmitFlag = false;

// flag to indicate that a packet was sent or received
volatile bool operationDone = false;

// this function is called when a complete packet
// is transmitted or received by the module
// IMPORTANT: this function MUST be 'void' type
//            and MUST NOT have any arguments!

void IRAM_ATTR setFlag(void) {
  // we sent or received a packet, set the flag
  operationDone = true;
}

int parseData(uint8_t* decryptedData){
  //make structure global at some point
  #pragma pack(push, 1)
  struct Payload{
    uint8_t id;
    int32_t lat;
    int32_t lng;
    uint16_t alt;
    uint8_t endFlag;
  };
  #pragma pack(pop)

  Payload p;
  memcpy(&p, decryptedData, sizeof(Payload));
  float lat = p.lat/1e6;
  float lng = p.lng/1e6;
  float alt = p.alt/100.0f;

  //checking ID and endFlag just to make use of them. But haven't implemented them properly in this program... 
  if(p.id == 0x01 && p.endFlag == 0xFF){
    float RSSI = radio.getRSSI();
    float SNR = radio.getSNR();
    String data = "#DATA:<" + String(lat, 6) + "," + String(lng, 6) + "," + String(alt, 6) + "," + String(RSSI) + "," + String(SNR) + ">";
    Serial.println(data);
    return 1;
  }
  else{
    Serial.println(F("Incorrect ID!"));
    return 0;
  }
}


void setup() {
  Serial.begin(115200);

  // initialize SX1262 with default settings
  Serial.print(F("[SX1262] Initializing ... "));
  int state = radio.begin(868.0);
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true) { delay(10); }
  }

  // set the function that will be called
  // when new packet is received
  radio.setDio1Action(setFlag);

  #if defined(INITIATING_NODE)
    // send the first packet on this node
    Serial.print(F("[SX1262] Sending first packet ... "));
    transmissionState = radio.startTransmit("Hello World!");
    transmitFlag = true;
  #else
    // start listening for LoRa packets on this node
    Serial.print(F("[SX1262] Starting to listen ... "));
    state = radio.startReceive();
    if (state == RADIOLIB_ERR_NONE) {
      Serial.println(F("success!"));
    } else {
      Serial.print(F("failed, code "));
      Serial.println(state);
      while (true) { delay(10); }
    }
  #endif
}

void loop() {
  // check if the previous operation finished
  if(operationDone) {
    // reset flag
    operationDone = false;

    if(transmitFlag) {
      // the previous operation was transmission, listen for response
      // print the result
      if (transmissionState == RADIOLIB_ERR_NONE) {
        // packet was successfully sent
        Serial.println(F("transmission finished!"));

      } else {
        Serial.print(F("failed, code "));
        Serial.println(transmissionState);

      }

      // listen for response
      radio.startReceive();
      transmitFlag = false;

    } else {
      // the previous operation was reception
      // print data and send another packet
      uint8_t packet[32];
      int state = radio.readData(packet, sizeof(packet));

      if (state == RADIOLIB_ERR_NONE) {
        // packet was successfully received
        Serial.println(F("[SX1262] Received packet!"));
        // print RSSI (Received Signal Strength Indicator)
        Serial.print(F("[SX1262] RSSI:\t\t"));
        float RSSI = radio.getRSSI();
        Serial.print(RSSI);
        Serial.println(F(" dBm"));

        // print SNR (Signal-to-Noise Ratio)
        Serial.print(F("[SX1262] SNR:\t\t"));
        Serial.print(radio.getSNR());
        Serial.println(F(" dB"));
        //try to decrypt
        Serial.println(F("Decrypting..."));
        uint8_t iv_key[16];
        uint8_t ciphertext[16];
        if(sizeof(packet) < 32){
          Serial.println(F("Packet too short"));
          return;
        }
        memcpy(iv_key, packet, 16);
        memcpy(ciphertext, packet+16, 16);

        uint8_t decrypted[16];
        int decLen = aesLib.decrypt(ciphertext, 16, decrypted, aes_key, 128, iv_key);

        if(decLen <= 0){
          Serial.println(F("Decryption failed!"));
          return;
        }
        if(parseData(decrypted)){
          Serial.println(F("Packet parsed successfully"));
        }



      }

      // wait a second before transmitting again
      delay(1000);

      // send another one
      Serial.print(F("[SX1262] Sending another packet ... "));
      transmissionState = radio.startTransmit("LOCATION LOGGED");
      transmitFlag = true;
    }
  
  }
}
