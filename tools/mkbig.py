#!/usr/bin/env python3
"""Write the 100000-byte test file /big.bin. Byte i is (i*7 + i//251) & 0xFF,
a pattern that catches off-by-one and block-boundary bugs in file reading."""
import sys

data = bytes(((i * 7 + i // 251) & 0xFF) for i in range(100000))
with open(sys.argv[1], "wb") as f:
    f.write(data)
