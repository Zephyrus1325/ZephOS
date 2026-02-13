clear
make
qemu-system-arm -M vexpress-a9 -m 512M -kernel kernel.elf -serial stdio -monitor vc
REM -S -gdb tcp::1234