import argparse
from .kb_config import KBConfig
from .keyboard import KCParser, name_to_kc

'''
kbconfig --reset
kbconfig --key=0,1,2,KC(A)  # set layer 0, row 1, col 2 to 'A'
'''
parser = argparse.ArgumentParser()
parser.add_argument("--key", "-k", nargs=4, action="append", help="Set a key. Example: `--key=0 1 2 \"KC(A)\"` set layer 0, row 1, col 2 to regular keycode 'A'")
parser.add_argument("--reset", "-r", action='store_true', help="Reset to the bootloader. Happens after all other commands have been processed.")
parser.add_argument("--list", action='store_true', help="Print a list of valid key names")

def main():
    args = parser.parse_args()

    if args.list:
        for keyname in name_to_kc.keys():
            print(keyname)
        return

    kb = KBConfig()

    if args.key is not None:
        key_parser = KCParser('')
        for layer, row, col, key_str in args.key:
            code = key_parser.parse(key_str)
            print(f"{code:08x}")
            kb.set_key(int(layer), int(row), int(col), code)

    if args.reset:
        kb.reset_to_bootloader()
