# Sample fxcg-tools project Makefile
CC=sh3eb-elf-gcc
CFLAGS=-m4-nofpu -mb -Os -mhitachi -Wall -nostdlib -I../include -lfxcg -lgcc -L../lib
LDFLAGS=$(CFLAGS) -T../lib/prizm.ld -Wl,static -Wl,-gc-sections
OBJCOPY=sh3eb-elf-objcopy
MKFXI=python ../fx-imglib/mkfxi.py
MKG3A=../mkg3a/mkg3a

OBJECTS=main.o resources.o
ADDIN=FishCakes

all: $(ADDIN).g3a

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<
.s.o:
	$(CC) -c $(CFLAGS) -o $@ $<
# mkfxi also supports .bmp input
.png.fxi:
	$(MKFXI) $< $@

resources.o: foo.fxi bar.fxi
	$(OBJCOPY) -O elf32-big -I binary \
	--rename-section .data=.rodata,alloc,load,readonly,data,contents \
	-redefine-syms=resources.lst \
	$< $@
	
$(ADDIN).bin: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $<
$(ADDIN).g3a: $(ADDIN).bin
	$(MKG3A) -n ":$(ADDIN)" $< $@