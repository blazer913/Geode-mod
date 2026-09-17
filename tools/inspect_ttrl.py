#!/usr/bin/env python3
import struct
import sys
from pathlib import Path

def hexdump(data, start=0, count=256):
    chunk = data[start:start+count]
    for off in range(0, len(chunk), 16):
        row = chunk[off:off+16]
        hx = " ".join(f"{b:02x}" for b in row)
        asc = "".join(chr(b) if 32 <= b < 127 else "." for b in row)
        print(f"{start+off:08x}  {hx:<47}  {asc}")

def main():
    if len(sys.argv) != 2:
        print("usage: inspect_ttrl.py file.ttrl")
        raise SystemExit(2)

    p = Path(sys.argv[1])
    data = p.read_bytes()

    print("file:", p)
    print("size:", len(data))

    if len(data) >= 8:
        magic = data[:4]
        version = data[4]
        flags = data[5]
        header_size = struct.unpack_from("<H", data, 6)[0]
        print("magic:", magic)
        print("version:", version)
        print("flags:", flags)
        print("header_size:", header_size)

    print("\nfirst 256 bytes:")
    hexdump(data, 0, 256)

    print("\nlast 256 bytes:")
    hexdump(data, max(0, len(data)-256), 256)

if __name__ == "__main__":
    main()
