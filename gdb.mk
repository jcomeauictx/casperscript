gs.dsm: bin/gs
	objdump --disassemble $< > $@
gs.xxd: bin/gs
	xxd $< $@
gdb:	.gdbinit
	gdb bin/gs
%.s: %.c
	gcc -S -fverbose-asm -g $(CCFLAGS) $< -o $@
