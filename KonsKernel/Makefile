# KonsKernel/Makefile (MIT start.asm)
CC = gcc
ASM = nasm
LD = ld
CFLAGS = -m32 -ffreestanding -fno-stack-protector -nostdlib -Wall -w -std=gnu99
ASMFLAGS = -f elf32
LDFLAGS = -m elf_i386 -T kernel/linker.ld -nostdlib

# Quellcode-Dateien
KERNEL_C_SOURCES = kernel/kernel.c
KERNEL_ASM_SOURCES = kernel/start.asm    # NEU: start.asm hinzugefügt!
BOOT_SOURCE = boot/boot.asm

# Objektdateien
KERNEL_C_OBJS = $(KERNEL_C_SOURCES:.c=.o)
KERNEL_ASM_OBJS = $(KERNEL_ASM_SOURCES:.asm=.o)
KERNEL_OBJS = $(KERNEL_ASM_OBJS) $(KERNEL_C_OBJS)  # WICHTIG: Assembly zuerst!
BOOT_OBJ = boot/boot.o

TARGET = kernel.bin
ISO = konskernel.iso

.PHONY: all clean run iso

all: $(TARGET)

# Finaler Kernel - BOOT_OBJ AUSKOMMENTIERT oder entfernt, wenn du nur Multiboot benutzt
$(TARGET): $(KERNEL_OBJS)  # $(BOOT_OBJ) entfernt wenn du QEMU -kernel benutzt
	$(LD) $(LDFLAGS) -o $@ $(KERNEL_OBJS)

# Bootloader kompilieren (falls benötigt)
boot/boot.o: $(BOOT_SOURCE)
	$(ASM) -f bin $< -o $@  # ACHTUNG: Für Bootloader oft -f bin statt elf32!

# Start.asm (32-bit Kernel Entry) kompilieren - NEU!
kernel/start.o: kernel/start.asm
	$(ASM) $(ASMFLAGS) $< -o $@

# Kernel C-Dateien
kernel/kernel.o: kernel/kernel.c
	$(CC) $(CFLAGS) -c $< -o $@

# ISO für QEMU/CD
iso: $(TARGET)
	mkdir -p iso/boot/grub
	cp $(TARGET) iso/boot/
	echo 'menuentry "KonsKernel" { multiboot /boot/kernel.bin }' > iso/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO) iso
	rm -rf iso

QEMU := $(shell which qemu-system-i386 2>/dev/null || which qemu-system-x86_64 2>/dev/null)

# QEMU mit Multiboot (empfohlen für deinen jetzigen Code)
run: $(TARGET)
	$(QEMU) -kernel $(TARGET)

# QEMU mit ISO
run-iso: iso
	qemu-system-i386 -cdrom $(ISO)

# Aufräumen
clean:
	rm -f $(TARGET) $(ISO) $(BOOT_OBJ) kernel/*.o boot/*.o
