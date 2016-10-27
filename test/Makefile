TGT=../rom.bin
BLD=obj
BIN=$(BLD)/$(shell basename $(TGT))
SIZE=32768
TOOLCHAIN_PREFIX=pdp11-aout-
CC=$(TOOLCHAIN_PREFIX)gcc
LD=$(TOOLCHAIN_PREFIX)ld
C_FLAGS=-O0 -m40 -nostdlib -Werror -Wall -Wextra
LD_FLAGS=-T ldaout.cmd -M

SRC=$(shell find * -type f -name '*\.[sc]')
HDR=$(shell find * -type f -name '*\.h')

OBJ=$(foreach x, $(SRC), $(BLD)/$(x).o)

$(TGT): $(BIN)
	cp $(BIN) $(TGT)
	printf '\xA0\x00\xA0\x00\xA0\x00\xA0\x00\xA0\x00\xA0\x00\xA0\x00\xA0\x00' | dd of=$(TGT) bs=1 seek=0 count=16 conv=notrunc
	ls -l $(TGT)
	truncate -s $(SIZE) $(TGT)
	
$(BIN): $(OBJ)
	$(LD) $(LD_FLAGS) -o $(BIN) $(OBJ)

$(BLD)/%.o: % $(HDR)
	mkdir -p $(shell dirname $@)
	$(CC) $(C_FLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ) $(TGT) $(BIN)
