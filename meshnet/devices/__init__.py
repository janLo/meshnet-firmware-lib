from enum import Enum


class DeviceType(Enum):
    bin_switch = 0
    bin_sensor = 1
    analog_sensor = 3
    one_wire = 4
    rgb_lamp = 5
    dimmer = 6
    dht_sensor = 7
