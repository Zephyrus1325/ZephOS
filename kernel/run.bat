clear
make
qemu-system-arm -M vexpress-a9 -m 512M -kernel kernel.elf -serial vc -monitor stdio -drive file=disk.img,if=sd,format=raw
REM -S -gdb tcp::1234