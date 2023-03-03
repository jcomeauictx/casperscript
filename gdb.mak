gdb:	.gdbinit
	gdb bin/gs
gs.dsm: bin/gs
	objdump --disassemble $< > $@
gs.xxd: bin/gs
	xxd $< $@
%.s: %.c
	gcc -S -fverbose-asm -g $(CCFLAGS) $< -o $@
