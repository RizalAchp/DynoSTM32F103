#SHELL := /bin/bash
#PATH := /usr/local/bin:$(PATH)
PIO := $(HOME)/.platformio/penv/bin/pio -f -c vim

all:
	$(PIO) run

upload:
	$(PIO) run --target upload -v

clean:
	$(PIO) run --target clean -v

program:
	$(PIO) run --target program -v

uploadfs:
	$(PIO) run --target uploadfs -v

update:
	$(PIO) update -v

monitor:
	$(PIO) device monitor -v
