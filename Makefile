PEBBLE=/home/h-morishita/pebble-dev/PebbleSDK-current/bin/pebble

all:
	$(PEBBLE) build

clean:
	$(PEBBLE) clean

emu:
	$(PEBBLE) install --emulator basalt
