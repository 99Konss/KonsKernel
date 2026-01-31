/* kernel/linker.ld - KonsKernel */
ENTRY(kernel_main)

SECTIONS {
    . = 1M;
    
    .multiboot : {
        *(.multiboot)
    }
    
    .text : { *(.text) }
    .rodata : { *(.rodata) }
    .data : { *(.data) }
    .bss : { 
        *(COMMON)
        *(.bss)
    }
}
