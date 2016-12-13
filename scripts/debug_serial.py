#!/usr/bin/python3

import subprocess


def build_fakeio(portA, portB):
    return subprocess.Popen(["socat",
                             "PTY,link={:s},raw,wait-slave".format(portA),
                             "PTY,link={:s},raw,wait-slave".format(portB)])

if __name__ == "__main__":
    pass