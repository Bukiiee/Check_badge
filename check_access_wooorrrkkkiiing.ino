#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <regex>
#include <Arduino_JSON.h>

#define URL "http://192.168.63.173:5000/test"

#define RST_PIN D6
#define SS_PIN D5
#define GREEN_LED_PIN D2  
#define RED_LED_PIN D1 

const char* ssid = "POCOPHONE";
const char* password = "Matis2004";
// Déclaration et initialisation du tableau d'octets
byte authorizedUid[4] = {0};

MFRC522 mfrc522(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(9600);
  while (!Serial);
  SPI.begin();
  mfrc522.PCD_Init();
  delay(4);
  mfrc522.PCD_DumpVersionToSerial();
  Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));

  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);

  delay(1000);

  // WiFi.mode(WIFI_STA); //Optional
  WiFi.begin(ssid, password);
  Serial.println("\nConnecting");

  while(WiFi.status() != WL_CONNECTED){
      Serial.print(".");
      delay(500);
  }

  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  HTTPClient http;

  http.begin(URL);
  int code = http.GET();

  if (code == HTTP_CODE_OK) {
    String resp = http.getString();

    // Convertir la chaîne String en std::string
    std::string respStr = resp.c_str();

    // Utilisation d'une expression régulière pour extraire l'UID
    std::regex uidRegex("\\{0x([[:xdigit:]]+), 0x([[:xdigit:]]+), 0x([[:xdigit:]]+), 0x([[:xdigit:]]+)\\}");
    std::smatch match;

    std::vector<std::vector<byte>> uidArray; // Tableau pour stocker les valeurs extraites

    std::sregex_iterator it(respStr.begin(), respStr.end(), uidRegex);
    std::sregex_iterator end;

    while (it != end) {
      std::vector<byte> uidBytes;

      for (size_t i = 1; i < it->size(); i++) {
        String hexValue = it->str(i).c_str();
        byte byteValue = strtol(hexValue.c_str(), nullptr, 16);
        uidBytes.push_back(byteValue);
      }

      uidArray.push_back(uidBytes);
      ++it;
    }

    if (!mfrc522.PICC_IsNewCardPresent()) {
      return;
    }

    if (!mfrc522.PICC_ReadCardSerial()) {
      return;
    }

    // Comparer l'UID du badge scanné avec ceux récupérés depuis le web
    for (const auto& authorizedUid : uidArray) {
      if (compareUid(mfrc522.uid.uidByte, authorizedUid.data(), mfrc522.uid.size)) {
        digitalWrite(GREEN_LED_PIN, HIGH);
        Serial.println(F("Badge autorisé. LED verte allumée."));
        delay(1000);
        digitalWrite(GREEN_LED_PIN, LOW);
        break;  // Sortir de la boucle dès qu'un UID autorisé est trouvé
      } else {
        digitalWrite(RED_LED_PIN, HIGH);
        Serial.println(F("Badge non autorisé. LED rouge allumée."));
        delay(1000);
        digitalWrite(RED_LED_PIN, LOW);
        break;
      }
    }

    http.end();
    delay(5000);
  }
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

// Fonction pour comparer deux tableaux d'octets (UID)
bool compareUid(const byte* uid1, const byte* uid2, byte size) {
  for (byte i = 0; i < size; i++) {
    if (uid1[i] != uid2[i]) {
      return false;
    }
  }
  return true;
}
