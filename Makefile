default: prepare clean build

prepare:
	mkdir -p bin/sender
	mkdir -p bin/receiver

build:
	gcc sender.c -o bin/sender/sender
	gcc receiver.c -o bin/receiver/receiver

clean:
	- rm bin/sender/sender bin/receiver/*

bb: clean
	gcc sender.c -o sender
	gcc receiver.c -o receiver


test2:
	cd bin/sender/ && ./sender file.png
test:
	cd bin/receiver && ./receiver
cmp:
	cmp bin/sender/file.png bin/receiver/file.png
	ls -l bin/receiver/file.png
gen:
	for c in {A..Z}; do printf '%*s\n' 1024 '' | tr ' ' "$c"; done > output.txt
