#include "items.hpp"
#include <string.h>

Item::Item(const char* _id) {
  strlcpy(item_id, _id, MAX_ID_LEN + 1);
  item_id[MAX_ID_LEN] = '\0';
}

bool Item::hasChanged() { return false; }

BinSwitch::BinSwitch(const char* _id, uint8_t const pin)
  : Item(_id), value(0), pin(pin)
{
  pinMode(pin, OUTPUT);
}

void BinSwitch::setState(const char* state) {
  value = state[0];
  digitalWrite(pin, value);
}
void BinSwitch::getState(char* state) {}


BinSensor::BinSensor(const char* _id, uint8_t const pin)
  : Item(_id), pin(pin), value(false)
{
  pinMode(pin, INPUT);
}

void BinSensor::setState(const char* state) {}
void BinSensor::getState(char* state) {
  int16_t value = digitalRead(pin);
  state[0] = value & 0xff;
}
bool BinSensor::hasChanged() { return digitalRead(pin) != value; }

AnalogSensor::AnalogSensor(const char* _id, uint8_t const pin)
  : Item(_id), pin(pin), value(0)
{
}

void AnalogSensor::setState(const char* state) {}
void AnalogSensor::getState(char* state) {
  uint16_t value = analogRead(pin);
  state[0] = value & 0xff;
  state[1] = (value >> 8) & 0xff;
}
bool AnalogSensor::hasChanged() { return value != analogRead(pin); }

OneWire::OneWire(const char* _id, uint8_t const pin)
  : Item(_id), pin(pin)
{   /* ToDo */ }

void OneWire::setState(const char* state) {}
void OneWire::getState(char* state) { /* ToDo */ }
bool OneWire::hasChanged() { return false; }

Dimmer::Dimmer(const char* _id, uint8_t const pin)
: Item(_id), value(0), pin(pin)
{
  pinMode(pin, OUTPUT);
}

void Dimmer::setState(const char* state) {
  value = state[0];
  analogWrite(pin, value & 0xff);
}

void Dimmer::getState(char* state) {
  state[0] = value & 0xff;
}

RGBLamp::RGBLamp(const char* _id, uint8_t const r, uint8_t const g, uint8_t const b)
: Item(_id), red(0), green(0), blue(0), red_pin(r), green_pin(g), blue_pin(b)
{
  pinMode(red_pin, OUTPUT);
  pinMode(green_pin, OUTPUT);
  pinMode(blue_pin, OUTPUT);
}

void RGBLamp::setState(const char* state) {
  red = state[0];
  green = state[1];
  blue = state[2];

  analogWrite(red_pin, red & 0xff);
  analogWrite(green_pin, green & 0xff);
  analogWrite(blue_pin, blue & 0xff);
}

void RGBLamp::getState(char* state) {
  state[0] = red & 0xff;
  state[1] = green & 0xff;
  state[2] = blue & 0xff;
}

DHTSensor::DHTSensor(const char * _id, uint8_t pin)
  : Item(_id), pin(pin)
{ /* Todo */ }

void DHTSensor::setState(const char* state) {}
void DHTSensor::getState(char* state) { /* ToDo */ }
bool DHTSensor::hasChanged() { return false; }


ItemRegistry::ItemRegistry()
  : item_cnt(0)
{}

int ItemRegistry::configure(const char * configMessage) {
  unsigned new_id = item_cnt;
  if (MAX_ITEMS == new_id) {
    /* We reached MAX_IEMS */
    return -1;
  }

  items_t item_type = (items_t)configMessage[0];
  const char * cfg = configMessage + 1;
  const char * name = cfg + 3;

  Item* new_item;
  switch (item_type) {
    case bin_switch:
      new_item = new BinSwitch(name, cfg[0]);
      break;
    case bin_sensor:
      new_item = new BinSensor(name, cfg[0]);
      break;
    case analog_sensor:
      new_item = new AnalogSensor(name, cfg[0]);
      break;
    case one_wire:
      new_item = new OneWire(name, cfg[0]);
      break;
    case rgb_lamp:
      new_item = new RGBLamp(name, cfg[0], cfg[1], cfg[2]);
      break;
    case dimmer:
      new_item = new Dimmer(name, cfg[0]);
      break;
    case dht_sensor:
      new_item = new DHTSensor(name, cfg[0]);
        break;
    default:
      return -1;
  }
  item_list[new_id] = new_item;

  item_cnt++;
  return new_id;
}

void ItemRegistry::setState(const char* stateMessage) {}
