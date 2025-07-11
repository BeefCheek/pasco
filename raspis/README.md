sudo python3 /home/pi/osc.py -d /dev/ttyUSB0 -p sensor1 -n 9 -v
sudo python3 leds.py -c 255 255 255 -b 0 -n 7

default sound card
cat /proc/asound/cards
sudo micro /etc/asound.conf

    print("Enabling I2C")
    shell.run_command("sudo raspi-config nonint do_i2c 0")
    print("Enabling SPI")
    shell.run_command("sudo raspi-config nonint do_spi 0")
    print("Enabling Serial")
    shell.run_command("sudo raspi-config nonint do_serial_hw 0")
    print("Enabling SSH")
    shell.run_command("sudo raspi-config nonint do_ssh 0")
    print("Enabling Camera")
    shell.run_command("sudo raspi-config nonint do_camera 0")
    print("Disable raspi-config at Boot")
    shell.run_command("sudo raspi-config nonint disable_raspi_config_at_boot 0")

sudo esekeyd /etc/esekeyd.conf /dev/input/event0