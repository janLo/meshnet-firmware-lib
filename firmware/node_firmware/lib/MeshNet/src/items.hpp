#ifndef _ITEMS_HPP_
#define _ITEMS_HPP_

#include <Arduino.h>

class Message;

#define MAX_ITEMS 16
#define MAX_ID_LEN 16

typedef enum {
  bin_switch = 0,
  bin_sensor = 1,
  analog_sensor = 3,
  one_wire = 4,
  rgb_lamp = 5,
  dimmer = 6,
  dht_sensor = 7
} items_t;

class Item {
  char item_id[MAX_ID_LEN + 1];

protected:
  void setId(const char *id);

public:
  Item();

  virtual void setState(Message *state) = 0;
  virtual void getState(Message *state) = 0;
  virtual bool hasChanged();
};

class BinSwitch : public Item {
  bool value;
  uint8_t pin;

public:
  BinSwitch(Message *msg);

  void setState(Message *state);
  void getState(Message *state);
  bool hasChanged();
};

class BinSensor : public Item {
  uint8_t pin;
  bool value;

public:
  BinSensor(Message *msg);

  void setState(Message *state);
  void getState(Message *state);
  bool hasChanged();
};

class AnalogSensor : public Item {
  uint8_t pin;
  int16_t value;
  uint16_t delta;

public:
  AnalogSensor(Message *msg);

  void setState(Message *state);
  void getState(Message *state);
  bool hasChanged();
};

class OneWire : public Item {
  uint8_t pin;

public:
  OneWire(Message *msg);

  void setState(Message *state);
  void getState(Message *state);
  bool hasChanged();
};

class Dimmer : public Item {
  unsigned value;
  uint8_t pin;

public:
  Dimmer(Message *msg);

  void setState(Message *state);
  void getState(Message *state);
};

class RGBLamp : public Item {
  unsigned red;
  unsigned green;
  unsigned blue;

  uint8_t red_pin;
  uint8_t green_pin;
  uint8_t blue_pin;

public:
  RGBLamp(Message *msg);

  void setState(Message *state);
  void getState(Message *state);
};

class DHTSensor : public Item {
  uint8_t pin;

  unsigned temp;
  unsigned temp_delta;
  unsigned humidity;
  unsigned humidity_delta;

public:
  DHTSensor(Message *msg);

  void setState(Message *state);
  void getState(Message *state);
  bool hasChanged();
};

class ItemRegistry {
  uint8_t item_cnt;
  Item *item_list[MAX_ITEMS];
  bool update_available[MAX_ITEMS];

public:
  ItemRegistry();

  int configure(Message *configMessage);
  void checkItems();
  void setState(Message *stateMessage);
  void requestState(Message *requestMessage);
  bool nextState(Message *msg);
};

#endif
