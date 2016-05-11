.PHONY: clean primes

.SUFFIXES:
.SUFFIXES: .mixal .mix

.mixal.mix:
	mixasm $<

primes: primes.mix
	mixvm -r -t $<

clean:
	rm -f \
		*.mix \
