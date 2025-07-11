import argparse
import serial
from pythonosc.udp_client import SimpleUDPClient
from collections import deque

parser = argparse.ArgumentParser(description="Launch OSC Proxy.")
parser.add_argument("-d", "--device", type=str, help="Serial device to read from.")
parser.add_argument("-p", "--path", type=str, help="Output sensor number")
parser.add_argument("-t", "--threshold", type=int, help="Sensor threshold")
parser.add_argument("-n", "--nsensors", type=int, help="Input sensors count")
parser.add_argument("-v", "--verbose", action="store_true")
args = parser.parse_args()

client = SimpleUDPClient("127.0.0.1", 1234)
sensor = "/rnbo/inst/0/messages/in/" + args.path

with serial.Serial(args.device, 115200) as ser:
    while True:
        line =  ser.readline()
        try:
            values = [int(v) for v in line.decode().split(" ")[:args.nsensors]]
            ovalues = [1 if int(v) > args.threshold else 0 for v in values]
            if args.verbose and (sum(ovalues) > 0):
                print(values)
            client.send_message(sensor, ovalues)
        except:
            print("bad line")
