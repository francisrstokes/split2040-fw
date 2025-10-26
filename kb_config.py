import usb.core
import usb.util

# Need to add a udev rule like:
# SUBSYSTEMS=="usb", ATTRS{idVendor}=="7083", ATTRS{idProduct}=="0003", GROUP="plugdev", MODE="0777"

device = usb.core.find(idVendor=0x7083)
cfg = device.get_active_configuration()
interface = cfg[(3, 0)]
ep_in = interface[0]
ep_out = interface[1]

buffer = bytearray([0] * 64)

ep_out.write(buffer)