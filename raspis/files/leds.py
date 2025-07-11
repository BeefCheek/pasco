import argparse
import board
import neopixel
from colorsys import hsv_to_rgb

PIN = board.D10
ORDER = neopixel.RGBW

VERT_DO = hsv_to_rgb(0.1, 0.8, 255) # ok
BLANC_GRIS = hsv_to_rgb(0.1, 0.7, 255)
ORANGE = hsv_to_rgb(0.06, 0.95, 255) # ok
VIOLET = hsv_to_rgb(0.7, 1., 255) + hsv_to_rgb(0., 1., 255) # only 6 last leds

def launch(hue: float, sat: float, npix: int, brightness: float):
    pixels = neopixel.NeoPixel(
        PIN, npix, brightness=brightness, auto_write=False, pixel_order=ORDER
    )
    color = (*tuple(int(c) for c in hsv_to_rgb(hue, sat, 255)), 255)
    pixels.fill(color)
    pixels.show()

parser = argparse.ArgumentParser(description="Launch LEDs.")
parser.add_argument("-b", "--brightness", type=float, help="Overall brightness.")
parser.add_argument("-n", "--num", type=int, help="Number of LEDs.")
parser.add_argument("-c", "--hue", type=float, help="HUE.")
parser.add_argument("-s", "--sat", type=float, help="Saturation.")
args = parser.parse_args()
launch(hue=args.hue, sat=args.sat, npix=args.num, brightness=args.brightness)
