import yaml

from . import schema


def load_config(filename: str):
    with open(filename, "r") as fp:
        return schema.validate_config(yaml.load(fp, yaml.SafeLoader))
