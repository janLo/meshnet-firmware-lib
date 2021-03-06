#!/usr/bin/env python

import os
import sys
import argparse
import subprocess

PROJECTS = {"master_firmware": ["RF24Mesh"],
            "node_firmware": ["RF24Mesh", "SipHash"]}


def _proj_path(proj):
    return os.path.join(
        os.path.dirname(os.path.abspath(os.path.expanduser(__file__))),
        proj)


def install(args):
    subprocess.call(["pip", "install", "--upgrade", "platformio"])


def init(args):
    pass


def libs(args):
    for proj in PROJECTS:
        print("\nProject: {}".format(proj))
        if args.action == "install":
            for lib in PROJECTS[proj]:
                subprocess.call(["platformio", "lib", "install", lib],
                                cwd=_proj_path(proj))
        else:
            subprocess.call(["platformio", "lib", "list"],
                            cwd=_proj_path(proj))
        print("\n")


def build(args):
    for proj in PROJECTS:
        print("\nProject: {}".format(proj))
        subprocess.call(["platformio", "run"],
                        cwd=_proj_path(proj))


def upload(args):
    pass


parser = argparse.ArgumentParser(description="handle firmware")
subparsers = parser.add_subparsers(help="Available commands")

install_parser = subparsers.add_parser("install", help="Install/Upgrade PlatformIO")
install_parser.set_defaults(func=install)

init_parser = subparsers.add_parser("init", help="Initialize Projects")
init_parser.set_defaults(func=init)

libs_parser = subparsers.add_parser("libs", help="Handle embedded libraries")
libs_parser.add_argument("action", help="What to do", choices=["list", "install"], default="list")
libs_parser.set_defaults(func=libs)

build_parser = subparsers.add_parser("build", help="Build firmware")
build_parser.set_defaults(func=build)

upload_parser = subparsers.add_parser("upload", help="Upload firmware to controller")
upload_parser.set_defaults(func=upload)

if __name__ == "__main__":
    args = parser.parse_args()
    args.func(args)
