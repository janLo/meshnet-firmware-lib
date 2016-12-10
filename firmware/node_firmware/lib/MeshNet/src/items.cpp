#include "items.hpp"
#include "common.hpp"
#include "message.hpp"

#include <string.h>

Item::Item() { item_id[MAX_ID_LEN] = '\0'; }

void Item::setId(const char *id) {
  strlcpy(item_id, id, MAX_ID_LEN + 1);
  item_id[MAX_ID_LEN] = '\0';
}

bool Item::hasChanged() { return false; }

BinSwitch::BinSwitch(Message *cfg) : Item(), value(0), pin(cfg->getByte()) {
  pinMode(pin, OUTPUT);
  setId((char *)cfg->getBytes(cfg->remain()));
}

void BinSwitch::setState(Message *state) {
  value = state->getBool();
  digitalWrite(pin, (value ? HIGH : LOW));
}

void BinSwitch::getState(Message *state) {
  state->setBool(digitalRead(pin) == HIGH);
}

bool BinSwitch::hasChanged() { return value != (digitalRead(pin) == HIGH); }

BinSensor::BinSensor(Message *cfg) : Item(), pin(cfg->getByte()), value(false) {
  pinMode(pin, INPUT);
  setId((char *)cfg->getBytes(cfg->remain()));
}

void BinSensor::setState(Message *state) {}
void BinSensor::getState(Message *state) {
  int16_t value = (digitalRead(pin) == HIGH);
  state->setBool(value);
}
bool BinSensor::hasChanged() { return (digitalRead(pin) == HIGH) != value; }

AnalogSensor::AnalogSensor(Message *cfg)
    : Item(), pin(cfg->getByte()), value(0), delta(cfg->getShort()) {
  setId((char *)cfg->getBytes(cfg->remain()));
}

void AnalogSensor::setState(Message *state) {}
void AnalogSensor::getState(Message *state) {
  uint16_t value = analogRead(pin);
  state->setShort(value);
}
bool AnalogSensor::hasChanged() {
  return (uint16_t)abs(value - analogRead(pin)) > delta;
}

OneWire::OneWire(Message *cfg) : Item(), pin(cfg->getByte()) {

  /* ToDo */
  setId((char *)cfg->getBytes(cfg->remain()));
}

void OneWire::setState(Message *state) {}
void OneWire::getState(Message *state) { /* ToDo */
}
bool OneWire::hasChanged() { return false; }

Dimmer::Dimmer(Message *cfg) : Item(), value(0), pin(cfg->getByte()) {
  pinMode(pin, OUTPUT);
  setId((char *)cfg->getBytes(cfg->remain()));
}

void Dimmer::setState(Message *state) {
  value = state->getByte();
  analogWrite(pin, value & 0xff);
}

void Dimmer::getState(Message *state) { state->setByte(value); }

RGBLamp::RGBLamp(Message *cfg)
    : Item(), red(0), green(0), blue(0), red_pin(cfg->getByte()),
      green_pin(cfg->getByte()), blue_pin(cfg->getByte()) {
  pinMode(red_pin, OUTPUT);
  pinMode(green_pin, OUTPUT);
  pinMode(blue_pin, OUTPUT);
  setId((char *)cfg->getBytes(cfg->remain()));
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

DHTSensor::DHTSensor(Message *cfg)
    : Item(), pin(cfg->getByte()), temp_delta(cfg->getShort()),
      humidity_delta(cfg->getShort()) { /* Todo */
  setId((char *)cfg->getBytes(cfg->remain()));
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
    DEBUG_LOG("already reached max items");
    return -1;
  }

  items_t item_type = (items_t)configMessage->getByte();

  DEBUG_LOG("Configure Node type %x", item_type);

  Item *new_item;
  switch (item_type) {
  case bin_switch:
    new_item = new BinSwitch(configMessage);
    break;
  case bin_sensor:
    new_item = new BinSensor(configMessage);
    break;
  case analog_sensor:
    new_item = new AnalogSensor(configMessage);
    break;
  case one_wire:
    new_item = new OneWire(configMessage);
    break;
  case rgb_lamp:
    new_item = new RGBLamp(configMessage);
    break;
  case dimmer:
    new_item = new Dimmer(configMessage);
    break;
  case dht_sensor:
    new_item = new DHTSensor(configMessage);
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
