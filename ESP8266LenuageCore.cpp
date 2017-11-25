/*
  ESP8266LenuageCore.h - Lenuage library for ESP8266 Arduino
*/

// include Arduino core library
#include "Arduino.h"

// include this library's description file
#include "ESP8266LenuageCore.h"

// include description files for other libraries used
#include "ESP8266HTTPClient.h"
#include <ArduinoJson.h>


// Constructor /////////////////////////////////////////////////////////////////
// Function that handles the creation and setup of instances

Lenuage::Lenuage(char* server, char* apikey, char* fingerprint = "") {
  // initialize this instance's variables
  strcpy(this->server, server);
  strcpy(this->apikey, apikey);
  strcpy(this->fingerprint, fingerprint);
  this->url = String(server) + "/boites/" + String(apikey) + "/";

  // do whatever is required to initialize the library
  // initialisation du buffer
  for (int i = 0; i < TILES_MAX_NUMBER; i++) {
    clearTile(i);
  }
};

// Public Methods //////////////////////////////////////////////////////////////
// Functions available in ESP8266LenuageCore sketches, this library, and other libraries

String Lenuage::getBoite() {
  String json; // buffer

  HTTPClient http;
  if (server[4] == 's') {
    http.begin(url, fingerprint);
  }
  else {
    http.begin(url);
  }
  http.addHeader("Accept", "application/json");

  int httpCode = http.GET();
  if (httpCode > 0 ){
    // HTTP header has been send and Server response header has been handled

    // file found at server
    if (httpCode == HTTP_CODE_OK) {
      json = http.getString();
    }
  }
  http.end();

  return json;
};

String Lenuage::getTile(int id) {
  String json; // buffer
  String urlTile = url + "tiles/" + String(id) + "/";
  Serial.println(urlTile); // debug

  HTTPClient http;
  if (server[4] == 's') {
    http.begin(urlTile, fingerprint);
  }
  else {
    http.begin(urlTile);
  }
  http.addHeader("Accept", "application/json");

  int httpCode = http.GET();
  if (httpCode > 0 ){
    // HTTP header has been send and Server response header has been handled

    // file found at server
    if (httpCode == HTTP_CODE_OK) {
      json = http.getString();
    }
  }
  http.end();

  return json;
};

void Lenuage::updateBuffer() {
  // we create a buffer
  String json;

  // then we get a records collection
  json = getBoite();

  // Serial.println(json); // debug

  // Don’t reuse the same JsonBuffer
  // During is lifetime a JsonBuffer growth until it’s discarded.
  // If you try to reuse the same instance several time, it will rapidly get full.
  StaticJsonBuffer<1024> jsonBuffer;

  JsonObject& root = jsonBuffer.parseObject(json); // JsonObject alloué en mémoire dynamiquement ?
  if (!root.success()) {
    Serial.println("parseObject() failed");  // debug
    return;
  }

  // on met le flag "exist" à "false"
  for (int i = 0; i < TILES_MAX_NUMBER; i++) {
    tiles[i]._exist = false;
  }

  // pour chaque tuile on renseigne l'id de la tuile précédente
  uint8_t previous_id = 0;

  // on explore l'arbre json
  JsonArray& data = root["tiles"];
  for (auto tile : data) {
    JsonObject& tile_data = tile;

    uint8_t id = 0;
    uint32_t last_activity = 0;

    if (tile_data.containsKey("id")) {
      id = int(tile_data["id"]);
    }
    else {
      continue; // problème de lecture, on passe à la prochaine valeur de la boucle for
    }

    if (tile_data.containsKey("last_activity")) {
      last_activity = long(tile_data["last_activity"]);
    }
    else {
      continue; // problème de lecture, on passe à la prochaine valeur de la boucle for
    }

    // la tuile est-elle présente dans le buffer ? et si oui, à quel indice ?
    bool exist = false;
    uint8 index = 0;
    for (int i = 0; i < TILES_MAX_NUMBER; i++) {
      if (tiles[i]._id == id) {
        exist = true;
        index = i;
        break;
      }
    }

    if (exist) {
      // la tuile existe
      // est-elle à jour ?
      if (tiles[index]._last_activity == last_activity) {
        // la tuile est à jour
        // ... rien à faire
      }
      else {
        // la tuile n'est pas à jour
        // on active le flag update
        tiles[index]._update = true;

        // ATTENTION
        // SUPPRIMER APRES AVOIR ECRIT LA METHODE UPDATE
        tiles[index]._last_activity = last_activity;
        // ATTENTION
      }
    }
    else {
      // la tuile n'existe pas encore

      // on recherche la premiere place libre dans le buffer (s'il en reste)
      bool buffer_overflow = true;
      for (int i = 0; i < TILES_MAX_NUMBER; i++) {
        if (tiles[i]._id == 0) {
          // une case est libre !
          buffer_overflow = false;
          index = i;
          break;
        }
      }

      // si une place est disponible
      if (not buffer_overflow) {
        // on crée la tuile
        tiles[index]._id = id;
        tiles[index]._last_activity = last_activity;

        // on la met à jour
        tiles[index]._update = true;
      }
      else {
        // le buffer est plein
        // que fait-on ?
      }
    }

    // on active le flag "exist"
    tiles[index]._exist = true;
    tiles[index]._previous_id = previous_id;
    previous_id = id;
    // remarque : la première tuile est la seule à avoir un previous_id==0 et id!=0
  }

  // on libère les tuiles pour lesquelles le flag "exist" est à "false"
  // cela correspond aux tuiles supprimées depuis la précédente mise à jour du buffer
  for (int i = 0; i < TILES_MAX_NUMBER; i++) {
    if (tiles[i]._id != 0 and (not tiles[i]._exist)) {
      Serial.print("suppression de la tuile "); // debug
      Serial.println(tiles[i]._id); // debug
      clearTile(i);
    }
  }

  // mise à jour des tuiles dont le flag "update" est true
  for (int i = 0; i < TILES_MAX_NUMBER; i++) {
    if (tiles[i]._update) {
      updateTile(i);
    }
  }
};

void Lenuage::updateTile(int index) {
  Serial.print("mise a jour de la tuile "); // debug
  Serial.println(tiles[index]._id); // debug

  // on efface la tuile
  for (int i = 0; i < 64; i++) {
    tiles[index]._screen.clear();
  }

  // we create a buffer
  String json;

  // then we get a records collection
  json = getTile(tiles[index]._id);

  // Serial.println(json); // debug

  // Don’t reuse the same JsonBuffer
  // During is lifetime a JsonBuffer growth until it’s discarded.
  // If you try to reuse the same instance several time, it will rapidly get full.
  StaticJsonBuffer<1024> jsonBuffer;

  JsonObject& root = jsonBuffer.parseObject(json); // JsonObject alloué en mémoire dynamiquement ?
  if (!root.success()) {
    Serial.println("parseObject() failed");  // debug
    return;
  }

  if (root.containsKey("duration")) {
    //Serial.println(int(root["duration"]));
    tiles[index]._duration = int(root["duration"]);
  }

  if (root.containsKey("brightness")) {
    //Serial.println(int(root["brightness"]));
    tiles[index]._brightness = int(root["brightness"]);
  }

  // on explore l'arbre json
  JsonArray& data = root["items"];
  for (auto item : data) {
    JsonObject& item_data = item;

    int scrolling = 0;
    if (item_data.containsKey("scrolling")) {
      scrolling = int(item_data["scrolling"]);
    }

    int x = 0;
    if (item_data.containsKey("x")) {
      x = int(item_data["x"]);
    }

    int y = 0;
    if (item_data.containsKey("y")) {
      y = int(item_data["y"]);
    }

    int width = 8;
    if (item_data.containsKey("width")) {
      width = int(item_data["width"]);
    }

    int height = 8;
    if (item_data.containsKey("height")) {
      height = int(item_data["height"]);
    }

    String content = "";
    if (item_data.containsKey("content")) {
      content = item_data["content"].asString();
    }

    if (item_data.containsKey("type")) {
      String type = item_data["type"].asString();
      if (type.equals("text")) {
        tiles[index]._screen.print(x, y, content);
      }
      else {
        if (type.equals("icon")) {
          tiles[index]._screen.drawBitmap(x, y, Bitmap(width, height, content));
        }
        else {
          Serial.println("type inconnu ..."); // debug
          return;
        }
      }
    }
  }

  // mise à jour effectuée !
  tiles[index]._update = false;
};

void Lenuage::printBuffer() {
  for (int i = 0; i < TILES_MAX_NUMBER; i++) {
    Serial.print("index ");
    Serial.print(i);
    Serial.print(" | id ");
    Serial.print(tiles[i]._id);
    Serial.print(" | last_activity ");
    Serial.print(long(tiles[i]._last_activity));
    Serial.print(" | duration ");
    Serial.print(long(tiles[i]._duration));
    Serial.print(" | previous_id ");
    Serial.print(tiles[i]._previous_id);
    Serial.print(" | update ");
    Serial.print(tiles[i]._update);
    Serial.print(" | exist ");
    Serial.println(tiles[i]._exist);
    Serial.print(" | brightness ");
    Serial.println(tiles[i]._brightness);
  }
};

void Lenuage::clearTile(int i) {
  tiles[i]._id = 0;
  tiles[i]._last_activity = 0;
  tiles[i]._previous_id = 0;
  tiles[i]._update = 0;
};

Screen Lenuage::getTileScreen(int id) {
  for (int i = 0; i < TILES_MAX_NUMBER; i++) {
    if (tiles[i]._id == id) {
      // on a trouvé la tuile !
      return(tiles[i]._screen);
    }
  }
  // par defaut on renvoie le premier buffer de la file
  return tiles[0]._screen;
};

int Lenuage::getNextTile(int id) {
  // exemple id=23
  // quelle est la tuile dont le previous_id == 23 ?
    // c'est la tuile id == 24
  for (int i = 0; i < TILES_MAX_NUMBER; i++) {
    if (tiles[i]._previous_id == id and tiles[i]._exist) {
      // on a trouvé la tuile !
      return(tiles[i]._id);
    }
  }
  // si on n'a pas trouvé, on affiche la première tuile du buffer
  // celle dont _previous_id == 0 et _exist == 1
  for (int i = 0; i < TILES_MAX_NUMBER; i++) {
    if (tiles[i]._previous_id == 0 and tiles[i]._exist) {
      // on a trouvé la tuile !
      return(tiles[i]._id);
    }
  }
  // par défaut on renvoie la première tuile du buffer
  return(tiles[0]._id);
};

int Lenuage::getTileDuration(int id) {
  for (int i = 0; i < TILES_MAX_NUMBER; i++) {
    if (tiles[i]._id == id) {
      // on a trouvé la tuile !
      return(tiles[i]._duration * 1000);
    }
  }
  // par defaut on renvoie 5 secondes
  return 5000;
};

int Lenuage::getTileBrightness(int id) {
  for (int i = 0; i < TILES_MAX_NUMBER; i++) {
    if (tiles[i]._id == id) {
      // on a trouvé la tuile !
      return(tiles[i]._brightness);
    }
  }
  // par defaut on renvoie la valeur minimale
  return 0;
};


// Private Methods /////////////////////////////////////////////////////////////
// Functions only available to other functions in this library
