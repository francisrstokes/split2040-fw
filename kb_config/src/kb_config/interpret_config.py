import argparse
import struct
import ctypes

from .keyboard import print_layer, key_to_str

class kb_config_flash_header_t(ctypes.Structure):
    _fields_ = [
        ("sentinel", ctypes.c_char * 4),
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

    def __repr__(self):
        lines = [
            f"Sentinel:       {str(self.sentinel, 'utf8')}",
            f"Format version: {self.format_version}",
            f"Write count:    {self.write_count}",
            f"Row count:      {self.row_count}",
            f"Column count:   {self.column_count}",
            f"Layer count:    {self.layer_count}",
            f"Led count:      {self.led_count}",
            f"Macro count:    {self.macro_count}",
            f"Combo count:    {self.combo_count}",
            f"Macro max size: {self.macro_max_size}",
            f"Combo max size: {self.combo_max_size}",
        ]
        return '\n'.join(lines) + '\n'

def hexdump(data, display_offset, grouping, groups_per_line):
    hex_groups = data.hex(" ", grouping).split(" ")
    offset = 0
    str_out = ''
    while offset < len(hex_groups):
        str_out += f"{display_offset + offset * grouping:08x}: {' '.join(hex_groups[offset:offset + groups_per_line])}\n"
        offset += groups_per_line
    return str_out

class ConfigInterpreter:
    def __init__(self, config_dump: bytes):
        self.config_dump = config_dump
        self.header = kb_config_flash_header_t.from_buffer_copy(config_dump[0:ctypes.sizeof(kb_config_flash_header_t)])

    def interpret(self):
        if self.header.sentinel != b'BEEK':
            return f"Invalid config, bad sentinel: {self.header.sentinel}"

        header_size = ctypes.sizeof(kb_config_flash_header_t)
        layer_size = ctypes.sizeof(ctypes.c_uint32) * self.header.row_count * self.header.column_count
        layers = self.header.layer_count
        macro_size = self.header.macro_max_size + 4
        macros = self.header.macro_count
        combo_size = 4 * 4 + 4
        combos = self.header.combo_count

        layers_offset = header_size

        class kb_macro(ctypes.Structure):
            _fields_ = [
                ("type", ctypes.c_uint16),
                ("length", ctypes.c_uint16),
                ("string", ctypes.c_char * self.header.macro_max_size),
            ]

            def __repr__(self):
                if self.type == 1:
                    return f"\"{str(self.string, 'utf8')}\" [{self.length} bytes]"
                else:
                    return "<unused>"

        class kb_combo(ctypes.Structure):
            _fields_ = [
                ("keys_in", ctypes.c_uint32 * self.header.combo_max_size),
                ("key_out", ctypes.c_uint32),
            ]

            def __repr__(self):
                valid_keys = [k for k in self.keys_in if not k == 0]
                if len(valid_keys) == 0:
                    return '<unused>'
                return ' + '.join([key_to_str(k) for k in valid_keys]) + ' -> ' + key_to_str(self.key_out)

        def dump(title, start, size):
            str_out = title.center(80, ".") + '\n'
            str_out += hexdump(self.config_dump[start:start+size], start, 4, 8) + '\n'
            return str_out

        str_out = ''
        offset = header_size
        str_out += str(self.header)

        for layer in range(layers):
            str_out += f"layer {layer}".center(80, ".") + '\n'
            layer_offset = layers_offset + layer * layer_size
            layer_data = []
            for row_num in range(self.header.row_count):
                row_offset = row_num * self.header.column_count * ctypes.sizeof(ctypes.c_uint32)
                row = []
                for col_num in range(self.header.column_count):
                    col_offset = col_num * ctypes.sizeof(ctypes.c_uint32)
                    offset = layer_offset + row_offset + col_offset
                    row.append(struct.unpack("I", self.config_dump[offset:offset+4])[0])
                layer_data.append(row)
            str_out += print_layer(layer_data)

        offset = header_size + layer_size * self.header.layer_count
        for macro in range(macros):
            macro_raw = self.config_dump[offset:offset+macro_size]
            str_out += dump(f"macro {macro}", offset, macro_size)
            offset += macro_size
            str_out += str(kb_macro.from_buffer_copy(macro_raw)) + '\n'

        for combo in range(combos):
            combo_raw = self.config_dump[offset:offset+combo_size]
            str_out += dump(f"combo {combo}", offset, combo_size)
            offset += combo_size
            str_out += str(kb_combo.from_buffer_copy(combo_raw)) + '\n'

        return str_out

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("file", type=str, help="The raw config file")
    args = parser.parse_args()

    with open(args.file, "rb") as f:
        config_dump = f.read()

    print(ConfigInterpreter(config_dump).interpret())

if __name__ == "__main__":
    main()
