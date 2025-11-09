import usb.core
import usb.util
from typing import List
from time import sleep
from array import array
import struct
import ctypes
import queue
import math
import enum
from datetime import timedelta, datetime

# Need to add a udev rule like:
# SUBSYSTEMS=="usb", ATTRS{idVendor}=="7083", ATTRS{idProduct}=="0003", GROUP="plugdev", MODE="0777"

PACKET_SIZE                         = 64


KB_CONFIG_MSG_TYPE_REQ              = (0x00)
KB_CONFIG_MSG_TYPE_RES              = (0x80)

KB_CONFIG_MSG_GET_INFO              = (0x01)
KB_CONFIG_MSG_GET_LAYOUT            = (0x02)
KB_CONFIG_MSG_SET_KEY               = (0x03)
KB_CONFIG_MSG_COMMIT                = (0x04)
KB_CONFIG_MSG_GET_MACRO             = (0x05)
KB_CONFIG_MSG_SET_MACRO             = (0x06)
KB_CONFIG_MSG_RESET_TO_BL           = (0x07)
KB_CONFIG_MSG_DUMP_CONFIG           = (0x08)
KB_CONFIG_MSG_GET_COMBO             = (0x09)
KB_CONFIG_MSG_SET_COMBO             = (0x0A)

KB_CONFIG_COMMIT_OP_CANCEL          = (0)
KB_CONFIG_COMMIT_OP_SAVE            = (1)
KB_CONFIG_COMMIT_OP_ERASE           = (2)

ENTRY_TYPE_KC           = 0x00000000
ENTRY_TYPE_LAYER        = 0x10000000
ENTRY_TYPE_TAPHOLD      = 0x20000000
ENTRY_TYPE_DOUBLE_TAP   = 0x30000000
ENTRY_TYPE_MACRO        = 0x40000000
ENTRY_TYPE_KBC          = 0x50000000
ENTRY_TYPE_CC           = 0x60000000
ENTRY_TYPE_MOUSE        = 0x70000000

ENTRY_TYPE_MASK         = (0xf0000000)
ENTRY_TYPE_SHIFT        = (0xf0000000)
ENTRY_ARG4_MASK         = (0x0f000000)
ENTRY_ARG4_SHIFT        = (24)
ENTRY_ARG8_MASK         = (0x00ff0000)
ENTRY_ARG8_SHIFT        = (16)

KC_MASK                 = (0x000000ff)
KEY_MODS_MASK           = (0x0000ff00)
KEY_MODS_SHIFT          = (8)

KEY_MOD_LCTRL           = 0x01
KEY_MOD_LSHIFT          = 0x02
KEY_MOD_LALT            = 0x04
KEY_MOD_LMETA           = 0x08
KEY_MOD_RCTRL           = 0x10
KEY_MOD_RSHIFT          = 0x20
KEY_MOD_RALT            = 0x40
KEY_MOD_RMETA           = 0x80

bit_to_mod_name = {
    0x01: "LC",
    0x02: "LS",
    0x04: "LA",
    0x08: "LG",
    0x10: "RC",
    0x20: "RS",
    0x40: "RA",
    0x80: "RG",
}

mod_name_to_bit = {
    "LC": 0x01,
    "LS": 0x02,
    "LA": 0x04,
    "LG": 0x08,
    "RC": 0x10,
    "RS": 0x20,
    "RA": 0x40,
    "RG": 0x80,
}

kc_to_name = {
    0x00: "",
    0x01: "____",
    0x04: "A",
    0x05: "B",
    0x06: "C",
    0x07: "D",
    0x08: "E",
    0x09: "F",
    0x0a: "G",
    0x0b: "H",
    0x0c: "I",
    0x0d: "J",
    0x0e: "K",
    0x0f: "L",
    0x10: "M",
    0x11: "N",
    0x12: "O",
    0x13: "P",
    0x14: "Q",
    0x15: "R",
    0x16: "S",
    0x17: "T",
    0x18: "U",
    0x19: "V",
    0x1a: "W",
    0x1b: "X",
    0x1c: "Y",
    0x1d: "Z",
    0x1e: "1",
    0x1f: "2",
    0x20: "3",
    0x21: "4",
    0x22: "5",
    0x23: "6",
    0x24: "7",
    0x25: "8",
    0x26: "9",
    0x27: "0",
    0x28: "ENTER",
    0x29: "ESC",
    0x2a: "BACKSPACE",
    0x2b: "TAB",
    0x2c: "SPACE",
    0x2d: "MINUS",
    0x2e: "EQUAL",
    0x2f: "LEFTBRACE",
    0x30: "RIGHTBRACE",
    0x31: "BACKSLASH",
    0x32: "HASHTILDE",
    0x33: "SEMICOLON",
    0x34: "APOSTROPHE",
    0x35: "GRAVE",
    0x36: "COMMA",
    0x37: "DOT",
    0x38: "SLASH",
    0x39: "CAPSLOCK",
    0x3a: "F1",
    0x3b: "F2",
    0x3c: "F3",
    0x3d: "F4",
    0x3e: "F5",
    0x3f: "F6",
    0x40: "F7",
    0x41: "F8",
    0x42: "F9",
    0x43: "F10",
    0x44: "F11",
    0x45: "F12",
    0x46: "SYSRQ",
    0x47: "SCROLLLOCK",
    0x48: "PAUSE",
    0x49: "INSERT",
    0x4a: "HOME",
    0x4b: "PAGEUP",
    0x4c: "DELETE",
    0x4d: "END",
    0x4e: "PAGEDOWN",
    0x4f: "RIGHT",
    0x50: "LEFT",
    0x51: "DOWN",
    0x52: "UP",
    0x53: "NUMLOCK",
    0x54: "KPSLASH",
    0x55: "KPASTERISK",
    0x56: "KPMINUS",
    0x57: "KPPLUS",
    0x58: "KPENTER",
    0x59: "KP1",
    0x5a: "KP2",
    0x5b: "KP3",
    0x5c: "KP4",
    0x5d: "KP5",
    0x5e: "KP6",
    0x5f: "KP7",
    0x60: "KP8",
    0x61: "KP9",
    0x62: "KP0",
    0x63: "KPDOT",
    0x64: "102ND",
    0x65: "COMPOSE",
    0x66: "POWER",
    0x67: "KPEQUAL",
    0x68: "F13",
    0x69: "F14",
    0x6a: "F15",
    0x6b: "F16",
    0x6c: "F17",
    0x6d: "F18",
    0x6e: "F19",
    0x6f: "F20",
    0x70: "F21",
    0x71: "F22",
    0x72: "F23",
    0x73: "F24",
    0x74: "OPEN",
    0x75: "HELP",
    0x76: "PROPS",
    0x77: "FRONT",
    0x78: "STOP",
    0x79: "AGAIN",
    0x7a: "UNDO",
    0x7b: "CUT",
    0x7c: "COPY",
    0x7d: "PASTE",
    0x7e: "FIND",
    0x7f: "MUTE",
    0x80: "VOLUMEUP",
    0x81: "VOLUMEDOWN",
    0x85: "KPCOMMA",
    0x87: "RO",
    0x88: "KATAKANAHIRAGANA",
    0x89: "YEN",
    0x8a: "HENKAN",
    0x8b: "MUHENKAN",
    0x8c: "KPJPCOMMA",
    0x90: "HANGEUL",
    0x91: "HANJA",
    0x92: "KATAKANA",
    0x93: "HIRAGANA",
    0x94: "ZENKAKUHANKAKU",
    0xb6: "KPLEFTPAREN",
    0xb7: "KPRIGHTPAREN",
    0xe0: "LEFTCTRL",
    0xe1: "LEFTSHIFT",
    0xe2: "LEFTALT",
    0xe3: "LEFTMETA",
    0xe4: "RIGHTCTRL",
    0xe5: "RIGHTSHIFT",
    0xe6: "RIGHTALT",
    0xe7: "RIGHTMETA",
    0xe8: "MEDIA_PLAYPAUSE",
    0xe9: "MEDIA_STOPCD",
    0xea: "MEDIA_PREVIOUSSONG",
    0xeb: "MEDIA_NEXTSONG",
    0xec: "MEDIA_EJECTCD",
    0xed: "MEDIA_VOLUMEUP",
    0xee: "MEDIA_VOLUMEDOWN",
    0xef: "MEDIA_MUTE",
    0xf0: "MEDIA_WWW",
    0xf1: "MEDIA_BACK",
    0xf2: "MEDIA_FORWARD",
    0xf3: "MEDIA_STOP",
    0xf4: "MEDIA_FIND",
    0xf5: "MEDIA_SCROLLUP",
    0xf6: "MEDIA_SCROLLDOWN",
    0xf7: "MEDIA_EDIT",
    0xf8: "MEDIA_SLEEP",
    0xf9: "MEDIA_COFFEE",
    0xfa: "MEDIA_REFRESH",
    0xfb: "MEDIA_CALC",
}

name_to_kc = {
    "NONE": 0x00,
    "ERR_OVF": 0x01,
    "A": 0x04,
    "B": 0x05,
    "C": 0x06,
    "D": 0x07,
    "E": 0x08,
    "F": 0x09,
    "G": 0x0a,
    "H": 0x0b,
    "I": 0x0c,
    "J": 0x0d,
    "K": 0x0e,
    "L": 0x0f,
    "M": 0x10,
    "N": 0x11,
    "O": 0x12,
    "P": 0x13,
    "Q": 0x14,
    "R": 0x15,
    "S": 0x16,
    "T": 0x17,
    "U": 0x18,
    "V": 0x19,
    "W": 0x1a,
    "X": 0x1b,
    "Y": 0x1c,
    "Z": 0x1d,
    "1": 0x1e,
    "2": 0x1f,
    "3": 0x20,
    "4": 0x21,
    "5": 0x22,
    "6": 0x23,
    "7": 0x24,
    "8": 0x25,
    "9": 0x26,
    "0": 0x27,
    "ENTER": 0x28,
    "ESC": 0x29,
    "BACKSPACE": 0x2a,
    "TAB": 0x2b,
    "SPACE": 0x2c,
    "MINUS": 0x2d,
    "EQUAL": 0x2e,
    "LEFTBRACE": 0x2f,
    "RIGHTBRACE": 0x30,
    "BACKSLASH": 0x31,
    "HASHTILDE": 0x32,
    "SEMICOLON": 0x33,
    "APOSTROPHE": 0x34,
    "GRAVE": 0x35,
    "COMMA": 0x36,
    "DOT": 0x37,
    "SLASH": 0x38,
    "CAPSLOCK": 0x39,
    "F1": 0x3a,
    "F2": 0x3b,
    "F3": 0x3c,
    "F4": 0x3d,
    "F5": 0x3e,
    "F6": 0x3f,
    "F7": 0x40,
    "F8": 0x41,
    "F9": 0x42,
    "F10": 0x43,
    "F11": 0x44,
    "F12": 0x45,
    "SYSRQ": 0x46,
    "SCROLLLOCK": 0x47,
    "PAUSE": 0x48,
    "INSERT": 0x49,
    "HOME": 0x4a,
    "PAGEUP": 0x4b,
    "DELETE": 0x4c,
    "END": 0x4d,
    "PAGEDOWN": 0x4e,
    "RIGHT": 0x4f,
    "LEFT": 0x50,
    "DOWN": 0x51,
    "UP": 0x52,
    "NUMLOCK": 0x53,
    "KPSLASH": 0x54,
    "KPASTERISK": 0x55,
    "KPMINUS": 0x56,
    "KPPLUS": 0x57,
    "KPENTER": 0x58,
    "KP1": 0x59,
    "KP2": 0x5a,
    "KP3": 0x5b,
    "KP4": 0x5c,
    "KP5": 0x5d,
    "KP6": 0x5e,
    "KP7": 0x5f,
    "KP8": 0x60,
    "KP9": 0x61,
    "KP0": 0x62,
    "KPDOT": 0x63,
    "102ND": 0x64,
    "COMPOSE": 0x65,
    "POWER": 0x66,
    "KPEQUAL": 0x67,
    "F13": 0x68,
    "F14": 0x69,
    "F15": 0x6a,
    "F16": 0x6b,
    "F17": 0x6c,
    "F18": 0x6d,
    "F19": 0x6e,
    "F20": 0x6f,
    "F21": 0x70,
    "F22": 0x71,
    "F23": 0x72,
    "F24": 0x73,
    "OPEN": 0x74,
    "HELP": 0x75,
    "PROPS": 0x76,
    "FRONT": 0x77,
    "STOP": 0x78,
    "AGAIN": 0x79,
    "UNDO": 0x7a,
    "CUT": 0x7b,
    "COPY": 0x7c,
    "PASTE": 0x7d,
    "FIND": 0x7e,
    "MUTE": 0x7f,
    "VOLUMEUP": 0x80,
    "VOLUMEDOWN": 0x81,
    "KPCOMMA": 0x85,
    "RO": 0x87,
    "KATAKANAHIRAGANA": 0x88,
    "YEN": 0x89,
    "HENKAN": 0x8a,
    "MUHENKAN": 0x8b,
    "KPJPCOMMA": 0x8c,
    "HANGEUL": 0x90,
    "HANJA": 0x91,
    "KATAKANA": 0x92,
    "HIRAGANA": 0x93,
    "ZENKAKUHANKAKU": 0x94,
    "KPLEFTPAREN": 0xb6,
    "KPRIGHTPAREN": 0xb7,
    "LEFTCTRL": 0xe0,
    "LEFTSHIFT": 0xe1,
    "LEFTALT": 0xe2,
    "LEFTMETA": 0xe3,
    "RIGHTCTRL": 0xe4,
    "RIGHTSHIFT": 0xe5,
    "RIGHTALT": 0xe6,
    "RIGHTMETA": 0xe7,
    "MEDIA_PLAYPAUSE": 0xe8,
    "MEDIA_STOPCD": 0xe9,
    "MEDIA_PREVIOUSSONG": 0xea,
    "MEDIA_NEXTSONG": 0xeb,
    "MEDIA_EJECTCD": 0xec,
    "MEDIA_VOLUMEUP": 0xed,
    "MEDIA_VOLUMEDOWN": 0xee,
    "MEDIA_MUTE": 0xef,
    "MEDIA_WWW": 0xf0,
    "MEDIA_BACK": 0xf1,
    "MEDIA_FORWARD": 0xf2,
    "MEDIA_STOP": 0xf3,
    "MEDIA_FIND": 0xf4,
    "MEDIA_SCROLLUP": 0xf5,
    "MEDIA_SCROLLDOWN": 0xf6,
    "MEDIA_EDIT": 0xf7,
    "MEDIA_SLEEP": 0xf8,
    "MEDIA_COFFEE": 0xf9,
    "MEDIA_REFRESH": 0xfa,
    "MEDIA_CALC": 0xfb,
}

def struct_to_string(self):
    s = ""
    s += f"{type(self).__name__}(\n"
    for f in self._fields_:
        s += f"    .{f[0]} = {getattr(self, f[0])},\n"
    s += f")"
    return s

class PacketHeader(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("type", ctypes.c_uint8),
        ("packet_number", ctypes.c_uint8),
        ("payload_length", ctypes.c_uint16),
    ]

    def __repr__(self):
        return struct_to_string(self)

class GetInfo(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("protocol_version", ctypes.c_uint8),
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
        return struct_to_string(self)

class GetMacro(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("macro_type", ctypes.c_uint8),
        ("padding", ctypes.c_uint8),
        ("length", ctypes.c_uint16),
        ("string", ctypes.c_char * 32),
    ]

    def __repr__(self):
        return struct_to_string(self)

PAYLOAD_SIZE = PACKET_SIZE - ctypes.sizeof(PacketHeader)

def round64(x):
    if x == 0:
        return 64
    return (x + 63) & ~63

def u16(buf, offset=0):
    return struct.unpack("<H", buf[offset:offset+2])[0]

def u32(buf, offset=0):
    return struct.unpack("<I", buf[offset:offset+4])[0]

class Message:
    class Status(enum.Enum):
        Incomplete = 0
        Complete = 1
        Unexpected = 2

    def __init__(self, message_type: int, length: int, data: array):
        self.message_type = message_type
        self.data = data
        self.length = length
        self.next_packet_number = 1

    @staticmethod
    def from_packet(packet: array):
        header = PacketHeader.from_buffer(packet)

        bytes_in_packet = min(header.payload_length, PAYLOAD_SIZE)
        offset = ctypes.sizeof(PacketHeader)

        return Message(header.type, header.payload_length, packet[offset:offset+bytes_in_packet])

    def is_complete(self):
        return len(self.data) == self.length

    def add_packet(self, packet: array):
        header = PacketHeader.from_buffer(packet)
        if header.type != self.message_type:
            return Message.Status.Unexpected

        if header.packet_number != self.next_packet_number:
            return Message.Status.Unexpected

        bytes_remaining = self.length - len(self.data)
        bytes_in_packet = min(bytes_remaining, PAYLOAD_SIZE)
        offset = ctypes.sizeof(PacketHeader)
        self.data.extend(packet[offset:offset+bytes_in_packet])

        self.next_packet_number += 1

        if self.is_complete():
            return Message.Status.Complete

        return Message.Status.Incomplete

class KBConfig:
    def __init__(self):
        self.device = usb.core.find(idVendor=0x7083)
        cfg = self.device.get_active_configuration() # pyright: ignore[reportOptionalMemberAccess, reportAttributeAccessIssue]
        interface = cfg[(3, 0)] # pyright: ignore[reportIndexIssue]
        self.ep_in = interface[0]
        self.ep_out = interface[1]

    def drain_in_packets(self):
        try:
            while True:
                self.ep_in.read(PACKET_SIZE, timeout=1)
        except:
            return

    def wait_for_message(self, timeout_ms = 1000):
        message = None
        start_time = datetime.now()
        elapsed_time = timedelta()
        while True:
            try:
                packet = self.ep_in.read(PACKET_SIZE, timeout=1)
                if message == None:
                    if PacketHeader.from_buffer(packet).packet_number == 0:
                        message = Message.from_packet(packet)
                        if message.is_complete():
                            return message
                else:
                    status = message.add_packet(packet)
                    if status == Message.Status.Complete:
                        return message
                    elif status == Message.Status.Unexpected:
                        message = None
            except:
                sleep(0.001)

            elapsed_time += datetime.now() - start_time
            if (elapsed_time.total_seconds() * 1000) > timeout_ms:
                raise TimeoutError

            start_time = datetime.now()

    @staticmethod
    def prepare_message(message_type: int, data: bytearray | bytes | None = None):
        if data is None:
            data = bytearray([])

        message_size = max(1, math.ceil( len(data) / (PACKET_SIZE - ctypes.sizeof(PacketHeader)) )) * PACKET_SIZE
        message_buffer = array("B", [0] * message_size)
        packet_number = 0
        data_offset = 0
        header = PacketHeader()
        message_offset = 0

        while message_offset < message_size:
            message_offset = packet_number * PACKET_SIZE
            header.type = message_type
            header.packet_number = packet_number
            header.payload_length = len(data)

            message_buffer[message_offset:ctypes.sizeof(PacketHeader)] = array("B", bytes(header))

            packet_payload_size = min(PAYLOAD_SIZE, len(data) - data_offset)
            message_offset += ctypes.sizeof(PacketHeader)
            message_buffer[message_offset:message_offset+packet_payload_size] = array("B", data[data_offset:packet_payload_size])

            packet_number += 1
            data_offset += PAYLOAD_SIZE
            message_offset += PACKET_SIZE

        return message_buffer

    def get_info(self):
        self.ep_out.write(KBConfig.prepare_message(
            KB_CONFIG_MSG_GET_INFO | KB_CONFIG_MSG_TYPE_REQ,
            bytearray()
        ))

        response = self.wait_for_message()
        assert(response.message_type == KB_CONFIG_MSG_GET_INFO | KB_CONFIG_MSG_TYPE_RES)
        info = GetInfo.from_buffer(response.data)
        print(info)

        self.info = info

    def get_macro(self, index):
        self.ep_out.write(KBConfig.prepare_message(
            KB_CONFIG_MSG_GET_MACRO | KB_CONFIG_MSG_TYPE_REQ,
            bytearray([index])
        ))

        response = self.wait_for_message()
        assert(response.message_type == KB_CONFIG_MSG_GET_MACRO | KB_CONFIG_MSG_TYPE_RES)
        macro = GetMacro.from_buffer(response.data)
        print(macro)

        return response.data

    def set_macro(self, index, string: bytearray | bytes):
        self.ep_out.write(KBConfig.prepare_message(
            KB_CONFIG_MSG_SET_MACRO | KB_CONFIG_MSG_TYPE_REQ,
            bytes([index, 0x01, 0x00]) + (len(string) + 1).to_bytes(2, "little") + string + b'\x00'
        ))

    def get_layout(self, layer: int):
        self.ep_out.write(KBConfig.prepare_message(
            KB_CONFIG_MSG_GET_LAYOUT | KB_CONFIG_MSG_TYPE_REQ,
            bytearray([layer])
        ))

        message = self.wait_for_message()
        assert(message.message_type == KB_CONFIG_MSG_GET_LAYOUT | KB_CONFIG_MSG_TYPE_RES)
        assert(len(message.data) == self.info.row_count * self.info.column_count * 4)

        keymap = []
        offset = 0
        for _ in range(self.info.row_count):
            row = []
            for _ in range(self.info.column_count):
                keymap_entry_value = u32(message.data, offset)
                row.append(keymap_entry_value)
                offset += 4
            keymap.append(row)
        return keymap

    def set_key(self, layer: int, row: int, col: int, key: int):
        self.ep_out.write(KBConfig.prepare_message(
            KB_CONFIG_MSG_SET_KEY | KB_CONFIG_MSG_TYPE_REQ,
            bytearray([layer, row, col, 0xff]) + struct.pack("<I", key)
        ))

    def commit_to_flash(self):
        self.ep_out.write(KBConfig.prepare_message(
            KB_CONFIG_MSG_COMMIT | KB_CONFIG_MSG_TYPE_REQ,
            bytearray([0x4c, 0x4f, 0x4f, 0x43, KB_CONFIG_COMMIT_OP_SAVE])
        ))

    def erase_flash_config(self):
        self.ep_out.write(KBConfig.prepare_message(
            KB_CONFIG_MSG_COMMIT | KB_CONFIG_MSG_TYPE_REQ,
            bytearray([0x4c, 0x4f, 0x4f, 0x43, KB_CONFIG_COMMIT_OP_ERASE])
        ))

    def reset_to_bootloader(self):
        self.ep_out.write(KBConfig.prepare_message(KB_CONFIG_MSG_RESET_TO_BL | KB_CONFIG_MSG_TYPE_REQ))

    def dump_config(self, filename: str):
        self.ep_out.write(KBConfig.prepare_message(
            KB_CONFIG_MSG_DUMP_CONFIG | KB_CONFIG_MSG_TYPE_REQ
        ))

        message = self.wait_for_message()
        assert(message.message_type == KB_CONFIG_MSG_DUMP_CONFIG | KB_CONFIG_MSG_TYPE_RES)
        assert(len(message.data) == 4096)

        with open(filename, "wb") as f:
            f.write(message.data.tobytes())

    def set_combo(self, index, keys, key_out):
        keys = keys + ([0] * (4 - len(keys)))
        payload = index.to_bytes(1, 'little')
        for k in keys:
            payload += k.to_bytes(4, "little")
        payload += key_out.to_bytes(4, "little")

        self.ep_out.write(KBConfig.prepare_message(
            KB_CONFIG_MSG_SET_COMBO | KB_CONFIG_MSG_TYPE_REQ,
            payload
        ))

def print_layer_raw(layer: List[List[int]]):
    for row in layer:
        for key in row:
            if key == 0:
                print("           ", end='')
            elif key == 1:
                print("__________ ", end='')
            else:
                print(f"0x{key:08x} ", end='')
        print("")

def apply_mods_to_kc(kc: int, mods: int):
    kc_str = kc_to_name.get(kc, '')
    for name, mask in mod_name_to_bit.items():
        if (mods & mask) == mask:
            kc_str = f"{name}({kc_str})"
    return kc_str

def kc_to_str(key: int):
    if key == 0:
        return ""
    if key == 1:
        return "____"
    mods = (key & KEY_MODS_MASK) >> KEY_MODS_SHIFT
    kc_str = apply_mods_to_kc(key & KC_MASK, mods)
    return f"KC({kc_str})"

def th_to_str(key: int):
    mods = (key & ENTRY_ARG4_MASK) >> ENTRY_ARG4_SHIFT
    kc_str = apply_mods_to_kc(key & KC_MASK, 0)
    hkc_str = apply_mods_to_kc((key & ENTRY_ARG8_MASK) >> ENTRY_ARG8_SHIFT, mods)
    return f"TH({kc_str},{hkc_str})"

def dt_to_str(key: int):
    mods = (key & ENTRY_ARG4_MASK) >> ENTRY_ARG4_SHIFT
    kc_str = apply_mods_to_kc(key & KC_MASK, 0)
    dkc_str = apply_mods_to_kc((key & ENTRY_ARG8_MASK) >> ENTRY_ARG8_SHIFT, mods)
    return f"DT({kc_str},{dkc_str})"

def la_to_str(key: int):
    com = f"{(key & ENTRY_ARG8_MASK) >> ENTRY_ARG8_SHIFT:02x}"
    layer = key & KC_MASK
    return f"LA({com},{layer})"

def kbc_to_str(key: int):
    com = f"{key & 0xffff:04x}"
    return f"KBC({com})"

def print_layer(layer: List[List[int]]):
    layer_strs = []
    longest = 0
    for row in layer:
        row_strs = []
        for key in row:
            s = ''
            if (key & ENTRY_TYPE_MASK) == ENTRY_TYPE_KC:
                s = kc_to_str(key)
            elif (key & ENTRY_TYPE_MASK) == ENTRY_TYPE_LAYER:
                s = la_to_str(key)
            elif (key & ENTRY_TYPE_MASK) == ENTRY_TYPE_TAPHOLD:
                s = th_to_str(key)
            elif (key & ENTRY_TYPE_MASK) == ENTRY_TYPE_DOUBLE_TAP:
                s = dt_to_str(key)
            elif (key & ENTRY_TYPE_MASK) == ENTRY_TYPE_MACRO:
                s = "MA"
            elif (key & ENTRY_TYPE_MASK) == ENTRY_TYPE_KBC:
                s = kbc_to_str(key)
            elif (key & ENTRY_TYPE_MASK) == ENTRY_TYPE_CC:
                s = "CC"
            elif (key & ENTRY_TYPE_MASK) == ENTRY_TYPE_MOUSE:
                s = "MO"
            longest = max(longest, len(s))
            row_strs.append(s)
        layer_strs.append(row_strs)

    keys_bar = "+" + "+".join([ '-' * longest for _ in range(len(layer[0])) ]) + "+"
    for row_strs in layer_strs:
        print(keys_bar)
        print("|", end='')
        for key in row_strs:
            print(f"{key.center(longest)}", end='|')
        print("")
    print(keys_bar)

if __name__ == "__main__":
    kb = KBConfig()
    kb.drain_in_packets()
    # kb.erase_flash_config()

    # kb.get_info()

    kb.set_macro(1, b"test")
    kb.set_key(3, 1, 11, 0x40000001)

    # sleep(2)
    # kb.commit_to_flash()

    # data = kb.dump_config("config_dump.bin")

    # kb.get_macro(0)
    # with open("macro.bin", "wb") as f:
    #     f.write(data)
    # kb.set_key(layer=0, row=2, col=0, key=0x1B)
    # kb.commit_to_flash()

    kb.get_info()
    layers = []
    print("")
    for layer in range(kb.info.layer_count):
        print(f"Layer {layer}:")
        layer = kb.get_layout(layer)
        layers.append(layer)
        print_layer(layer)
        print("")

