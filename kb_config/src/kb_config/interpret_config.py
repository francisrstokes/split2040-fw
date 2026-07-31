import argparse
import struct
import ctypes

from keyboard import print_layer

class kb_config_flash_header_t(ctypes.Structure):
    _fields_ = [
        ("sentinel", ctypes.c_uint32),
        ("format_version", ctypes.c_uint32),
        ("write_count", ctypes.c_uint32),
        ("row_count", ctypes.c_uint8),
        ("column_count", ctypes.c_uint8),
        ("layer_count", ctypes.c_uint8),
        ("led_count", ctypes.c_uint8),
        ("macro_count", ctypes.c_uint8),
        ("combo_count", ctypes.c_uint8),
        ("macro_max_size", ctypes.c_uint8),
        ("combo_max_size", ctypes.c_uint8),
    ]

parser = argparse.ArgumentParser()
parser.add_argument("file", type=str, help="The raw config file")

def main():
    args = parser.parse_args()

    with open(args.file, "rb") as f:
        config_dump = f.read()

    header = kb_config_flash_header_t.from_buffer_copy(config_dump[0:ctypes.sizeof(kb_config_flash_header_t)])

    if header.sentinel != int.from_bytes(b'KEEB', 'big'):
        print("Invalid config")
        print(header.sentinel, hex(header.sentinel), header.sentinel.to_bytes(4, 'little'))
        exit(1)

    header_size = ctypes.sizeof(kb_config_flash_header_t)
    layer_size = ctypes.sizeof(ctypes.c_uint32) * header.row_count * header.column_count
    layers = header.layer_count
    macro_size = header.macro_max_size + 4
    macros = header.macro_count
    combo_size = 4 * 4 + 4
    combos = header.combo_count

    layers_offset = header_size

    def hexdump(data, display_offset, grouping, groups_per_line):
        hex_groups = data.hex(" ", grouping).split(" ")
        offset = 0
        while offset < len(hex_groups):
            print(f"{display_offset + offset * grouping:08x}: {' '.join(hex_groups[offset:offset + groups_per_line])}")
            offset += groups_per_line

    def dump(title, start, size):
        print(title.center(80, "."))
        hexdump(config_dump[start:start+size], start, 4, 8)
        print()
        return start + size

    offset = 0
    offset = dump("header", offset, header_size)

    for layer in range(layers):
        # offset = dump(f"layer {layer}", offset, layer_size)
        print(f"layer {layer}".center(80, "."))
        layer_offset = layers_offset + layer * layer_size
        layer_data = []
        for row_offset in range(header.row_count):
            row = []
            for col_offset in range(header.column_count):
                offset = layer_offset + row_offset * header.column_count + col_offset * ctypes.sizeof(ctypes.c_uint32)
                row.append(struct.unpack("I", config_dump[offset:offset+4])[0])
            layer_data.append(row)
        print_layer(layer_data)

    for macro in range(macros):
        offset = dump(f"macro {macro}", offset, macro_size)

    for combo in range(combos):
        offset = dump(f"combo {combo}", offset, combo_size)

if __name__ == "__main__":
    main()
