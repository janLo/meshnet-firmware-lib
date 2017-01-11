from typing import Dict

import voluptuous as vol

CONFIG_NODES = "nodes"
CONFIG_DEVICES = "devices"
CONFIG_NAME = "name"
CONFIG_PIN = "pin"
CONFIG_TYPE = "type"
CONFIG_DATA = "data"


def dev_schema(typename: str, data_schema: Dict):
    return vol.Schema({vol.Required(CONFIG_NAME): str,
                       vol.Required(CONFIG_TYPE): typename,
                       vol.Required(CONFIG_DATA): vol.Schema(data_schema)})


NODE_SCHEMA = vol.Schema({int: {vol.Required(CONFIG_NAME): str,
                                vol.Required(CONFIG_DEVICES):
                                    vol.Any(dev_schema("bin_switch", {"pin": int}),
                                            dev_schema("bin_sensor", {"pin": int}))}})

CONFIG_SCHEMA = vol.Schema({vol.Required(CONFIG_NODES): NODE_SCHEMA})


def validate_config(config):
    return CONFIG_SCHEMA(config)
