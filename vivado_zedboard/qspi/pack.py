"""Wrap the raw RV32 image in a length and CRC header for the QSPI slot."""
import struct
import sys
import zlib
from pathlib import Path

image = Path(sys.argv[1]).read_bytes()
if not 4 <= len(image) <= 0x3a00:
    raise SystemExit("Application image must be 4 through 14848 bytes")
header = struct.pack("<4sII", b"PRV1", len(image), zlib.crc32(image))
Path(sys.argv[2]).write_bytes(header + image)
