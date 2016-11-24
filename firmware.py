#!/usr/bin/env python3

import argparse

NODE_LIBS = ["RF24Mesh", "SipHash"]
MASTER_LIBS = ["RF24Mesh"]


def install(args):
    pass


def init(args):
    pass


def libs(args):
    pass


def build(args):
    pass


def upload(args):
    pass


parser = argparse.ArgumentParser(description="handle firmware")
subparsers = parser.add_subparsers(help="Available commands")

install_parser = subparsers.add_parser("install", help="Install PlatformIO")
install_parser.set_default(install)

init_parser = subparsers.add_parser("init", help="Initialize Projects")
init_parser.set_default(init)

libs_parser = subparsers.add_parser("libs", help="Handle embedded librraies")
libs_parser.set_default(libs)

build_parser = subparsers.add_parser("build", help="Build firmware")
build_parser.set_default(build)

upload_parser = subparsers.add_parser("upload", help="Upload firmware to controller")
upload_parser.set_default(upload)


if __name__ == "__main__":
    args = parser.parse_args()
    args.func(args)
