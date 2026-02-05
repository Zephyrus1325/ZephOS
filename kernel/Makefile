TARGET = kernel.elf
OBJ_DIR = build

# Ferramentas
CC = arm-none-eabi-gcc
AS = arm-none-eabi-as

# Flags (Sincronizadas com o CPULator)
CFLAGS = -mcpu=cortex-a9 -mfloat-abi=softfp -mfpu=neon-fp16 \
         -ffreestanding -nostdlib -g -O0 -I.
			
ASFLAGS =  -march=armv7-a -mcpu=cortex-a9 -mfpu=neon-fp16 -mfloat-abi=softfp --gdwarf2

# Localizar fontes (Kernel e subpastas)
SRCS_C = $(shell find -name "*.c")
SRCS_S = $(shell find -name "*.s")

# Mapear fontes para objetos dentro da pasta build/
# Exemplo: kernel/main.c -> build/main.o
OBJS = $(addprefix $(OBJ_DIR)/, $(notdir $(SRCS_C:.c=.o)))
OBJS += $(addprefix $(OBJ_DIR)/, $(notdir $(SRCS_S:.s=.o)))

# VPATH ajuda o Make a encontrar os arquivos nas subpastas
VPATH = $(dir $(SRCS_C) $(SRCS_S))

all: $(OBJ_DIR) $(TARGET)

# Criar a pasta build se não existir
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Linkagem final
$(TARGET): $(OBJS)
	$(CC) -T kernel.ld $(OBJS) -o $(TARGET) $(CFLAGS)
	@echo "Sucesso! O arquivo $(TARGET) foi gerado."

# Compilação C -> Objeto
$(OBJ_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compilação Assembly -> Objeto
$(OBJ_DIR)/%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(TARGET)
	@echo "Limpeza concluída."