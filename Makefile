DEST = ../backup

default: prepare clean build

prepare:
	mkdir -p bin

build:
	gcc sender.c -o bin/sender
	gcc receiver.c -o bin/receiver
	./bin/receiver

clean:
	- rm -f bin/*

bb:
	gcc sender.c -o sender
	gcc receiver.c -o receiver
	./receiver

gen:

	for c in {A..Z}; do printf '%*s\n' 1024 '' | tr ' ' "$c"; done > output.txt
