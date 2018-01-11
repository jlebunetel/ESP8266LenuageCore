// Librairie à adapter en fonction de l'afficheur utilisé
#include <ESP8266LaboiteMAX72XX.h>
LedMatrixPanel afficheur;

// We need a WiFi connection
#include <ESP8266WiFi.h>
#include <WiFiManager.h>             //https://github.com/tzapu/WiFiManager

// Librairie pour générer les buffers
// à importer uniquement pour les screens personnalisés
#include <ESP8266LaboiteScreen.h>
Screen_32_16_2 welcome;

// la librairie Redgick !
#include <ESP8266LenuageCore.h>

Lenuage lenuage(
  "http://dev.laboite.pro",                 // server url
  "ed49d498-b9b8-44b6-bf6e-ace527c6f3b9",   // api key
  // the server ssl fingerprint is needed for a HTTPS connection
  // get it using the following command in a terminal:
  // echo | openssl s_client -connect redgick.com:443 |& openssl x509 -fingerprint -noout
  "09:02:E4:52:CA:EE:45:EB:EC:93:4B:46:91:63:87:62:81:BC:93:78"
);


void setup() {
  // initialisation de la liaison série
  Serial.begin(115200);

  // display initialization
  afficheur.init();
  afficheur.intensity(0);

  // message d'accueil
  welcome.print(0, 2, "la");
  welcome.print(1, 9, "boite");
  welcome.print(28, 1, "2", FONT_4X6);
  afficheur.display(welcome.getBuffer());

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
  afficheur.intensity(lenuage.getTileBrightness(current_id));
  afficheur.display(lenuage.getTileScreen(current_id).getBuffer());

  unsigned long top = millis();

  // mise à jour du buffer des tuiles
  lenuage.updateBuffer();

  // affichage du buffer
  //lenuage.printBuffer();

  // on attend avant d'afficher la tuile suivante
  int delai = lenuage.getTileDuration(current_id);
  while (millis() < top + delai) {
    delay(50);
  }

  // quelle est la tuile suivante ?
  current_id = lenuage.getNextTile(current_id);
}
