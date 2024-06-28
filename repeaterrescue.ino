#include <HardwareSerial.h>
#include "src/tncattach/KISS.h"
#include <EEPROM.h>
#include <SimpleHOTP.h>
#include "src/ax25beacon/ax25.h"
#include "src/Base32/Base32.h"
#include "src/QRCode/src/qrmcode.h"
#include <WiFi.h>
#include <esp_wifi.h>


//
// CONFIG
//

#define CALLSIGN "VK4XSS-3" // with SSID. Call sign must be no larger than 6 chars and ssid 0 to 15
#define PATH_1 "WIDE1-1" // alternative is null
#define PATH_2 NULL
#define SYMBOL_TABLE "/" // primary table
#define SYMBOL "r" // repeater
#define BEACON_FREQUENCY 900 // in seconds

// Latitude is expressed as a fixed 8-character field, in degrees and decimal minutes (to two decimal places), followed by the letter N for north or S for south.
// Latitude degrees are in the range 00 to 90. Latitude minutes are expressed as whole minutes and hundredths of a minute, separated by a decimal point.
// For example:
// 4903.50N is 49 degrees 3 minutes 30 seconds north.
// In generic format examples, the latitude is shown as the 8-character string ddmm.hhN (i.e. degrees, minutes and hundredths of a minute north).
// Longitude is expressed as a fixed 9-character field, in degrees and decimal minutes (to two decimal places), followed by the letter E for east or W for west.
// Longitude degrees are in the range 000 to 180. Longitude minutes are expressed as whole minutes and hundredths of a minute, separated by a decimal point.
// For example:
// 07201.75W is 72 degrees 1 minute 45 seconds west.
// In generic format examples, the longitude is shown as the 9-character string dddmm.hhW (i.e. degrees, minutes and hundredths of a minute west).
#define BEACON_LATITUDE "3749.22S"
#define BEACON_LONGITUDE "14500.67E"

#define PULSE_LENGTH_MS 1000 // how long the relays should remain on

#define TNC_BAUD_RATE 4800
#define TNC_SERIAL_MODE SERIAL_8N1

#define MAX_TRIES 5 // note that the receiver might get the same message multiple times, so might lock after a successful command
#define LOCK_RESET_SECONDS 300 // 5 minutes

#define IGNORE_TIME_SECONDS 30 // after a successful command we ignore messages for 30 seconds to prevent lock outs

#define TRIGGER_MODE_SWITCH "\r\ncmd:" // what string to look for to set kiss mode
#define TRIGGER_MODE_ACTION "KISS ON\rRESTART\r"










// shouldn't need to change below unless using a different platform

//
// END OF CONFIG
//

// hard config
// setup uart 2 (gpio 16/17)
HardwareSerial SerialPort(2);
byte relays[] = {2, 15, 5, 4};
#define BUZZER_PIN 18
#define EEPROM_SIZE 128 // bytes
#define LED_BUILTIN 12 
#define KEY_SIZE 64


// code starts here

// this should all remain the same - probably
#define BEACON_TO_CALL "APRRES"
uint64_t currentCounter;
const int TONE_PWM_CHANNEL = 0;
int locked = 0;
int failed_counter = 0;
unsigned long locked_time;

int ignore = 0;
unsigned long ignore_time;

int trigger_index = 0;
char trigger_mode_switch_word[] = TRIGGER_MODE_SWITCH;

Key key;
Base32 base32;

void send_packet(char *data){
  int out_len = kiss_write_frame("", 0);
  SerialPort.write(write_buffer, out_len); // send data

  int ax_25_len = ax25_frame(CALLSIGN,BEACON_TO_CALL,PATH_1,PATH_2,data)-2;
  out_len = kiss_write_frame((char*)ax25_frame_out, ax_25_len);
  SerialPort.write(write_buffer, out_len); // send data

  
  out_len = kiss_write_frame("", 0);
  SerialPort.write(write_buffer, out_len); // send data
  Serial.println("send packet");
}

void setup() {
  Serial.begin(115200);
  // led ?
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  // Beep
  ledcAttachPin(BUZZER_PIN, TONE_PWM_CHANNEL);

  ledcWriteNote(TONE_PWM_CHANNEL, NOTE_C, 3);  
  delay(50);
  ledcWrite(TONE_PWM_CHANNEL, 0);

  delay(200);

  ledcWriteNote(TONE_PWM_CHANNEL, NOTE_C, 3);  
  delay(50);
  ledcWrite(TONE_PWM_CHANNEL, 0);

  ledcDetachPin(BUZZER_PIN);

  // check if we have a key and counter yet
  delay(1000); // give serial a chance to connect after programming
  EEPROM.begin(EEPROM_SIZE);
  char magic = EEPROM.readChar(0);
  uint8_t tmp[KEY_SIZE];
  if (magic != 'R'){
    delay(2000);
    Serial.println("Did not find magic in EEPROM - assuming new setup");
    EEPROM.writeULong64(8, 0);
    EEPROM.writeChar(0, 'R');
    
    key = Key(64); // create new key

    
    key.exportToArray(tmp);

    EEPROM.writeBytes(16, &tmp, KEY_SIZE);
    EEPROM.commit();
  }

  // HOTP
  currentCounter = EEPROM.readULong64(8); // read the current counter
  Serial.print("Current HOTP Counter: ");
  Serial.println(currentCounter);

  // load the key back out of eeprom to validate
  EEPROM.readBytes(16,&tmp, KEY_SIZE);
  key = Key(tmp, KEY_SIZE);

  byte* tempEncoded = NULL;
  int base32_len = base32.toBase32(tmp, sizeof(tmp), (byte*&)tempEncoded, false);

  uint8_t baseMac[6];
  esp_err_t ret = esp_efuse_mac_get_default(baseMac);

  char url[500];
  snprintf(url, 500, "otpauth://hotp/RepeaterRescue%02x%02x%02x%02x%02x%02x?secret=%.*s&counter=%lld",
    baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5],
    base32_len, tempEncoded,
    currentCounter
  );


  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(8)];
  qrcode_initText(&qrcode, qrcodeData, 8, ECC_LOW, url);

  // Top quiet zone
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t y = 0; y < qrcode.size; y = y + 2) {
      // Left quiet zone
      Serial.print("        ");
      // Each horizontal module
      for (uint8_t x = 0; x < qrcode.size; x++) {
          if (qrcode_getModule(&qrcode, x, y) && qrcode_getModule(&qrcode, x, y+1)) Serial.print("\u2588");
          if (qrcode_getModule(&qrcode, x, y) && !qrcode_getModule(&qrcode, x, y+1)) Serial.print("\u2580");
          if (!qrcode_getModule(&qrcode, x, y) && qrcode_getModule(&qrcode, x, y+1)) Serial.print("\u2584");
          if (!qrcode_getModule(&qrcode, x, y) && !qrcode_getModule(&qrcode, x, y+1)) Serial.print(" ");
      }
      Serial.println();
  }
  // Bottom quiet zone
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println();

  free(tempEncoded);
  Serial.println(url);



  // relay setup
  pinMode(relays[0], OUTPUT);
  pinMode(relays[1], OUTPUT);
  pinMode(relays[2], OUTPUT);
  pinMode(relays[3], OUTPUT);
  digitalWrite(relays[0], LOW);
  digitalWrite(relays[1], LOW);
  digitalWrite(relays[2], LOW);
  digitalWrite(relays[3], LOW);

  Serial.println(F("Start. Waiting 3 seconds for TNC"));
  delay(3000);
  SerialPort.begin(TNC_BAUD_RATE, TNC_SERIAL_MODE, 16, 17);
  Serial.println("Started hardware serial.");
  
  send_latest_hotp();



}

void send_latest_hotp(){
  char test_packet[50];
  snprintf(test_packet,50,"=" BEACON_LATITUDE SYMBOL_TABLE BEACON_LONGITUDE SYMBOL "HOTP=%lld LOCK=%d",
    currentCounter,
    locked
    );
  Serial.println(test_packet);
  send_packet(test_packet);
}

void command_feedback(char* to_call, char* message){
  Serial.println(message);
  char test_packet[50];
  snprintf(test_packet,50,":%-9s:%s HOTP=%lld",
    to_call,
    message,
    currentCounter
    );
  Serial.println(test_packet);

  int out_len = kiss_write_frame("", 0);
  SerialPort.write(write_buffer, out_len); // send data

  int ax_25_len = ax25_frame(CALLSIGN,BEACON_TO_CALL,PATH_1,PATH_2,test_packet)-2;
  out_len = kiss_write_frame((char*)ax25_frame_out, ax_25_len);
  SerialPort.write(write_buffer, out_len); // send data

  
  out_len = kiss_write_frame("", 0);
  SerialPort.write(write_buffer, out_len); // send data
  Serial.println("sent feedback");
}

void ack(char* to_call, char* message){
  Serial.println(message);
  char test_packet[50];
  snprintf(test_packet,50,":%-9s:ack%s",
    to_call,
    message
    );
  Serial.println(test_packet);

  int out_len = kiss_write_frame("", 0);
  SerialPort.write(write_buffer, out_len); // send data

  int ax_25_len = ax25_frame(CALLSIGN,BEACON_TO_CALL,PATH_1,PATH_2,test_packet)-2;
  out_len = kiss_write_frame((char*)ax25_frame_out, ax_25_len);
  SerialPort.write(write_buffer, out_len); // send data

  
  out_len = kiss_write_frame("", 0);
  SerialPort.write(write_buffer, out_len); // send data
  Serial.println("sent ack");
}

void loop() {
  digitalWrite(LED_BUILTIN, LOW);
  if (SerialPort.available())
  {
    digitalWrite(LED_BUILTIN, HIGH);
    char data = SerialPort.read();

    // check if we might not be in kiss mode
    if (data == trigger_mode_switch_word[trigger_index]){
      trigger_index++;
      if (trigger_index >= sizeof(trigger_mode_switch_word) - 1){ // we don't want to match on the null
        Serial.println("Detected we might not be in kiss mode, attempting switch over");
        trigger_index = 0;
        SerialPort.write('\r'); // assuming kiss mode is enabled and saved to battery
        delay(500);
        SerialPort.println(TRIGGER_MODE_ACTION);
        delay(500);
        return;
      }
    } else {
      trigger_index = 0;
    }

    Serial.print(data);
    int len = kiss_serial_read(data);
    if (len){
      Serial.println();
      Serial.println("Kiss packet");
      frame_buffer[len] = {0};
      String str = (char*)frame_buffer;
      char call_match[11];
      snprintf(call_match, 11, "%-9s:", CALLSIGN);

      // try to determine source callsign (or read some random memory - who knows, it's C)
      char from_call[10]; // no f-calls allowed
      int from_call_index;
      for (from_call_index=0; from_call_index<6; from_call_index++){ // +7 is offset to the source callsign
        if (frame_buffer[from_call_index+7]>>1 == 0x20) break; // skip spaces
        from_call[from_call_index] = frame_buffer[from_call_index+7]>>1;
      }
      int from_call_ssid = (frame_buffer[from_call_index+7] >> 1) & 15;
      from_call[from_call_index++]='-';
      itoa(from_call_ssid, from_call + from_call_index++, 10);
      // decode ssid
      Serial.print("From Call: ");
      Serial.println(from_call);


      int match = str.indexOf(call_match);
      if (match > 0){ // message for us
        String check_ack = str.substring(str.length()-6);
        int match_ack = check_ack.lastIndexOf("{");
        if (match_ack >= 0 ){ // ack should have two or more chars
          Serial.print("found ack type: ");
          String ack_reply = check_ack.substring(match_ack+1);
          
          char ack_reply_c[10];
          ack_reply.toCharArray(ack_reply_c, 10);

          Serial.println(ack_reply);
          ack(from_call, ack_reply_c);
        }

        String command = str.substring(match+10, match+10+8); // 10 = length of the callsign + msg seperator. 8 = relay id, relay state, hotp
        Serial.print("Message Command: ");
        Serial.println(command);
        if (command.length() != 8){
          command_feedback(from_call,"Incorrect command length");
          return;
        }



        // TODO add HOTP checking

        String token = command.substring(2, 6+8);
        Serial.print("Token provided: ");
        Serial.println(token);

        if (!(
          isDigit(token.charAt(0)) &&
          isDigit(token.charAt(1)) &&
          isDigit(token.charAt(2)) &&
          isDigit(token.charAt(3)))
          ){
            command_feedback(from_call,"Token is not a number");
        }

        int provided_token = token.toInt();
        
        SimpleHOTP hotp(key, currentCounter);
        Serial.println(provided_token);

        if (locked){
          command_feedback(from_call, "Currently locked out");
          return;
        }
        if (ignore){
          command_feedback(from_call, "Currently ignoring messages");
          return;
        }

        int check_token = hotp.validate(provided_token);
        if (check_token){
          currentCounter = check_token;
          EEPROM.writeULong64(8, check_token);
          EEPROM.commit();
          Serial.println("token provided is correct");
          Serial.print("new counter = ");
          Serial.println(check_token);
          send_latest_hotp();
          failed_counter = 0;
          ignore = 1; // ignore messages for some time to prevent duplicates
          ignore_time = millis();
        } else {
          failed_counter++;
          command_feedback(from_call, "Token incorrect");
          Serial.print("Failed attempts: ");
          Serial.println(failed_counter);

          if (failed_counter >= MAX_TRIES){
            command_feedback(from_call, "Locked out");
            locked_time = millis();
            locked = 1;
          }
          return;
        }


        int relay;
        int relay_command;
        if (isDigit(command.charAt(0))){ // is the first char a number
          relay = command.charAt(0) - 48; // convert ascii to number
        } else {
          command_feedback(from_call,"relay not a number");
          return;
        }

        if (relay == 0) {
          command_feedback(from_call,"Throwing away token");
          return;
        }

        if (!(relay >= 1 && relay <= 4)){
            command_feedback(from_call,"relay not 1,2,3 or 4");
            return;
        }

        if (isDigit(command.charAt(1))){ // is the first char a number
          relay_command = command.charAt(1) - 48; // convert ascii to number
        } else {
          command_feedback(from_call,"command not a number");
          return;
        }
        
        if (!(relay_command >= 0 && relay_command <= 2)){
            command_feedback(from_call,"command should be 0,1,2");
            return;
        }


        Serial.print("Got relay: ");
        Serial.println(relay);
        Serial.print("Got command: ");
        Serial.println(relay_command);

        relay--; // relays are 0 indexed
        
        digitalWrite(relays[relay], relay_command);
        
        if (relay_command == 2){ // handle the pulse option
          delay(PULSE_LENGTH_MS);
          digitalWrite(relays[relay], 0); // 0 indexed
        }

        command_feedback(from_call,"Executed command");

      }
    }
    
  }

  // run the periodic functions
  check_t_beacon();
  check_locked();
  check_ignore_time();
}

unsigned long lastBeacon = 0;
void check_t_beacon() {
	if ( millis() - lastBeacon < BEACON_FREQUENCY * 1000 ) return; // Not enough time has passed, return
  send_latest_hotp();
  lastBeacon = millis();
}

void check_locked() {
  if (!locked) return; // not locked - nothing to do
	if ( millis() - locked_time < LOCK_RESET_SECONDS * 1000 ) return; // Not enough time has passed, return
  locked = 0;
  failed_counter = 0;
  Serial.println("Unlocked");
}

void check_ignore_time() {
  if (!ignore) return; // not locked - nothing to do
	if ( millis() - ignore_time < IGNORE_TIME_SECONDS * 1000 ) return; // Not enough time has passed, return
  ignore = 0;
  Serial.println("Disabling ignore");
}