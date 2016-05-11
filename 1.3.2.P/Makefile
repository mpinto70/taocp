.PHONY: clean primes

.SUFFIXES:
.SUFFIXES: .mixal .mix .asm .cpp .c .o .exe

.mixal.mix:
	mixasm $<

.o.exe:
	gcc -o $@ $<

.asm.o:
	yasm -f elf64 -g dwarf2 -l $<.lst $<

primes: primes.mix
	mixvm -r -t $<

primes-asm: primes.exe
	./$<

clean:
	rm -f \
		*.mix \
		*.lst \
		*.o \
		*.exe \
