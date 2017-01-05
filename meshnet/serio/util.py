def to_hex(hh: bytes, for_c=False):
    hex_values = ("{:02x}".format(c) for c in hh)
    if for_c:
        return "{" + ", ".join("0x{}".format(x) for x in hex_values) + "}"
    else:
        return " ".join(hex_values)
