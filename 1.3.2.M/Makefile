.PHONY: clean maximum

.SUFFIXES:
.SUFFIXES: .mixal .mix .asm .cpp .c .o .exe

.mixal.mix:
	mixasm $<

.o.exe:
	gcc -o $@ $<

.asm.o:
	yasm -f elf64 -g dwarf2 -l $<.lst $<

maximum: maximum.mix
	cp ./devices/tape0.dev ~/.mdk
	mixvm -r -t $<

maximum-asm: maximum.exe
	./$<

fill: fill.mix
	mixvm -r -t $<

clean:
	rm -f \
		*.mix \
		*.lst \
		*.o \
		*.exe \
