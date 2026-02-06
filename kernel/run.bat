make
qemu-system-arm -M vexpress-a9 -m 512M -kernel kernel.elf -serial vc -monitor stdio 
-S -gdb tcp::1234