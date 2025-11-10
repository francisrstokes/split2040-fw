import argparse
from time import sleep
from .kb_config import KBConfig
from .keyboard import KCParser, name_to_kc, print_layer

parser = argparse.ArgumentParser()
parser.add_argument("--key", "-k", nargs=4, action="append", help="Set a key. Example: `--key=0 1 2 \"KC(A)\"` set layer 0, row 1, col 2 to regular keycode 'A'")
parser.add_argument("--combo", "-c", nargs='+', action="append", help="Set a combo. Example: `--combo=0 \"KC(A)\" \"KC(B)\" \"KC(C)\"` press 'A' and 'B' to get 'C'")
parser.add_argument("--save", "-s", action="store_true", help="Save the current changes to flash. Happens after all other commands, before reset (if applicable)")
parser.add_argument("--get-layer", "-l", type=int, help="Get and print the a keyboard layer")
parser.add_argument("--reset", "-r", action='store_true', help="Reset to the bootloader. Happens after all other commands have been processed.")
parser.add_argument("--list", action='store_true', help="Print a list of valid key names")
parser.add_argument("--dump", type=str, help="Dump the config to a raw binary file with given filename")

def main():
    args = parser.parse_args()

    if args.list:
        for keyname in name_to_kc.keys():
            print(keyname)
        return

    kb = KBConfig()
    key_parser = KCParser('')

    if args.get_layer is not None:
        info = kb.get_info()
        if args.get_layer < info.layer_count:
            layer_data = kb.get_layout(args.get_layer)
            in_rows = [layer_data[i:i+info.column_count] for i in range(0, len(layer_data), info.column_count)]
            print_layer(in_rows)
        else:
            print(f"layer out of range: {args.get_layer}/{info.layer_count-1}")

    if args.dump is not None:
        kb.dump_config(args.dump)
        return

    if args.combo is not None:
        info = kb.get_info()
        for index, *keys, key_out in args.combo:
            if len(keys) >= info.combo_max_size:
                raise Exception(f"Too many keys provided for combo (max={info.combo_max_size})")
            resolved_keys = [key_parser.parse(k) for k in keys]
            resolved_key_out = key_parser.parse(key_out)
            kb.set_combo(int(index), resolved_keys, resolved_key_out, info.combo_max_size)

    if args.key is not None:
        for layer, row, col, key_str in args.key:
            code = key_parser.parse(key_str)
            print(f"{code:08x}")
            kb.set_key(int(layer), int(row), int(col), code)

    if args.save:
        kb.commit_to_flash()
        sleep(1)

    if args.reset:
        kb.reset_to_bootloader()
