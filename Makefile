PEBBLE=pebble

all:
	$(PEBBLE) build

clean:
	$(PEBBLE) clean

emu:
	$(PEBBLE) install --emulator basalt
