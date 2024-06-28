#include "Base32.h"

Base32 base32;

boolean tests()
{
  String encoded, decoded = "";
  byte* tempEncoded = NULL;
  byte* tempDecoded = NULL;

  //            encoding                                         decoding
  byte enc0[] = {};                                byte dec0[] = {};
  byte enc1[] = {'f'};                             byte dec1[] = {'M', 'Y', '=', '=', '=', '=', '=', '='}; // MY======
  byte enc2[] = {'f', 'o'};                        byte dec2[] = {'M', 'Z', 'X', 'Q', '=', '=', '=', '='}; // MZXQ====
  byte enc3[] = {'f', 'o', 'o'};                   byte dec3[] = {'M', 'Z', 'X', 'W', '6', '=', '=', '='}; // MZXW6===
  byte enc4[] = {'f', 'o', 'o', 'b'};              byte dec4[] = {'M', 'Z', 'X', 'W', '6', 'Y', 'Q', '='}; // MZXW6YQ=
  byte enc5[] = {'f', 'o', 'o', 'b', 'a'};         byte dec5[] = {'M', 'Z', 'X', 'W', '6', 'Y', 'T', 'B'}; // MZXW6YTB
  byte enc6[] = {'f', 'o', 'o', 'b', 'a', 'r'};    byte dec6[] = {'M', 'Z', 'X', 'W', '6', 'Y', 'T', 'B', 'O', 'I', '=', '=', '=', '=', '=', '='}; // MZXW6YTBOI======
  byte enc7[] = {0};                               byte dec7[] = {'A', 'A'};
  byte enc8[] = {0, 0};                            byte dec8[] = {'A', 'A', 'A', 'A'};
  byte enc9[] = {0, 0, 0};                         byte dec9[] = {'A', 'A', 'A', 'A', 'A'};

  byte enc10[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x0F, 0xD5, 0x9B, 0x8D};
  byte dec10[] = {0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 
                  0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 
                  0x41, 0x41, 0x41, 0x41, 0x42, 0x37, 0x4B, 0x5A, 0x58, 0x44, 0x49};

  if (base32.toBase32(enc0, sizeof(enc0), (byte*&)tempEncoded, true) == 0 &&
    memcmp(tempEncoded, dec0, sizeof(dec0)) == 0) {} else { return false; }
  free(tempEncoded);

  if (base32.fromBase32(dec0, sizeof(dec0), (byte*&)tempDecoded) == 0 &&
    memcmp(tempDecoded, enc0, sizeof(enc0)) == 0) {} else { return false; }
  free(tempDecoded);

  if (base32.toBase32(enc1, sizeof(enc1), (byte*&)tempEncoded, true) > 0 &&
    memcmp(tempEncoded, dec1, sizeof(dec1)) == 0) {} else { return false; }
  free(tempEncoded);

  if (base32.fromBase32(dec1, sizeof(dec1), (byte*&)tempDecoded) > 0 &&
    memcmp(tempDecoded, enc1, sizeof(enc1)) == 0) {} else { return false; }
  free(tempDecoded);

  if (base32.toBase32(enc2, sizeof(enc2), (byte*&)tempEncoded, true) > 0 &&
    memcmp(tempEncoded, dec2, sizeof(dec2)) == 0) {} else { return false; }
  free(tempEncoded);

  if (base32.fromBase32(dec2, sizeof(dec2), (byte*&)tempDecoded) > 0 &&
    memcmp(tempDecoded, enc2, sizeof(enc2)) == 0) {} else { return false; }
  free(tempDecoded);

  if (base32.toBase32(enc3, sizeof(enc3), (byte*&)tempEncoded, true) > 0 &&
    memcmp(tempEncoded, dec3, sizeof(dec3)) == 0) {} else { return false; }
  free(tempEncoded);

  if (base32.fromBase32(dec3, sizeof(dec3), (byte*&)tempDecoded) > 0 &&
    memcmp(tempDecoded, enc3, sizeof(enc3)) == 0) {} else { return false; }
  free(tempDecoded);

  if (base32.toBase32(enc4, sizeof(enc4), (byte*&)tempEncoded, true) > 0 &&
    memcmp(tempEncoded, dec4, sizeof(dec4)) == 0) {} else { return false; }
  free(tempEncoded);

  if (base32.fromBase32(dec4, sizeof(dec4), (byte*&)tempDecoded) > 0 &&
    memcmp(tempDecoded, enc4, sizeof(enc4)) == 0) {} else { return false; }
  free(tempDecoded);

  if (base32.toBase32(enc5, sizeof(enc5), (byte*&)tempEncoded, true) > 0 &&
    memcmp(tempEncoded, dec5, sizeof(dec5)) == 0) {} else { return false; }
  free(tempEncoded);

  if (base32.fromBase32(dec5, sizeof(dec5), (byte*&)tempDecoded) > 0 &&
    memcmp(tempDecoded, enc5, sizeof(enc5)) == 0) {} else { return false; }
  free(tempDecoded);

  if (base32.toBase32(enc6, sizeof(enc6), (byte*&)tempEncoded, true) > 0 &&
    memcmp(tempEncoded, dec6, sizeof(dec6)) == 0) {} else { return false; }
  free(tempEncoded);

  if (base32.fromBase32(dec6, sizeof(dec6), (byte*&)tempDecoded) > 0 &&
    memcmp(tempDecoded, enc6, sizeof(enc6)) == 0) {} else { return false; }
  free(tempDecoded);

  if (base32.toBase32(enc7, sizeof(enc7), (byte*&)tempEncoded, true) > 0 &&
    memcmp(tempEncoded, dec7, sizeof(dec7)) == 0) {} else { return false; }
  free(tempEncoded);

  if (base32.fromBase32(dec7, sizeof(dec7), (byte*&)tempDecoded) > 0 &&
    memcmp(tempDecoded, enc7, sizeof(enc7)) == 0) {} else { return false; }
  free(tempDecoded);

  if (base32.toBase32(enc8, sizeof(enc8), (byte*&)tempEncoded, true) > 0 &&
    memcmp(tempEncoded, dec8, sizeof(dec8)) == 0) {} else { return false; }
  free(tempEncoded);

  if (base32.fromBase32(dec8, sizeof(dec8), (byte*&)tempDecoded) > 0 &&
    memcmp(tempDecoded, enc8, sizeof(enc8)) == 0) {} else { return false; }
  free(tempDecoded);

  if (base32.toBase32(enc9, sizeof(enc9), (byte*&)tempEncoded, true) > 0 &&
    memcmp(tempEncoded, dec9, sizeof(dec9)) == 0) {} else { return false; }
  free(tempEncoded);

  if (base32.fromBase32(dec9, sizeof(dec9), (byte*&)tempDecoded) > 0 &&
    memcmp(tempDecoded, enc9, sizeof(enc9)) == 0) {} else { return false; }
  free(tempDecoded);

  if (base32.toBase32(enc10, sizeof(enc10), (byte*&)tempEncoded, true) > 0 &&
    memcmp(tempEncoded, dec10, sizeof(dec10)) == 0) {} else { return false; }
  free(tempEncoded);
  
  if (base32.fromBase32(dec10, sizeof(dec10), (byte*&)tempDecoded) > 0 &&
    memcmp(tempDecoded, enc10, sizeof(enc10)) == 0) {} else { return false; }
  free(tempDecoded);

  return true;
}

void setup()
{
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  if (tests() == true) 
  { // if tests passed, then on-board LED is blinking
    while (1)
    {
      state = !state;
      digitalWrite(13, state);
      delay(100);
    }
  }
  else
  { // if tests failed, then on-board LED is on
    digitalWrite(13, state); 
  }
}

void loop()
{
}


