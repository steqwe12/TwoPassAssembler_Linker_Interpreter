all: assembler linker emulator

assembler: src/assembler.cpp src/helpers.cpp src/bitno.cpp src/parser.cpp src/lekser.cpp inc/helpers.hpp
	g++ -g -Wno-write-strings src/assembler.cpp src/helpers.cpp src/bitno.cpp src/parser.cpp src/lekser.cpp -o asembler

src/lekser.cpp: misc/lekser.l inc/helpers.hpp
	flex -o src/lekser.cpp misc/lekser.l

src/parser.cpp: misc/parser.y misc/lekser.l inc/helpers.hpp
	bison -o src/parser.cpp --defines=inc/parser.hpp misc/parser.y

linker: src/linker.cpp
	g++ -Wno-write-strings src/linker.cpp -o linker

emulator: src/emulator.cpp
	g++ -Wno-write-strings src/emulator.cpp -o emulator

clean:
	rm -rf *.o *.hex src/lekser.cpp src/parser.cpp inc/parser.hpp asembler error_log.txt linker emulator error_emul.txt

.PHONY: clean

debug: assembler
	gdb --args ./asembler -o handler.o handler.s 