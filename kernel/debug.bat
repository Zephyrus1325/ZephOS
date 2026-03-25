cls
arm-none-eabi-gdb kernel.elf -ex "target remote:1234" -ex "break main.c:30" -ex "continue"
REM -S -gdb tcp::1234
REM arm-none-eabi-objdump -d kernel.elf > dump.txt 