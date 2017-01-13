import unittest

from meshnet.serio.util import to_hex


class TestUtil(unittest.TestCase):
    def test_to_hex(self):
        data = b"heuhfsdowofurhfualdgf"
        self.assertEqual("68 65 75 68 66 73 64 6f 77 6f 66 75 72 68 66 75 61 6c 64 67 66",
                         to_hex(data))
