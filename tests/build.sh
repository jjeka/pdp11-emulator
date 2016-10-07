set -e

if [ "$#" -ne 1 ]; then
    echo "Illegal number of parameters"
	exit 1
fi

mkdir -p obj
pdp11-aout-gcc -O0 -m40 -nostdlib -S -o obj/$1.s $1
pdp11-aout-as obj/$1.s -o obj/$1.o
pdp11-aout-ld -T ldaout.cmd obj/$1.o -o rom.bin
truncate -s 32768 rom.bin
cp rom.bin ../rom.bin
