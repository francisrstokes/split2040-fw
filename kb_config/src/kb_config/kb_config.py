import usb.core
from typing import List
from time import sleep
from array import array
import struct
import ctypes
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
KB_CONFIG_MSG_GET_RING_BUFFER_DATA  = (0x0B)

KB_CONFIG_COMMIT_OP_CANCEL          = (0)
KB_CONFIG_COMMIT_OP_SAVE            = (1)
KB_CONFIG_COMMIT_OP_ERASE           = (2)

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

        return info

    def get_macro(self, index):
        self.ep_out.write(KBConfig.prepare_message(
            KB_CONFIG_MSG_GET_MACRO | KB_CONFIG_MSG_TYPE_REQ,
            bytearray([index])
        ))

        response = self.wait_for_message()
        assert(response.message_type == KB_CONFIG_MSG_GET_MACRO | KB_CONFIG_MSG_TYPE_RES)
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

        keys: List[int] = []
        offset = 0
        for offset in range(0, len(message.data), 4):
            keymap_entry_value = int.from_bytes(message.data[offset:offset+4], "little")
            keys.append(keymap_entry_value)
        return keys

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

    def set_combo(self, index: int, keys: List[int], key_out: int, max_keys_per_combo=4):
        keys = keys + ([0] * (max_keys_per_combo - len(keys)))
        payload = index.to_bytes(1, 'little')
        for k in keys:
            payload += k.to_bytes(4, "little")
        payload += key_out.to_bytes(4, "little")

        self.ep_out.write(KBConfig.prepare_message(
            KB_CONFIG_MSG_SET_COMBO | KB_CONFIG_MSG_TYPE_REQ,
            payload
        ))

    def get_ring_buffer_data(self):
        self.ep_out.write(KBConfig.prepare_message(
            KB_CONFIG_MSG_GET_RING_BUFFER_DATA | KB_CONFIG_MSG_TYPE_REQ,
            bytearray([])
        ))

        message = self.wait_for_message()
        assert(message.message_type == KB_CONFIG_MSG_GET_RING_BUFFER_DATA | KB_CONFIG_MSG_TYPE_RES)
        return message.data, message.length
