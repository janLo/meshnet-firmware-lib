#include "items.hpp"
#include "common.hpp"
#include "message.hpp"

#include <string.h>

Item::Item(const char *_id) {
  strlcpy(item_id, _id, MAX_ID_LEN + 1);
  item_id[MAX_ID_LEN] = '\0';
}

bool Item::hasChanged() { return false; }

BinSwitch::BinSwitch(const char *_id, uint8_t const pin)
    : Item(_id), value(0), pin(pin) {
  pinMode(pin, OUTPUT);
}

void BinSwitch::setState(Message *state) {
  value = state->getByte();
  digitalWrite(pin, value);
}

void BinSwitch::getState(Message *state) { state->setByte(digitalRead(pin)); }

bool BinSwitch::hasChanged() { return value != digitalRead(pin); }

BinSensor::BinSensor(const char *_id, uint8_t const pin)
    : Item(_id), pin(pin), value(false) {
  pinMode(pin, INPUT);
}

void BinSensor::setState(Message *state) {}
void BinSensor::getState(Message *state) {
  int16_t value = digitalRead(pin);
  state->setByte(value & 0xff);
}
bool BinSensor::hasChanged() { return digitalRead(pin) != value; }

AnalogSensor::AnalogSensor(const char *_id, uint8_t const pin)
    : Item(_id), pin(pin), value(0) {}

void AnalogSensor::setState(Message *state) {}
void AnalogSensor::getState(Message *state) {
  uint16_t value = analogRead(pin);
  state->setShort(value);
}
bool AnalogSensor::hasChanged() { return value != analogRead(pin); }

OneWire::OneWire(const char *_id, uint8_t const pin)
    : Item(_id), pin(pin) { /* ToDo */
}

void OneWire::setState(Message *state) {}
void OneWire::getState(Message *state) { /* ToDo */
}
bool OneWire::hasChanged() { return false; }

Dimmer::Dimmer(const char *_id, uint8_t const pin)
    : Item(_id), value(0), pin(pin) {
  pinMode(pin, OUTPUT);
}

void Dimmer::setState(Message *state) {
  value = state->getByte();
  analogWrite(pin, value & 0xff);
}

void Dimmer::getState(Message *state) { state->setByte(value); }

RGBLamp::RGBLamp(const char *_id, uint8_t const r, uint8_t const g,
                 uint8_t const b)
    : Item(_id), red(0), green(0), blue(0), red_pin(r), green_pin(g),
      blue_pin(b) {
  pinMode(red_pin, OUTPUT);
  pinMode(green_pin, OUTPUT);
  pinMode(blue_pin, OUTPUT);
}

void RGBLamp::setState(Message *state) {
  red = state->getByte();
  green = state->getByte();
  blue = state->getByte();

  analogWrite(red_pin, red & 0xff);
  analogWrite(green_pin, green & 0xff);
  analogWrite(blue_pin, blue & 0xff);
}

void RGBLamp::getState(Message *state) {
  state->setByte(red);
  state->setByte(green);
  state->setByte(blue);
}

DHTSensor::DHTSensor(const char *_id, uint8_t pin)
    : Item(_id), pin(pin) { /* Todo */
}

void DHTSensor::setState(Message *state) {}
void DHTSensor::getState(Message *state) { /* ToDo */
}
bool DHTSensor::hasChanged() { return false; }

ItemRegistry::ItemRegistry() : item_cnt(0) {}

int ItemRegistry::configure(Message *configMessage) {
  unsigned new_id = item_cnt;
  if (MAX_ITEMS == new_id) {
    /* We reached MAX_IEMS */
    DEBUG_LOG("already reached may items");
    return -1;
  }

  items_t item_type = (items_t)configMessage->getByte();
  const char *cfg = configMessage->getBytes(3);
  const char *name = configMessage->getBytes(configMessage->remain());

  Item *new_item;
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
    DEBUG_LOG("got an invalid sensor type: %x", item_type);
    return -1;
  }
  item_list[new_id] = new_item;

  item_cnt++;
  return new_id;
}

void ItemRegistry::checkItems() {
  for (uint8_t i = 0; i < item_cnt; ++i) {
    if (!update_available[i] && item_list[i]->hasChanged()) {
      update_available[i] = true;
    }
  }
}

void ItemRegistry::setState(Message *stateMessage) {
  uint8_t item_id = stateMessage->getByte();

  if (item_id >= item_cnt) {
    DEBUG_LOG("got an invalid item-id: %x", item_id);
    return;
  }

  item_list[item_id]->setState(stateMessage);
  update_available[item_id] = true;
}

void ItemRegistry::requestState(Message *requestMessage) {
  uint8_t item_id = requestMessage->getByte();

  if (item_id >= item_cnt) {
    DEBUG_LOG("got an invalid item-id: %x", item_id);
    return;
  }

  update_available[item_id] = true;
}

bool ItemRegistry::nextState(Message *msg) {
  for (uint8_t i = 0; i < item_cnt; ++i) {
    if (update_available[i]) {
      msg->setByte(i);
      item_list[i]->getState(msg);
      update_available[i] = false;
      return true;
    }
  }
  return false;
}
