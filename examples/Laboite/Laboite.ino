// Librairie à adapter en fonction de l'afficheur utilisé
#include <ESP8266LaboiteMAX72XX.h>
LedMatrixPanel afficheur;

// We need a WiFi connection
#include <ESP8266WiFi.h>
#include <WiFiManager.h>             //https://github.com/tzapu/WiFiManager

// la librairie Redgick !
#include <ESP8266LenuageCore.h>

Lenuage lenuage(
  "http://dev.laboite.pro",                 // server url
  "",   // api key
  // the server ssl fingerprint is needed for a HTTPS connection
  // get it using the following command in a terminal:
  // echo | openssl s_client -connect redgick.com:443 |& openssl x509 -fingerprint -noout
  "09:02:E4:52:CA:EE:45:EB:EC:93:4B:46:91:63:87:62:81:BC:93:78"
);

uint8_t welcome[64] = {0b00000000,  0b00000000, 0b00000000, 0b00000000,
                       0b01000000, 0b00000000, 0b00000000, 0b00000000,
                       0b01000000, 0b00000000, 0b00000000, 0b00000000,
                       0b01000001, 0b11000000, 0b00000000, 0b00000000,
                       0b01000010, 0b01000000, 0b00000000, 0b00000000,
                       0b01000010, 0b11000000, 0b00000000, 0b00000000,
                       0b01111001, 0b01000000, 0b00000000, 0b00000000,
                       0b00000000, 0b00000000, 0b00000000, 0b00000000,
                       0b00000000, 0b00000000, 0b00000000, 0b00000000,
                       0b01110000, 0b00001000, 0b10000000, 0b00000000,
                       0b01001000, 0b00000001, 0b11000000, 0b00000000,
                       0b01110001, 0b10011000, 0b10000110, 0b00000000,
                       0b01001010, 0b01001000, 0b10001011, 0b00000000,
                       0b01001010, 0b01001000, 0b10001100, 0b00000000,
                       0b01110001, 0b10011100, 0b01100110, 0b00000000,
                       0b00000000, 0b00000000, 0b00000000, 0b00000000,
                      };


void setup() {
  // initialisation de la liaison série
  Serial.begin(115200);

  // display initialization
  afficheur.init();
  afficheur.intensity(0);
  afficheur.display(welcome); // message d'accueil

  // we start by connecting to a WiFi network
  WiFiManager wifiManager;
  wifiManager.autoConnect("laboite");

  // mise à jour du buffer des tuiles
  lenuage.updateBuffer();
}

int current_id = lenuage.getNextTile(0);

void loop() {
  // on affiche une tuile
  Serial.print("on affiche la tuile : ");
  Serial.println(current_id);
  afficheur.display(lenuage.getTileBuffer(current_id));

  unsigned long top = millis();

  // mise à jour du buffer des tuiles
  lenuage.updateBuffer();

  // affichage du buffer
  //lenuage.printBuffer();

  // on attend avant d'afficher la tuile suivante
  int delai = lenuage.getTileDuration(current_id);
  while(millis() < top + delai) {
    delay(50);
  }

  // quelle est la tuile suivante ?
  current_id = lenuage.getNextTile(current_id);
}
