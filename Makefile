DEST = ../backup

default: prepare clean build

prepare:
	mkdir -p bin

build:
	gcc sender.c -o bin/sender
	gcc receiver.c -o bin/receiver
	./bin/receiver

clean:
	- rm receiver sender output2.png

bb: clean
	gcc sender.c -o sender
	gcc receiver.c -o receiver
send:
	./sender file.png
rec: bb
	./receiver 
	cmp file.png output2.png

gen:

	for c in {A..Z}; do printf '%*s\n' 1024 '' | tr ' ' "$c"; done > output.txt
