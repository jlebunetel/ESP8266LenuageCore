/*
  ESP8266LenuageCore.h - Lenuage library for ESP8266 Arduino
*/

// ensure this library description is only included once
#ifndef ESP8266LenuageCore_h
#define ESP8266LenuageCore_h

// uncomment to activate debugging mode
//#define DEBUG 1

// include description files for other libraries used
#include <ESP8266LaboiteScreen.h>


// taille en mémoire, en octet ?
struct tile {
  uint8_t  _id;
  uint64_t _last_activity;
  uint8_t  _previous_id;
  Screen_32_16_2 _screen;
  uint8_t  _duration;
  bool     _exist;
  bool     _update;
};
typedef struct tile Tile;

#define TILES_MAX_NUMBER 50


// library interface description
class Lenuage {
  // user-accessible "public" interface
  public:
    Lenuage(char* server, char* apikey, char* fingerprint);
    String getBoite();
    String getTile(int id);
    void updateBuffer();
    void updateTile(int index);
    void printBuffer();
    void clearTile(int i);
    Screen getTileScreen(int id);
    int getNextTile(int id);
    int getTileDuration(int id);

  // library-accessible "private" interface
  private:
    char server[100];
    char apikey[36+1];
    char fingerprint[59+1];
    String url;
    Tile tiles[TILES_MAX_NUMBER];
};

#endif
