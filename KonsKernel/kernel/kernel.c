/* kernel.c - KonsKernel v1.4.0 */
/* Complete Kernel with SAFE Memory Management */
/* Shoutout to DeepSeek for the help! */

#include <stdint.h>

char* strstr(const char* haystack, const char* needle);

#define NULL ((void*)0)  // <-- DIESE ZEILE EINFÜGEN

// ========================
// 1. MULTIBOOT HEADER (for GRUB/QEMU)
// ========================
__attribute__((section(".multiboot")))
const unsigned int multiboot_header[] = {
    0x1BADB002,                      // Magic number
    0x00000003,                      // Flags: align modules + memory map
    -(0x1BADB002 + 0x00000003)       // Checksum
};

// ========================
// 2. EXTERNE ASSEMBLY STUBS DECLARATIONS (HIER!)
// ========================

// Assembly Stubs für Interrupts - MUSS hier stehen!
extern void _isr0(void);
extern void _isr1(void);
extern void _isr2(void);
extern void _isr3(void);
extern void _isr4(void);
extern void _isr5(void);
extern void _isr6(void);
extern void _isr7(void);
extern void _isr8(void);
extern void _isr9(void);
extern void _isr10(void);
extern void _isr11(void);
extern void _isr12(void);
extern void _isr13(void);
extern void _isr14(void);
extern void _isr15(void);
extern void _isr16(void);
extern void _isr17(void);
extern void _isr18(void);
extern void _isr19(void);
extern void _isr20(void);
extern void _isr21(void);
extern void _isr22(void);
extern void _isr23(void);
extern void _isr24(void);
extern void _isr25(void);
extern void _isr26(void);
extern void _isr27(void);
extern void _isr28(void);
extern void _isr29(void);
extern void _isr30(void);
extern void _isr31(void);

extern void _irq0(void);
extern void _irq1(void);
extern void _irq2(void);
extern void _irq3(void);
extern void _irq4(void);
extern void _irq5(void);
extern void _irq6(void);
extern void _irq7(void);
extern void _irq8(void);
extern void _irq9(void);
extern void _irq10(void);
extern void _irq11(void);
extern void _irq12(void);
extern void _irq13(void);
extern void _irq14(void);
extern void _irq15(void);

// ========================
// 2. DEFINES & CONSTANTS
// ========================
#define VIDEO_MEMORY 0xB8000         // VGA text buffer address
#define SCREEN_WIDTH 80              // Characters per line
#define SCREEN_HEIGHT 25             // Lines per screen

#define PAGE_SIZE 4096
#define BITMAP_SIZE 32768            // Für 128MB RAM

// Memory Management
#define HEAP_START 0x300000          // 3MB - SICHERER Start (nach Kernel)
#define HEAP_SIZE 0x100000           // 1MB Heap (klein und sicher)
#define BLOCK_SIZE 16                // Minimale Blockgröße
#define MAX_ALLOCS 64                // Maximale Anzahl an Allokationen

#define IDT_ENTRIES 256
#define IRQ0 32
#define IRQ1 33

// GDT Configuration
#define GDT_ENTRIES 3                // NULL + Kernel Code + Kernel Data
#define SEG_KERNEL_CODE 0x08         // Selector for kernel code segment
#define SEG_KERNEL_DATA 0x10         // Selector for kernel data segment

// Shell Configuration
#define CMD_HISTORY_SIZE 10
#define CMD_BUFFER_SIZE 256

// KFS Konstanten
#define KFS_MAGIC 0x4B46531A  // "KFS" + 0x1A
#define KBLOCK_SIZE 512        // Wie echte Disks
#define MAX_FILES 64          // Maximale Dateien im System
#define MAX_NAME_LEN 28       // Dateinamenlänge
#define MAX_BLOCKS_PER_FILE 16 // Max Blöcke pro Datei

#define RAMDISK_SIZE (4 * 1024 * 1024)
static uint8_t ramdisk[RAMDISK_SIZE];

//KonsEdit
#define EDITOR_WIDTH 78
#define EDITOR_HEIGHT 20
#define MAX_EDIT_SIZE 8192  // 8KB pro Datei

// Color Constants
#define COLOR_BLACK         0x00
#define COLOR_BLUE          0x01
#define COLOR_GREEN         0x02
#define COLOR_CYAN          0x03
#define COLOR_RED           0x04
#define COLOR_MAGENTA       0x05
#define COLOR_BROWN         0x06
#define COLOR_LIGHT_GRAY    0x07
#define COLOR_DARK_GRAY     0x08
#define COLOR_LIGHT_BLUE    0x09
#define COLOR_BLUE_ON_BLUE  0x11
#define COLOR_LIGHT_GREEN   0x0A
#define COLOR_LIGHT_CYAN    0x0B
#define COLOR_LIGHT_RED     0x0C
#define COLOR_LIGHT_MAGENTA 0x0D
#define COLOR_YELLOW        0x0E
#define COLOR_WHITE         0x0F

// Combined colors: (BACKGROUND << 4) | FOREGROUND
#define COLOR_WHITE_ON_BLACK   (COLOR_WHITE)
#define COLOR_RED_ON_BLACK     (COLOR_RED)
#define COLOR_GREEN_ON_BLACK   (COLOR_GREEN)
#define COLOR_BLUE_ON_BLACK    (COLOR_BLUE)
#define COLOR_YELLOW_ON_BLACK  (COLOR_YELLOW)
#define COLOR_CYAN_ON_BLACK    (COLOR_LIGHT_CYAN)

// Blue background colors (your theme)
#define COLOR_WHITE_ON_BLUE    ((COLOR_BLUE << 4) | COLOR_WHITE)
#define COLOR_GREEN_ON_BLUE    ((COLOR_BLUE << 4) | COLOR_GREEN)
#define COLOR_YELLOW_ON_BLUE   ((COLOR_BLUE << 4) | COLOR_YELLOW)
#define COLOR_CYAN_ON_BLUE     ((COLOR_BLUE << 4) | COLOR_LIGHT_CYAN)
#define COLOR_RED_ON_BLUE      ((COLOR_BLUE << 4) | COLOR_RED)
#define COLOR_BLACK_ON_BLUE    ((COLOR_BLUE << 4) | COLOR_BLACK)

// ========================
// THEME SYSTEM (FAST FIX)
// ========================

// AKTUELLES THEME - ÄNDERE NUR HIER!
#define CURRENT_BG COLOR_DARK_GRAY  // ODER COLOR_BLUE für altes Theme

// Automatische Color-Generierung
#define MAKE_COLOR(fg, bg) ((bg << 4) | fg)
#define AUTO_COLOR(fg) MAKE_COLOR(fg, (CURRENT_BG >> 4))

// Theme Colors (wird automatisch angepasst!)
#define TXT_NORMAL    AUTO_COLOR(COLOR_WHITE)
#define TXT_SUCCESS   AUTO_COLOR(COLOR_GREEN)
#define TXT_ERROR     AUTO_COLOR(COLOR_RED)
#define TXT_WARNING   AUTO_COLOR(COLOR_YELLOW)
#define TXT_INFO      AUTO_COLOR(COLOR_LIGHT_CYAN)
#define TXT_CYAN      AUTO_COLOR(COLOR_CYAN)
#define TXT_BLUE      AUTO_COLOR(COLOR_BLUE)

// ========================
// SMART THEME SYSTEM
// ========================

// ★★★ HIER THEME WÄHLEN ★★★
// OPTIONEN:
//   COLOR_BLACK          - Dunkel (Modern)
//   COLOR_DARK_GRAY      - Dark Gray (Sehr professionell)
//   COLOR_LIGHT_GRAY     - Light Gray (Wie moderne Terminals)
//   COLOR_BLUE           - Original Blue
//   COLOR_GREEN          - Matrix Style
//   COLOR_CYAN           - Retro Terminal

#define THEME_BACKGROUND COLOR_DARK_GRAY  // ← EMPFEHLUNG!

// ========================
// GRAY-SPECIFIC COLOR PALETTE
// ========================

// Für Gray Backgrounds optimierte Farben (bessere Lesbarkeit)
#define GRAY_TXT_WHITE     ((THEME_BACKGROUND << 4) | COLOR_WHITE)
#define GRAY_TXT_BLACK     ((THEME_BACKGROUND << 4) | COLOR_BLACK)       // Für Kontrast
#define GRAY_TXT_GREEN     ((THEME_BACKGROUND << 4) | COLOR_GREEN)       // Etwas dunkler für besseren Kontrast
#define GRAY_TXT_BRIGHT_GREEN ((THEME_BACKGROUND << 4) | COLOR_LIGHT_GREEN)  // Heller
#define GRAY_TXT_RED       ((THEME_BACKGROUND << 4) | COLOR_RED)
#define GRAY_TXT_BRIGHT_RED ((THEME_BACKGROUND << 4) | COLOR_LIGHT_RED)
#define GRAY_TXT_BLUE      ((THEME_BACKGROUND << 4) | COLOR_BLUE)
#define GRAY_TXT_BRIGHT_BLUE ((THEME_BACKGROUND << 4) | COLOR_LIGHT_BLUE)
#define GRAY_TXT_CYAN      ((THEME_BACKGROUND << 4) | COLOR_CYAN)
#define GRAY_TXT_BRIGHT_CYAN ((THEME_BACKGROUND << 4) | COLOR_LIGHT_CYAN)
#define GRAY_TXT_YELLOW    ((THEME_BACKGROUND << 4) | COLOR_YELLOW)
#define GRAY_TXT_MAGENTA   ((THEME_BACKGROUND << 4) | COLOR_MAGENTA)
#define GRAY_TXT_BRIGHT_MAGENTA ((THEME_BACKGROUND << 4) | COLOR_LIGHT_MAGENTA)
#define GRAY_TXT_GRAY      ((THEME_BACKGROUND << 4) | COLOR_LIGHT_GRAY)   // Etwas heller für Kontrast

// ========================
// INTELLIGENTE FARBAUSWAHL
// ========================

// Automatische Auswahl basierend auf Background
// Bei dunklem Background helle Texte, bei hellem Background dunkle Texte

#if THEME_BACKGROUND == COLOR_BLACK || THEME_BACKGROUND == COLOR_DARK_GRAY
    // DUNKLES THEME - helle Texte
    #define TXT_NORMAL    ((THEME_BACKGROUND << 4) | COLOR_WHITE)
    #define TXT_SUCCESS   ((THEME_BACKGROUND << 4) | COLOR_LIGHT_GREEN)  // Heller für dunklen Hintergrund
    #define TXT_ERROR     ((THEME_BACKGROUND << 4) | COLOR_LIGHT_RED)    // Heller für dunklen Hintergrund
    #define TXT_WARNING   ((THEME_BACKGROUND << 4) | COLOR_YELLOW)
    #define TXT_INFO      ((THEME_BACKGROUND << 4) | COLOR_LIGHT_CYAN)   // Heller für dunklen Hintergrund
    #define TXT_BLUE      ((THEME_BACKGROUND << 4) | COLOR_LIGHT_BLUE)   // Heller für dunklen Hintergrund
    #define TXT_MAGENTA   ((THEME_BACKGROUND << 4) | COLOR_LIGHT_MAGENTA)// Heller für dunklen Hintergrund
    #define TXT_GRAY      ((THEME_BACKGROUND << 4) | COLOR_LIGHT_GRAY)
#elif THEME_BACKGROUND == COLOR_LIGHT_GRAY
    // HELLES THEME - dunkle Texte (besserer Kontrast)
    #define TXT_NORMAL    ((THEME_BACKGROUND << 4) | COLOR_BLACK)        // Schwarz auf Hellgrau
    #define TXT_SUCCESS   ((THEME_BACKGROUND << 4) | COLOR_GREEN)        // Dunkleres Grün
    #define TXT_ERROR     ((THEME_BACKGROUND << 4) | COLOR_RED)          // Dunkleres Rot
    #define TXT_WARNING   ((THEME_BACKGROUND << 4) | COLOR_BROWN)        // Braun statt Gelb (besser lesbar)
    #define TXT_INFO      ((THEME_BACKGROUND << 4) | COLOR_BLUE)         // Dunkleres Blau
    #define TXT_BLUE      ((THEME_BACKGROUND << 4) | COLOR_BLUE)
    #define TXT_MAGENTA   ((THEME_BACKGROUND << 4) | COLOR_MAGENTA)
    #define TXT_GRAY      ((THEME_BACKGROUND << 4) | COLOR_DARK_GRAY)    // Dunkleres Grau
#else
    // STANDARD (für Blue, Green, etc.)
    #define TXT_NORMAL    ((THEME_BACKGROUND << 4) | COLOR_WHITE)
    #define TXT_SUCCESS   ((THEME_BACKGROUND << 4) | COLOR_GREEN)
    #define TXT_ERROR     ((THEME_BACKGROUND << 4) | COLOR_RED)
    #define TXT_WARNING   ((THEME_BACKGROUND << 4) | COLOR_YELLOW)
    #define TXT_INFO      ((THEME_BACKGROUND << 4) | COLOR_LIGHT_CYAN)
    #define TXT_BLUE      ((THEME_BACKGROUND << 4) | COLOR_BLUE)
    #define TXT_MAGENTA   ((THEME_BACKGROUND << 4) | COLOR_MAGENTA)
    #define TXT_GRAY      ((THEME_BACKGROUND << 4) | COLOR_LIGHT_GRAY)
#endif

// ========================
// AUTO-COMPATIBILITY MACROS
// ========================

// ★★★ AUTO-KONVERTIERUNG ALLER ALTEN FARBEN ★★★
// Dein existierender Code funktioniert ohne Änderungen!

#define COLOR_WHITE_ON_BLUE     TXT_NORMAL
#define COLOR_GREEN_ON_BLUE     TXT_SUCCESS
#define COLOR_RED_ON_BLUE       TXT_ERROR
#define COLOR_YELLOW_ON_BLUE    TXT_WARNING
#define COLOR_CYAN_ON_BLUE      TXT_INFO
#define COLOR_LIGHT_CYAN_ON_BLUE TXT_INFO
#define COLOR_BLUE_ON_BLUE      TXT_BLUE
#define COLOR_MAGENTA_ON_BLUE   TXT_MAGENTA
#define COLOR_LIGHT_GRAY_ON_BLUE TXT_GRAY
#define COLOR_DARK_GRAY_ON_BLUE  TXT_GRAY
#define COLOR_BLACK_ON_BLUE     ((THEME_BACKGROUND << 4) | COLOR_BLACK)
#define COLOR_BROWN_ON_BLUE     ((THEME_BACKGROUND << 4) | COLOR_BROWN)

// Für andere Backgrounds (falls du später wechselst)
#define COLOR_WHITE_ON_BLACK    TXT_NORMAL
#define COLOR_WHITE_ON_DARK_GRAY TXT_NORMAL
#define COLOR_WHITE_ON_LIGHT_GRAY TXT_NORMAL
#define COLOR_BLACK_ON_LIGHT_GRAY TXT_NORMAL  // Für helles Theme

// ========================
// 3. STRUCTURE DEFINITIONS
// ========================

// GDT Entry Structure
struct gdt_entry {
    unsigned short limit_low;
    unsigned short base_low;
    unsigned char base_middle;
    unsigned char access;
    unsigned char granularity;
    unsigned char base_high;
} __attribute__((packed));

// GDT Pointer Structure
struct gdt_ptr {
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

// IDT Entry Structure
struct idt_entry {
    unsigned short base_low;
    unsigned short selector;
    unsigned char zero;
    unsigned char flags;
    unsigned short base_high;
} __attribute__((packed));

// IDT Pointer Structure
struct idt_ptr {
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

// Register state for interrupt handlers
struct regs {
    unsigned int gs, fs, es, ds;
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    unsigned int int_no, err_code;
    unsigned int eip, cs, eflags, useresp, ss;
};

// Neue Structs für Memory Management
struct multiboot_info {
    unsigned int flags;
    unsigned int mem_lower;
    unsigned int mem_upper;
    unsigned int boot_device;
    unsigned int cmdline;
    unsigned int mods_count;
    unsigned int mods_addr;
    unsigned int syms[4];
    unsigned int mmap_length;
    unsigned int mmap_addr;
    unsigned int drives_length;
    unsigned int drives_addr;
    unsigned int config_table;
    unsigned int boot_loader_name;
    unsigned int apm_table;
} __attribute__((packed));

// Memory Map Entry
struct mmap_entry {
    unsigned int size;
    unsigned long long base_addr;
    unsigned long long length;
    unsigned int type;
} __attribute__((packed));

// Einfache Heap-Allocation-Tracking
struct alloc_info {
    void* ptr;
    unsigned int size;
    int used;
};

// KFS Strukturen
struct kfs_superblock {
    uint32_t magic;           // 0x4B46531A
    uint32_t total_blocks;    // Gesamtblöcke im System
    uint32_t free_blocks;     // Freie Blöcke
    uint32_t inode_count;     // Anzahl INodes
    uint32_t block_size;      // Sollte 512 sein
    char volume_name[32];     // Volume Name
};

struct kfs_inode {
    uint32_t id;              // INode Nummer
    char name[MAX_NAME_LEN];  // Dateiname
    uint32_t size;            // Dateigröße in Bytes
    uint32_t blocks[16];      // Block-Pointer (direkt)
    uint8_t type;             // 1=Datei, 2=Verzeichnis
    uint32_t parent;          // Eltern-INode
    uint32_t created;         // Erstellungszeit (Timestamp)
    uint32_t modified;        // Änderungszeit
};

struct kfs_dir_entry {
    uint32_t inode_id;        // Verweis auf INode
    char name[MAX_NAME_LEN];  // Eintragsname (für Verzeichnisse)
};

int debug_mode = 0;

// ========================
// 4. GLOBAL VARIABLES
// ========================
int cursor_x = 0;
int cursor_y = 0;

// GDT Variables
struct gdt_entry gdt[GDT_ENTRIES];
struct gdt_ptr gp;

// IDT Variables
struct idt_entry idt[IDT_ENTRIES];
struct idt_ptr idtp;

// Shell Variables
char cmd_buffer[CMD_BUFFER_SIZE];
int cmd_pos = 0;
char cmd_history[CMD_HISTORY_SIZE][CMD_BUFFER_SIZE];
int history_count = 0;
int history_index = -1;

// Memory Management Variables (SIMPLE VERSION)
unsigned int total_memory = 0;
unsigned int free_memory = 0;
unsigned int used_memory = 0;
unsigned int kernel_memory = 0;

unsigned char page_bitmap[BITMAP_SIZE];  // 1 bit pro 4KB page
unsigned int total_pages = 0;
unsigned int free_pages = 0;
unsigned int used_pages = 0;

// Simple Heap Management
static unsigned int heap_pointer = HEAP_START;
static unsigned int heap_end = HEAP_START + HEAP_SIZE;
struct alloc_info allocations[MAX_ALLOCS];
int alloc_count = 0;

static struct kfs_superblock* superblock = (struct kfs_superblock*)ramdisk;

// INode Tabelle (nach Superblock)
static struct kfs_inode* inode_table = (struct kfs_inode*)(ramdisk + BLOCK_SIZE);

// Block Bitmap (nach INode Tabelle)
static uint8_t* block_bitmap = (uint8_t*)(ramdisk + BLOCK_SIZE + (MAX_FILES * sizeof(struct kfs_inode)));

// Datenblöcke (danach)
static uint8_t* data_blocks;
// Aktuelles Verzeichnis
static uint32_t current_dir_inode = 1;  // Start bei Root (INode 1)



// ========================
// 5. FUNCTION DECLARATIONS
// ========================

// GDT Functions
void gdt_set_gate(int num, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran);
void gdt_install(void);

// IDT Functions
void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags);
void isr_install(void);
void irq_install(void);

// PIC Functions
void pic_remap(int offset1, int offset2);
void pic_send_eoi(unsigned char irq);
void pic_mask_irq(unsigned char irq_line);
void pic_unmask_irq(unsigned char irq_line);

// Interrupt Handlers
void isr_handler(struct regs *r);
void irq_handler(struct regs *r);

// Display Functions
void print_char_color(char c, int x, int y, unsigned char color);
void print_char(char c, int x, int y);
void kprint(const char* str, unsigned char color);
void kprint_no_scroll(const char* str, unsigned char color);
void kprint_at(const char* str, int x, int y, unsigned char color);
void print_right(const char* str, int y, unsigned char color);
void scroll_screen(void);

// Cursor Functions
void set_cursor(int x, int y);
void get_cursor(int* x, int* y);
void clear_screen(unsigned char bg_color);

// Helper Functions
static inline unsigned char inb(unsigned short port);
static inline void outb(unsigned short port, unsigned char val);

// Shell Functions
void execute_command(char *cmd);
int strcmp(const char* s1, const char* s2);
int strlen(const char* s);
void strcpy(char* dest, const char* src);
int strstart(const char* str, const char* prefix);

void history_add(const char* cmd);
void show_history_command(void);

// Memory Management Functions (SAFE VERSION)
void read_multiboot_info(unsigned int addr);
void init_simple_memory(void);
void init_simple_heap(void);
void* kmalloc_safe(unsigned int size);
void kfree_safe(void* ptr);
void print_memory_info(void);
void debug_memory(void);

// KFS functions
void kfs_init(void);
void kfs_format(const char* volume_name);
int kfs_create(const char* name, uint8_t type);
int kfs_write(int inode_idx, const void* data, uint32_t size);
int kfs_read(int inode_idx, void* buffer, uint32_t size);
int kfs_delete(const char* name);

// ========================
// 6. MEMORY MANAGEMENT (SAFE VERSION)
// ========================

void read_multiboot_info(unsigned int addr) {
    struct multiboot_info* mbi = (struct multiboot_info*)addr;

    if (mbi->flags & 0x01) {  // Memory info available
        total_memory = (mbi->mem_upper + 1024) * 1024;  // Convert to bytes
    } else {
        // Fallback: 16MB
        total_memory = 16 * 1024 * 1024;
    }

    // Kernel nutzt etwa 1MB
    kernel_memory = 0x100000;
    used_memory = kernel_memory;
    free_memory = total_memory - used_memory;

    // Page calculations
    total_pages = total_memory / PAGE_SIZE;
    used_pages = used_memory / PAGE_SIZE;
    free_pages = total_pages - used_pages;
}

void init_simple_memory(void) {
    // Bitmap initialisieren (alles frei)
    for(int i = 0; i < BITMAP_SIZE; i++) {
        page_bitmap[i] = 0xFF;
    }

    // Kernel-Bereich als belegt markieren (0x100000 - 0x200000)
    unsigned int kernel_start = 0x100000;
    unsigned int kernel_end = 0x200000;

    for(unsigned int addr = kernel_start; addr < kernel_end; addr += PAGE_SIZE) {
        int page_idx = addr / PAGE_SIZE;
        int byte_idx = page_idx / 8;
        int bit_idx = page_idx % 8;

        // Sicherheitscheck
        if(byte_idx < BITMAP_SIZE) {
            page_bitmap[byte_idx] &= ~(1 << bit_idx);
            used_pages++;
            free_pages--;
        }
    }

    // VGA-Bereich als belegt markieren (0xB8000)
    unsigned int vga_start = 0xB8000;
    unsigned int vga_end = 0xB8000 + (SCREEN_WIDTH * SCREEN_HEIGHT * 2);

    for(unsigned int addr = vga_start; addr < vga_end; addr += PAGE_SIZE) {
        int page_idx = addr / PAGE_SIZE;
        int byte_idx = page_idx / 8;
        int bit_idx = page_idx % 8;

        if(byte_idx < BITMAP_SIZE) {
            page_bitmap[byte_idx] &= ~(1 << bit_idx);
            used_pages++;
            free_pages--;
        }
    }

    used_memory = used_pages * PAGE_SIZE;
    free_memory = free_pages * PAGE_SIZE;
}

void init_simple_heap(void) {
    // Heap-Bereich initialisieren
    heap_pointer = HEAP_START;
    heap_end = HEAP_START + HEAP_SIZE;

    // Allocation-Tracking initialisieren
    for(int i = 0; i < MAX_ALLOCS; i++) {
        allocations[i].ptr = NULL;
        allocations[i].size = 0;
        allocations[i].used = 0;
    }
    alloc_count = 0;

    kprint("Heap initialized at 0x", COLOR_WHITE_ON_BLUE);

    // Hex-Ausgabe für Heap-Start
    char hex[9];
    unsigned int addr = HEAP_START;
    for(int i = 7; i >= 0; i--) {
        int nibble = (addr >> (i * 4)) & 0xF;
        hex[7-i] = nibble < 10 ? '0' + nibble : 'A' + (nibble - 10);
    }
    hex[8] = '\0';
    kprint(hex, COLOR_GREEN_ON_BLUE);
    kprint("\n", COLOR_WHITE_ON_BLUE);
}

void* kmalloc_safe(unsigned int size) {
    // Auf Blockgröße alignen
    size = (size + BLOCK_SIZE - 1) & ~(BLOCK_SIZE - 1);

    // Prüfen ob genug Platz
    if(heap_pointer + size > heap_end) {
        return NULL;
    }

    // Freien Slot finden
    int slot = -1;
    for(int i = 0; i < MAX_ALLOCS; i++) {
        if(!allocations[i].used) {
            slot = i;
            break;
        }
    }

    if(slot == -1) {
        return NULL;  // Keine freien Slots
    }

    // Speicher zuweisen
    void* ptr = (void*)heap_pointer;
    allocations[slot].ptr = ptr;
    allocations[slot].size = size;
    allocations[slot].used = 1;

    heap_pointer += size;
    alloc_count++;
    used_memory += size;
    free_memory -= size;

    return ptr;
}

void kfree_safe(void* ptr) {
    if(!ptr) return;

    // Slot finden und freigeben
    for(int i = 0; i < MAX_ALLOCS; i++) {
        if(allocations[i].used && allocations[i].ptr == ptr) {
            allocations[i].used = 0;
            alloc_count--;

            used_memory -= allocations[i].size;
            free_memory += allocations[i].size;

            // NOTE: Einfache Version - kein tatsächliches Zurückgeben an Heap
            // Für einfache Demos reicht das
            return;
        }
    }
}

void print_memory_info(void) {
    kprint("\n=== MEMORY INFORMATION ===\n", COLOR_WHITE_ON_DARK_GRAY);

    // Total memory in MB
    unsigned int total_mb = total_memory / (1024 * 1024);
    unsigned int free_mb = free_memory / (1024 * 1024);
    unsigned int used_mb = used_memory / (1024 * 1024);
    unsigned int kernel_mb = kernel_memory / (1024 * 1024);

    kprint("Total RAM:     ", COLOR_WHITE_ON_DARK_GRAY);

    // Total MB ausgeben
    char total_str[16];
    char* total_ptr = total_str;
    unsigned int n = total_mb;

    if(n == 0) {
        *total_ptr++ = '0';
    } else {
        char temp[16];
        int i = 0;
        while(n > 0) {
            temp[i++] = '0' + (n % 10);
            n /= 10;
        }
        while(i > 0) {
            *total_ptr++ = temp[--i];
        }
    }
    *total_ptr++ = 'M';
    *total_ptr++ = 'B';
    *total_ptr = '\0';

    kprint(total_str, COLOR_GREEN);
    kprint("\n", COLOR_WHITE_ON_DARK_GRAY);

    kprint("Kernel usage:  ", COLOR_WHITE_ON_DARK_GRAY);
    char kernel_str[16];
    total_ptr = kernel_str;
    n = kernel_mb;

    if(n == 0) {
        *total_ptr++ = '0';
    } else {
        char temp[16];
        int i = 0;
        while(n > 0) {
            temp[i++] = '0' + (n % 10);
            n /= 10;
        }
        while(i > 0) {
            *total_ptr++ = temp[--i];
        }
    }
    *total_ptr++ = 'M';
    *total_ptr++ = 'B';
    *total_ptr = '\0';

    kprint(kernel_str, COLOR_YELLOW);
    kprint("\n", COLOR_WHITE_ON_DARK_GRAY);

    kprint("Free memory:   ", COLOR_WHITE_ON_DARK_GRAY);
    char free_str[16];
    total_ptr = free_str;
    n = free_mb;

    if(n == 0) {
        *total_ptr++ = '0';
    } else {
        char temp[16];
        int i = 0;
        while(n > 0) {
            temp[i++] = '0' + (n % 10);
            n /= 10;
        }
        while(i > 0) {
            *total_ptr++ = temp[--i];
        }
    }
    *total_ptr++ = 'M';
    *total_ptr++ = 'B';
    *total_ptr = '\0';

    kprint(free_str, COLOR_GREEN);
    kprint("\n", COLOR_WHITE_ON_DARK_GRAY);

    kprint("Pages:         ", COLOR_WHITE_ON_DARK_GRAY);
    char page_str[32];
    char * page_ptr = page_str;

    // Total pages
    n = total_pages;
    if(n == 0) {
        *page_ptr++ = '0';
    } else {
        char temp[16];
        int i = 0;
        while(n > 0) {
            temp[i++] = '0' + (n % 10);
            n /= 10;
        }
        while(i > 0) {
            *page_ptr++ = temp[--i];
        }
    }
    *page_ptr++ = ' ';
    *page_ptr++ = 't';
    *page_ptr++ = 'o';
    *page_ptr++ = 't';
    *page_ptr++ = 'a';
    *page_ptr++ = 'l';
    *page_ptr++ = ',';
    *page_ptr++ = ' ';

    // Free pages
    n = free_pages;
    if(n == 0) {
        *page_ptr++ = '0';
    } else {
        char temp[16];
        int i = 0;
        while(n > 0) {
            temp[i++] = '0' + (n % 10);
            n /= 10;
        }
        while(i > 0) {
            *page_ptr++ = temp[--i];
        }
    }
    *page_ptr++ = ' ';
    *page_ptr++ = 'f';
    *page_ptr++ = 'r';
    *page_ptr++ = 'e';
    *page_ptr++ = 'e';
    *page_ptr = '\0';

    kprint(page_str, COLOR_CYAN_ON_BLUE);
    kprint("\n", COLOR_WHITE_ON_BLUE);

    kprint("Allocations:   ", COLOR_WHITE_ON_BLUE);
    char alloc_str[16];
    char * alloc_ptr = alloc_str;
    n = alloc_count;

    if(n == 0) {
        *alloc_ptr++ = '0';
    } else {
        char temp[16];
        int i = 0;
        while(n > 0) {
            temp[i++] = '0' + (n % 10);
            n /= 10;
        }
        while(i > 0) {
            *alloc_ptr++ = temp[--i];
        }
    }
    *alloc_ptr++ = ' ';
    *alloc_ptr++ = 'a';
    *alloc_ptr++ = 'c';
    *alloc_ptr++ = 't';
    *alloc_ptr++ = 'i';
    *alloc_ptr++ = 'v';
    *alloc_ptr++ = 'e';
    *alloc_ptr = '\0';

    kprint(alloc_str, alloc_count > 0 ? COLOR_YELLOW_ON_BLUE : COLOR_GREEN_ON_BLUE);
    kprint("\n", COLOR_WHITE_ON_BLUE);
}

void debug_memory(void) {
    kprint("\n=== MEMORY DEBUG ===\n", COLOR_RED_ON_BLUE);

    kprint("Heap pointer: 0x", COLOR_WHITE_ON_BLUE);
    char hex[9];
    unsigned int addr = heap_pointer;
    for(int i = 7; i >= 0; i--) {
        int nibble = (addr >> (i * 4)) & 0xF;
        hex[7-i] = nibble < 10 ? '0' + nibble : 'A' + (nibble - 10);
    }
    hex[8] = '\0';
    kprint(hex, COLOR_YELLOW_ON_BLUE);
    kprint("\n", COLOR_WHITE_ON_BLUE);

    kprint("Active allocations:\n", COLOR_WHITE_ON_BLUE);
    for(int i = 0; i < MAX_ALLOCS; i++) {
        if(allocations[i].used) {
            kprint("  Slot ", COLOR_WHITE_ON_BLUE);
            char slot_str[4];
            slot_str[0] = '0' + i;
            slot_str[1] = ':';
            slot_str[2] = ' ';
            slot_str[3] = '\0';
            kprint(slot_str, COLOR_WHITE_ON_BLUE);

            kprint("Ptr=0x", COLOR_WHITE_ON_BLUE);
            addr = (unsigned int)allocations[i].ptr;
            for(int j = 7; j >= 0; j--) {
                int nibble = (addr >> (j * 4)) & 0xF;
                hex[7-j] = nibble < 10 ? '0' + nibble : 'A' + (nibble - 10);
            }
            kprint(hex, COLOR_YELLOW_ON_BLUE);

            kprint(" Size=", COLOR_WHITE_ON_BLUE);
            char size_str[16];
            char * size_ptr = size_str;
            unsigned int size_n = allocations[i].size;

            if(size_n == 0) {
                *size_ptr++ = '0';
            } else {
                char temp[16];
                int j = 0;
                while(size_n > 0) {
                    temp[j++] = '0' + (size_n % 10);
                    size_n /= 10;
                }
                while(j > 0) {
                    *size_ptr++ = temp[--j];
                }
            }
            *size_ptr++ = 'B';
            *size_ptr = '\0';

            kprint(size_str, COLOR_GREEN_ON_BLUE);
            kprint("\n", COLOR_WHITE_ON_BLUE);
        }
    }
}

// ========================
// PORT I/O FUNCTIONS
// ========================

static inline unsigned char inb(unsigned short port) {
    unsigned char ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(unsigned short port, unsigned char val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

// ========================
// STRING FUNCTIONS
// ========================

int strcmp(const char* s1, const char* s2) {
    while(*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

int strlen(const char* s) {
    int len = 0;
    while(s[len]) len++;
    return len;
}

void strcpy(char* dest, const char* src) {
    while(*src) {
        *dest++ = *src++;
    }
    *dest = '\0';
}

int strstart(const char* str, const char* prefix) {
    while(*prefix) {
        if(*prefix++ != *str++) return 0;
    }
    return 1;
}

// ========================
// PIC FUNCTIONS
// ========================

void pic_remap(int offset1, int offset2) {
    unsigned char a1, a2;

    // Save current masks
    a1 = inb(0x21);
    a2 = inb(0xA1);

    // Start initialization sequence
    outb(0x20, 0x11);  // ICW1
    outb(0xA0, 0x11);

    // ICW2: vector offsets
    outb(0x21, offset1);  // Master PIC
    outb(0xA1, offset2);  // Slave PIC

    // ICW3: cascade
    outb(0x21, 0x04);     // Master: slave at IRQ2
    outb(0xA1, 0x02);     // Slave: cascade identity

    // ICW4: 8086 mode
    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    // Restore saved masks
    outb(0x21, a1);
    outb(0xA1, a2);
}

void pic_send_eoi(unsigned char irq) {
    if(irq >= 8) {
        outb(0xA0, 0x20);  // Slave PIC
    }
    outb(0x20, 0x20);      // Master PIC
}

// Rest of PIC functions bleiben gleich...

// ========================
// GDT FUNCTIONS
// ========================

void gdt_set_gate(int num, unsigned long base, unsigned long limit,
                  unsigned char access, unsigned char gran) {
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;

    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F);

    gdt[num].granularity |= (gran & 0xF0);
    gdt[num].access = access;
}

void gdt_install(void) {
    gp.limit = (sizeof(struct gdt_entry) * GDT_ENTRIES) - 1;
    gp.base = (unsigned int)&gdt;

    gdt_set_gate(0, 0, 0, 0, 0);
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    asm volatile("lgdt (%0)" : : "r" (&gp));

    asm volatile(
        "movw $0x10, %%ax\n"
        "movw %%ax, %%ds\n"
        "movw %%ax, %%es\n"
        "movw %%ax, %%fs\n"
        "movw %%ax, %%gs\n"
        "movw %%ax, %%ss\n"
        "ljmp $0x08, $1f\n"
        "1:\n"
        : : : "ax"
    );
}

// ========================
// IDT FUNCTIONS
// ========================

void idt_set_gate(unsigned char num, unsigned long base,
                  unsigned short sel, unsigned char flags) {
    idt[num].base_low = (base & 0xFFFF);
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].selector = sel;
    idt[num].zero = 0;
    idt[num].flags = flags;
}

// ========================
// INTERRUPT HANDLERS (gekürzt, bleibt gleich)
// ========================

void isr_handler(struct regs *r) {
    if (r->int_no >= 32) {
        if (r->int_no >= 32 && r->int_no <= 47) {
            pic_send_eoi(r->int_no - 32);
        }
        return;
    }

    kprint("\n[CPU EXCEPTION] INT 0x", COLOR_RED_ON_BLUE);

    char hex[5];
    hex[0] = '0';
    hex[1] = 'x';
    hex[2] = "0123456789ABCDEF"[(r->int_no >> 4) & 0xF];
    hex[3] = "0123456789ABCDEF"[r->int_no & 0xF];
    hex[4] = '\0';
    kprint(hex, COLOR_WHITE_ON_BLUE);

    if (r->int_no == 0) kprint(" (Division by zero)\n", COLOR_RED_ON_BLUE);
    else if (r->int_no == 8) kprint(" (Double Fault)\n", COLOR_RED_ON_BLUE);
    else if (r->int_no == 13) kprint(" (General Protection)\n", COLOR_RED_ON_BLUE);
    else if (r->int_no == 14) kprint(" (Page Fault)\n", COLOR_RED_ON_BLUE);
    else kprint(" (CPU Exception)\n", COLOR_RED_ON_BLUE);

    if (r->int_no == 8 || r->int_no == 13) {
        kprint("SYSTEM HALTED\n", COLOR_RED_ON_BLUE);
        for(;;) asm volatile("cli; hlt");
    }
}

void irq_handler(struct regs *r) {
    unsigned char irq_num = r->int_no - 32;

    // Timer handler (IRQ 0)
    if (irq_num == 0) {
        static uint32_t timer_ticks = 0;
        timer_ticks++;

        // Update uptime every second (PIT configured for ~18.2 Hz)
        if (timer_ticks % 18 == 0) {
            // Save cursor
            int old_x = cursor_x;
            int old_y = cursor_y;

            // Show uptime at top-right
            set_cursor(68, 0);

            uint32_t seconds = timer_ticks / 18;

            // Convert seconds to string
            char sec_str[10];
            int i = 0;
            uint32_t n = seconds;

            if (n == 0) {
                sec_str[i++] = '0';
            } else {
                char temp[10];
                int j = 0;
                while (n > 0) {
                    temp[j++] = '0' + (n % 10);
                    n /= 10;
                }
                while (j > 0) {
                    sec_str[i++] = temp[--j];
                }
            }
            sec_str[i] = '\0';

            kprint("Uptime: ", COLOR_CYAN_ON_BLUE);
            kprint(sec_str, COLOR_WHITE_ON_BLUE);
            kprint("s", COLOR_CYAN_ON_BLUE);
            kprint("   ", COLOR_CYAN_ON_BLUE); // Clear old

            // Restore cursor
            set_cursor(old_x, old_y);
        }

        // EOI for timer
        pic_send_eoi(irq_num);
    }

    // Keyboard handler (IRQ 1) - WITH ARROW KEY SUPPORT
    else if (irq_num == 1) {
        unsigned char scancode = inb(0x60);

        // ====== NUR WENN DEBUG MODE AN IST ======
        if (debug_mode) {
            kprint("[", COLOR_YELLOW_ON_BLUE);
            if (scancode & 0x80) {
                kprint("BRK:", COLOR_RED_ON_BLUE);
            } else {
                kprint("MAK:", COLOR_GREEN_ON_BLUE);
            }
            char hex[3];
            hex[0] = "0123456789ABCDEF"[(scancode >> 4) & 0xF];
            hex[1] = "0123456789ABCDEF"[scancode & 0xF];
            hex[2] = '\0';
            kprint(hex, COLOR_WHITE_ON_BLUE);
            kprint("]", COLOR_YELLOW_ON_BLUE);
        }
        // ====== ENDE DEBUG MODE ======

        // ONLY handle Make codes (key press, not release)
        if (!(scancode & 0x80)) {
            char key = 0;

            // DEUTSCHE TASTATUR
            switch(scancode) {
                // Buchstaben - DE Layout
                case 0x10: key = 'q'; break;
                case 0x11: key = 'w'; break;
                case 0x12: key = 'e'; break;
                case 0x13: key = 'r'; break;
                case 0x14: key = 't'; break;
                case 0x15: key = 'z'; break;
                case 0x16: key = 'u'; break;
                case 0x17: key = 'i'; break;
                case 0x18: key = 'o'; break;
                case 0x19: key = 'p'; break;
                case 0x1E: key = 'a'; break;
                case 0x1F: key = 's'; break;
                case 0x20: key = 'd'; break;
                case 0x21: key = 'f'; break;
                case 0x22: key = 'g'; break;
                case 0x23: key = 'h'; break;
                case 0x24: key = 'j'; break;
                case 0x25: key = 'k'; break;
                case 0x26: key = 'l'; break;
                case 0x2C: key = 'y'; break;
                case 0x2D: key = 'x'; break;
                case 0x2E: key = 'c'; break;
                case 0x2F: key = 'v'; break;
                case 0x30: key = 'b'; break;
                case 0x31: key = 'n'; break;
                case 0x32: key = 'm'; break;

                // Zahlen
                case 0x02: key = '1'; break;
                case 0x03: key = '2'; break;
                case 0x04: key = '3'; break;
                case 0x05: key = '4'; break;
                case 0x06: key = '5'; break;
                case 0x07: key = '6'; break;
                case 0x08: key = '7'; break;
                case 0x09: key = '8'; break;
                case 0x0A: key = '9'; break;
                case 0x0B: key = '0'; break;

                // ., ,,
                case 0x34:  key = '.'; break;

                case 0x33:  key = ','; break;

                case 0x35:  key = '#'; break;

                case 0x2B:  key = '+'; break;

                case 0x0C:  key = '-'; break;

                // Sondertasten
                case 0x1C: key = '\n'; break;  // Enter
                case 0x0E: key = '\b'; break;  // Backspace
                case 0x39: key = ' ';  break;  // Space
                case 0x0F: key = '\t'; break;  // Tab

                // Shift/Ctrl/Alt ignorieren
                case 0x2A: case 0x36: // Shift
                case 0x1D: case 0x38: // Ctrl/Alt
                    key = 0;
                    break;

                // ====== ARROW KEYS ======
                case 0x48: // Up arrow - previous command
                    if (debug_mode) kprint("[UP]", COLOR_CYAN_ON_BLUE);
                    if (history_count > 0) {
                        if (history_index > 0) {
                            history_index--;
                        } else {
                            history_index = 0;
                        }
                        show_history_command();
                    }
                    key = 0;
                    break;

                case 0x50: // Down arrow - next command
                    if (debug_mode) kprint("[DOWN]", COLOR_CYAN_ON_BLUE);
                    if (history_count > 0) {
                        if (history_index < history_count - 1) {
                            history_index++;
                            show_history_command();
                        } else {
                            // Back to empty line
                            history_index = history_count;
                            // Clear line
                            set_cursor(6, cursor_y);
                            for (int i = 0; i < cmd_pos; i++) {
                                kprint(" ", COLOR_WHITE_ON_BLUE);
                            }
                            set_cursor(6, cursor_y);
                            cmd_buffer[0] = '\0';
                            cmd_pos = 0;
                        }
                    }
                    key = 0;
                    break;

                case 0x4B: // Left arrow
                    if (debug_mode) kprint("[LEFT]", COLOR_CYAN_ON_BLUE);
                    if (cursor_x > 6) {
                        cursor_x--;
                        set_cursor(cursor_x, cursor_y);
                    }
                    key = 0;
                    break;

                case 0x4D: // Right arrow
                    if (debug_mode) kprint("[RIGHT]", COLOR_CYAN_ON_BLUE);
                    if (cursor_x < 6 + cmd_pos) {
                        cursor_x++;
                        set_cursor(cursor_x, cursor_y);
                    }
                    key = 0;
                    break;
                // ====== END ARROW KEYS ======

                default:
                    if (debug_mode) {
                        kprint("[?]", COLOR_CYAN_ON_BLUE);
                    }
                    key = 0;
                    break;
            }

            if (key != 0) {
                // ENTER: Execute command
                if (key == '\n') {
                    cmd_buffer[cmd_pos] = '\0';

                    // Save to history BEFORE execution
                    if (cmd_pos > 0) {
                        history_add(cmd_buffer);
                    }

                    // Execute command
                    execute_command(cmd_buffer);
                    cmd_pos = 0;
                    history_index = history_count;  // Reset to "new command"

                    // Show new prompt
                    kprint("\nkons> ", COLOR_WHITE_ON_BLUE);
                    set_cursor(6, cursor_y);
                }
                // BACKSPACE: Delete character
                else if (key == '\b') {
                    if (cmd_pos > 0 && cursor_x > 6) {
                        cmd_pos--;

                        // Shift buffer left
                        for (int i = cursor_x - 6; i < cmd_pos; i++) {
                            cmd_buffer[i] = cmd_buffer[i + 1];
                        }
                        cmd_buffer[cmd_pos] = '\0';

                        // Delete from screen and shift left
                        cursor_x--;
                        for (int i = cursor_x - 6; i <= cmd_pos; i++) {
                            char c = (i < cmd_pos) ? cmd_buffer[i] : ' ';
                            print_char_color(c, 6 + i, cursor_y, COLOR_WHITE_ON_BLUE);
                        }
                        set_cursor(cursor_x, cursor_y);
                    }
                }
                // TAB: 4 spaces
                else if (key == '\t') {
                    kprint("    ", COLOR_WHITE_ON_BLUE);
                    for(int i = 0; i < 4; i++) {
                        if (cmd_pos < CMD_BUFFER_SIZE - 1) {
                            cmd_buffer[cmd_pos++] = ' ';
                        }
                    }
                    set_cursor(cursor_x, cursor_y);
                }
                // NORMAL KEY: Add to buffer and echo
                else {
                    if (cmd_pos < CMD_BUFFER_SIZE - 1) {
                        // If inserting in middle of text, shift right
                        if (cursor_x < 6 + cmd_pos) {
                            int insert_pos = cursor_x - 6;
                            for (int i = cmd_pos; i > insert_pos; i--) {
                                cmd_buffer[i] = cmd_buffer[i-1];
                            }
                            cmd_buffer[insert_pos] = key;

                            // Redraw from insertion point
                            for (int i = insert_pos; i <= cmd_pos; i++) {
                                print_char_color(cmd_buffer[i], 6 + i, cursor_y, COLOR_WHITE_ON_BLUE);
                            }
                            cursor_x++;
                        } else {
                            // Append at end
                            cmd_buffer[cmd_pos++] = key;
                            char str[2] = {key, '\0'};
                            kprint(str, COLOR_WHITE_ON_BLUE);
                        }
                        set_cursor(cursor_x, cursor_y);
                    }
                }
            }
        }

        // EOI for keyboard
        pic_send_eoi(irq_num);
    }

    // Other IRQs (just acknowledge)
    else {
        pic_send_eoi(irq_num);
    }
}

// ========================
// ISR INSTALL
// ========================

void isr_install(void) {
    idtp.limit = (sizeof(struct idt_entry) * IDT_ENTRIES) - 1;
    idtp.base = (unsigned int)&idt;

    // Clear IDT
    for(int i = 0; i < IDT_ENTRIES; i++) {
        idt_set_gate(i, 0, 0, 0);
    }

    // CPU Exceptions (0-31)
    idt_set_gate(0, (unsigned long)_isr0, 0x08, 0x8E);
    idt_set_gate(1, (unsigned long)_isr1, 0x08, 0x8E);
    idt_set_gate(2, (unsigned long)_isr2, 0x08, 0x8E);
    idt_set_gate(3, (unsigned long)_isr3, 0x08, 0x8E);
    idt_set_gate(4, (unsigned long)_isr4, 0x08, 0x8E);
    idt_set_gate(5, (unsigned long)_isr5, 0x08, 0x8E);
    idt_set_gate(6, (unsigned long)_isr6, 0x08, 0x8E);
    idt_set_gate(7, (unsigned long)_isr7, 0x08, 0x8E);
    idt_set_gate(8, (unsigned long)_isr8, 0x08, 0x8E);
    idt_set_gate(9, (unsigned long)_isr9, 0x08, 0x8E);
    idt_set_gate(10, (unsigned long)_isr10, 0x08, 0x8E);
    idt_set_gate(11, (unsigned long)_isr11, 0x08, 0x8E);
    idt_set_gate(12, (unsigned long)_isr12, 0x08, 0x8E);
    idt_set_gate(13, (unsigned long)_isr13, 0x08, 0x8E);
    idt_set_gate(14, (unsigned long)_isr14, 0x08, 0x8E);
    idt_set_gate(15, (unsigned long)_isr15, 0x08, 0x8E);
    idt_set_gate(16, (unsigned long)_isr16, 0x08, 0x8E);
    idt_set_gate(17, (unsigned long)_isr17, 0x08, 0x8E);
    idt_set_gate(18, (unsigned long)_isr18, 0x08, 0x8E);
    idt_set_gate(19, (unsigned long)_isr19, 0x08, 0x8E);
    idt_set_gate(20, (unsigned long)_isr20, 0x08, 0x8E);
    idt_set_gate(21, (unsigned long)_isr21, 0x08, 0x8E);
    idt_set_gate(22, (unsigned long)_isr22, 0x08, 0x8E);
    idt_set_gate(23, (unsigned long)_isr23, 0x08, 0x8E);
    idt_set_gate(24, (unsigned long)_isr24, 0x08, 0x8E);
    idt_set_gate(25, (unsigned long)_isr25, 0x08, 0x8E);
    idt_set_gate(26, (unsigned long)_isr26, 0x08, 0x8E);
    idt_set_gate(27, (unsigned long)_isr27, 0x08, 0x8E);
    idt_set_gate(28, (unsigned long)_isr28, 0x08, 0x8E);
    idt_set_gate(29, (unsigned long)_isr29, 0x08, 0x8E);
    idt_set_gate(30, (unsigned long)_isr30, 0x08, 0x8E);
    idt_set_gate(31, (unsigned long)_isr31, 0x08, 0x8E);

    // Hardware IRQs (32-47)
    idt_set_gate(32, (unsigned long)_irq0, 0x08, 0x8E);
    idt_set_gate(33, (unsigned long)_irq1, 0x08, 0x8E);
    idt_set_gate(34, (unsigned long)_irq2, 0x08, 0x8E);
    idt_set_gate(35, (unsigned long)_irq3, 0x08, 0x8E);
    idt_set_gate(36, (unsigned long)_irq4, 0x08, 0x8E);
    idt_set_gate(37, (unsigned long)_irq5, 0x08, 0x8E);
    idt_set_gate(38, (unsigned long)_irq6, 0x08, 0x8E);
    idt_set_gate(39, (unsigned long)_irq7, 0x08, 0x8E);
    idt_set_gate(40, (unsigned long)_irq8, 0x08, 0x8E);
    idt_set_gate(41, (unsigned long)_irq9, 0x08, 0x8E);
    idt_set_gate(42, (unsigned long)_irq10, 0x08, 0x8E);
    idt_set_gate(43, (unsigned long)_irq11, 0x08, 0x8E);
    idt_set_gate(44, (unsigned long)_irq12, 0x08, 0x8E);
    idt_set_gate(45, (unsigned long)_irq13, 0x08, 0x8E);
    idt_set_gate(46, (unsigned long)_irq14, 0x08, 0x8E);
    idt_set_gate(47, (unsigned long)_irq15, 0x08, 0x8E);

    // Load IDT
    asm volatile("lidt (%0)" : : "r" (&idtp));
}

void irq_install(void) {
    // Mask ALL interrupts first
    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);

    // Unmask only Timer (IRQ0) and Keyboard (IRQ1)
    outb(0x21, 0xFC);  // 11111100 = IRQ0 & IRQ1 enabled

    // Keep slave PIC completely masked
    outb(0xA1, 0xFF);
}

// =========
// RAM-Disk
// =========

void kfs_format(const char* volume_name) {
    kprint("\nFormatting KFS volume: ", COLOR_YELLOW_ON_BLUE);
    kprint(volume_name, COLOR_WHITE_ON_BLUE);
    kprint("\n", COLOR_YELLOW_ON_BLUE);

    // Superblock initialisieren
    superblock->magic = KFS_MAGIC;
    superblock->total_blocks = RAMDISK_SIZE / BLOCK_SIZE;
    superblock->free_blocks = superblock->total_blocks - 10;  // Reserve für Metadaten
    superblock->inode_count = MAX_FILES;
    superblock->block_size = BLOCK_SIZE;

    // Volume Name kopieren
    int i = 0;
    while(volume_name[i] && i < 31) {
        superblock->volume_name[i] = volume_name[i];
        i++;
    }
    superblock->volume_name[i] = '\0';

    // INode Tabelle leeren
    for(i = 0; i < MAX_FILES; i++) {
        inode_table[i].id = 0;
        inode_table[i].name[0] = '\0';
        inode_table[i].size = 0;
        inode_table[i].type = 0;
        inode_table[i].parent = 0;
        inode_table[i].created = 0;
        inode_table[i].modified = 0;
        for(int j = 0; j < 16; j++) {
            inode_table[i].blocks[j] = 0;
        }
    }

    // Bitmap initialisieren (alles frei)
    uint32_t bitmap_size = superblock->total_blocks / 8;
    for(i = 0; i < bitmap_size; i++) {
        block_bitmap[i] = 0xFF;  // Alles frei
    }

    // Erste Blöcke für Metadaten als belegt markieren
    uint32_t meta_blocks = 10;  // Superblock + INodes + Bitmap
    for(i = 0; i < meta_blocks; i++) {
        int byte = i / 8;
        int bit = i % 8;
        block_bitmap[byte] &= ~(1 << bit);
        superblock->free_blocks--;
    }

    // Root-Verzeichnis erstellen (INode 1)
    inode_table[1].id = 1;
    inode_table[1].type = 2;  // Verzeichnis
    inode_table[1].parent = 1;  // Root ist sein eigener Parent
    inode_table[1].created = 123456;  // Platzhalter Timestamp
    inode_table[1].modified = 123456;

    // Namen setzen
    inode_table[1].name[0] = '/';
    inode_table[1].name[1] = '\0';

    kprint("KFS formatted successfully!\n", COLOR_GREEN_ON_BLUE);
    kprint("Total blocks: ", COLOR_WHITE_ON_BLUE);

    char blocks_str[16];
    char* ptr = blocks_str;
    uint32_t n = superblock->total_blocks;
    if(n == 0) *ptr++ = '0';
    else {
        char temp[16];
        int j = 0;
        while(n > 0) { temp[j++] = '0' + (n % 10); n /= 10; }
        while(j > 0) *ptr++ = temp[--j];
    }
    *ptr = '\0';
    kprint(blocks_str, COLOR_CYAN_ON_BLUE);
    kprint(" (", COLOR_WHITE_ON_BLUE);

    ptr = blocks_str;
    n = superblock->free_blocks;
    if(n == 0) *ptr++ = '0';
    else {
        char temp[16];
        int j = 0;
        while(n > 0) { temp[j++] = '0' + (n % 10); n /= 10; }
        while(j > 0) *ptr++ = temp[--j];
    }
    *ptr++ = ' ';
    *ptr++ = 'f';
    *ptr++ = 'r';
    *ptr++ = 'e';
    *ptr++ = 'e';
    *ptr = '\0';

    kprint(blocks_str, COLOR_GREEN_ON_BLUE);
    kprint(")\n", COLOR_WHITE_ON_BLUE);
}

// KFS initialisieren (beim Boot)
void kfs_init(void) {
    kprint("Initializing KFS... ", COLOR_WHITE_ON_BLUE);

    // Prüfen ob schon formatiert
    if(superblock->magic != KFS_MAGIC) {
        // Nicht formatiert - automatisch formatieren
        kfs_format("KonsKernelFS");
    } else {
        kprint("[OK]\n", COLOR_GREEN_ON_BLUE);
        kprint("Volume: ", COLOR_WHITE_ON_BLUE);
        kprint(superblock->volume_name, COLOR_CYAN_ON_BLUE);
        kprint("\n", COLOR_WHITE_ON_BLUE);
    }

    current_dir_inode = 1;  // Root als aktuelles Verzeichnis
}

// Freien Block finden
int find_free_block(void) {
    uint32_t total_blocks = superblock->total_blocks;

    for(uint32_t i = 0; i < total_blocks; i++) {
        int byte = i / 8;
        int bit = i % 8;

        if(block_bitmap[byte] & (1 << bit)) {
            // Block ist frei - als belegt markieren
            block_bitmap[byte] &= ~(1 << bit);
            superblock->free_blocks--;
            return i;
        }
    }

    return -1;  // Kein freier Block
}

// Block freigeben
void free_block(int block_idx) {
    if(block_idx < 0) return;

    int byte = block_idx / 8;
    int bit = block_idx % 8;

    // Als frei markieren
    block_bitmap[byte] |= (1 << bit);
    superblock->free_blocks++;
}

// Freien INode finden
int find_free_inode(void) {
    for(int i = 1; i < MAX_FILES; i++) {  // Start bei 1 (0 ist ungültig)
        if(inode_table[i].id == 0) {
            return i;
        }
    }
    return -1;
}

// Datei suchen
int find_file(const char* name) {
    for(int i = 1; i < MAX_FILES; i++) {
        if(inode_table[i].id != 0 &&
           inode_table[i].parent == current_dir_inode) {
            if(strcmp(inode_table[i].name, name) == 0) {
                return i;  // INode Nummer zurückgeben
            }
        }
    }
    return -1;
}

// Datei erstellen
int kfs_create(const char* name, uint8_t type) {
    kprint("Creating: ", COLOR_WHITE_ON_BLUE);
    kprint(name, COLOR_CYAN_ON_BLUE);
    kprint("... ", COLOR_WHITE_ON_BLUE);

    // Prüfen ob Name schon existiert
    if(find_file(name) != -1) {
        kprint("[FAILED - exists]\n", COLOR_RED_ON_BLUE);
        return -1;
    }

    // Freien INode finden
    int inode_idx = find_free_inode();
    if(inode_idx == -1) {
        kprint("[FAILED - no inodes]\n", COLOR_RED_ON_BLUE);
        return -1;
    }

    // INode initialisieren
    struct kfs_inode* inode = &inode_table[inode_idx];
    inode->id = inode_idx;
    inode->type = type;  // 1=Datei, 2=Verzeichnis
    inode->parent = current_dir_inode;
    inode->size = 0;
    inode->created = 123456;  // Platzhalter
    inode->modified = 123456;

    // Namen kopieren
    int i = 0;
    while(name[i] && i < MAX_NAME_LEN - 1) {
        inode->name[i] = name[i];
        i++;
    }
    inode->name[i] = '\0';

    // Für Verzeichnisse: . und .. Einträge erstellen
    if(type == 2) {  // Verzeichnis
        // Block für Verzeichniseinträge anfordern
        int block = find_free_block();
        if(block == -1) {
            inode->id = 0;  // INode wieder freigeben
            kprint("[FAILED - no blocks]\n", COLOR_RED_ON_BLUE);
            return -1;
        }

        inode->blocks[0] = block;

        // Verzeichnisstruktur initialisieren
        struct kfs_dir_entry* dir = (struct kfs_dir_entry*)(data_blocks + (block * BLOCK_SIZE));

        // . Eintrag (aktuelles Verzeichnis)
        dir[0].inode_id = inode_idx;
        dir[0].name[0] = '.';
        dir[0].name[1] = '\0';

        // .. Eintrag (Elternverzeichnis)
        dir[1].inode_id = current_dir_inode;
        dir[1].name[0] = '.';
        dir[1].name[1] = '.';
        dir[1].name[2] = '\0';

        // Rest leeren
        for(i = 2; i < BLOCK_SIZE / sizeof(struct kfs_dir_entry); i++) {
            dir[i].inode_id = 0;
            dir[i].name[0] = '\0';
        }

        inode->size = 2 * sizeof(struct kfs_dir_entry);
    }

    kprint("[OK]\n", COLOR_GREEN_ON_BLUE);
    return inode_idx;
}

// Datei schreiben
int kfs_write(int inode_idx, const void* data, uint32_t size) {
    if(inode_idx <= 0 || inode_idx >= MAX_FILES) return -1;

    struct kfs_inode* inode = &inode_table[inode_idx];
    if(inode->id == 0) return -1;  // INode existiert nicht

    // Alte Daten freigeben
    for(int i = 0; i < 16; i++) {
        if(inode->blocks[i] != 0) {
            free_block(inode->blocks[i]);
            inode->blocks[i] = 0;
        }
    }

    // Neue Blöcke zuweisen
    uint32_t blocks_needed = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    if(blocks_needed > 16) {
        kprint("File too large! (max 8KB)\n", COLOR_RED_ON_BLUE);
        return -1;
    }

    const uint8_t* src = (const uint8_t*)data;
    uint32_t written = 0;

    for(uint32_t i = 0; i < blocks_needed; i++) {
        int block = find_free_block();
        if(block == -1) {
            kprint("Out of disk space!\n", COLOR_RED_ON_BLUE);
            return written;  // Teilweise geschrieben
        }

        inode->blocks[i] = block;

        // Daten kopieren
        uint32_t to_copy = size - written;
        if(to_copy > BLOCK_SIZE) to_copy = BLOCK_SIZE;

        uint8_t* dst = data_blocks + (block * BLOCK_SIZE);
        for(uint32_t j = 0; j < to_copy; j++) {
            dst[j] = src[written + j];
        }

        written += to_copy;
    }

    inode->size = size;
    inode->modified = 123456;  // Platzhalter

    return written;
}

// Datei lesen
int kfs_read(int inode_idx, void* buffer, uint32_t size) {
    if(inode_idx <= 0 || inode_idx >= MAX_FILES) return -1;

    struct kfs_inode* inode = &inode_table[inode_idx];
    if(inode->id == 0) return -1;

    // Nicht mehr lesen als Dateigröße
    if(size > inode->size) size = inode->size;

    uint8_t* dst = (uint8_t*)buffer;
    uint32_t read = 0;

    for(int i = 0; i < 16 && read < size; i++) {
        if(inode->blocks[i] == 0) break;

        uint8_t* src = data_blocks + (inode->blocks[i] * BLOCK_SIZE);
        uint32_t to_read = size - read;
        if(to_read > BLOCK_SIZE) to_read = BLOCK_SIZE;

        for(uint32_t j = 0; j < to_read; j++) {
            dst[read + j] = src[j];
        }

        read += to_read;
    }

    return read;
}

// Datei löschen
int kfs_delete(const char* name) {
    kprint("Deleting: ", COLOR_WHITE_ON_BLUE);
    kprint(name, COLOR_CYAN_ON_BLUE);
    kprint("... ", COLOR_WHITE_ON_BLUE);

    int inode_idx = find_file(name);
    if(inode_idx == -1) {
        kprint("[FAILED - not found]\n", COLOR_RED_ON_BLUE);
        return -1;
    }

    struct kfs_inode* inode = &inode_table[inode_idx];

    // Blöcke freigeben
    for(int i = 0; i < 16; i++) {
        if(inode->blocks[i] != 0) {
            free_block(inode->blocks[i]);
        }
    }

    // INode freigeben
    inode->id = 0;
    inode->name[0] = '\0';
    inode->size = 0;

    kprint("[OK]\n", COLOR_GREEN_ON_BLUE);
    return 0;
}

// ========================
// COMMAND HISTORY FUNCTIONS
// ========================

void show_history_command(void) {
    if (history_index < 0 || history_index >= history_count) {
        return;
    }

    // WICHTIG: Zur Prompt-Position zurück
    set_cursor(6, cursor_y);

    // Clear line from prompt position to end
    for (int i = 0; i < SCREEN_WIDTH - 6; i++) {
        print_char_color(' ', 6 + i, cursor_y, COLOR_WHITE_ON_BLUE);
    }

    // Zurück zum Prompt
    set_cursor(6, cursor_y);

    // Show history command
    kprint_no_scroll(cmd_history[history_index], COLOR_WHITE_ON_BLUE);

    // Update buffer
    strcpy(cmd_buffer, cmd_history[history_index]);
    cmd_pos = strlen(cmd_buffer);

    // Cursor ans Ende setzen
    set_cursor(6 + cmd_pos, cursor_y);
}

void history_add(const char* cmd) {
    // Don't add empty commands
    if (strlen(cmd) == 0) return;

    // Don't add duplicate of last command
    if (history_count > 0 && strcmp(cmd_history[history_count-1], cmd) == 0) {
        return;
    }

    // Add to history if space available
    if (history_count < CMD_HISTORY_SIZE) {
        strcpy(cmd_history[history_count], cmd);
        history_count++;
        history_index = history_count;  // Point to "new command" position
    } else {
        // Shift history up (remove oldest)
        for (int i = 1; i < CMD_HISTORY_SIZE; i++) {
            strcpy(cmd_history[i-1], cmd_history[i]);
        }
        strcpy(cmd_history[CMD_HISTORY_SIZE-1], cmd);
        history_index = CMD_HISTORY_SIZE;
    }
}

// ========================
// SHELL COMMANDS WITH MEMORY COMMANDS
// ========================

// ========================
// KFS FUNCTION PROTOTYPES (wenn noch nicht global deklariert)
// ========================
#ifndef KFS_FUNCTIONS_DECLARED
#define KFS_FUNCTIONS_DECLARED

// Memory functions
void* kmalloc_safe(unsigned int size);
void kfree_safe(void* ptr);
void print_memory_info(void);
void debug_memory(void);

// File system functions
void kfs_init(void);
void kfs_format(const char* volume_name);
int kfs_create(const char* name, uint8_t type);
int kfs_write(int inode_idx, const void* data, uint32_t size);
int kfs_read(int inode_idx, void* buffer, uint32_t size);
int kfs_delete(const char* name);
int find_file(const char* name);

#endif // Stopped HERE WITH G!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void execute_command(char *cmd) { // g x2
    kprint("\nDEBUG: Command='", COLOR_YELLOW_ON_BLUE);
    kprint(cmd, COLOR_WHITE_ON_DARK_GRAY);
    kprint("'\n", COLOR_YELLOW_ON_BLUE);

    // Skip leading spaces
    while(*cmd == ' ') cmd++;

    // EMPTY COMMAND
    if(*cmd == '\0') {
        return;
    }

    // HELP COMMAND
    else if (strcmp(cmd, "help") == 0) {
        kprint("\n=== KonsShell Commands ===\n", COLOR_CYAN_ON_BLUE); // all g
        kprint("help     - Show this help\n", COLOR_GREEN_ON_BLUE);
        kprint("clear    - Clear screen\n", COLOR_GREEN_ON_BLUE);
        kprint("echo     - Print text\n", COLOR_GREEN_ON_BLUE);
        kprint("info     - System information\n", COLOR_GREEN_ON_BLUE);
        kprint("mem      - Memory information\n", COLOR_GREEN_ON_BLUE);
        kprint("mtest    - Test memory allocation\n", COLOR_GREEN_ON_BLUE);
        kprint("mdebug   - Memory debug info\n", COLOR_GREEN_ON_BLUE);
        kprint("color    - Change text color\n", COLOR_GREEN_ON_BLUE);
        kprint("reboot   - Simulate reboot\n", COLOR_YELLOW_ON_BLUE);
        kprint("about    - About KonsKernel\n", COLOR_GREEN_ON_BLUE);
        kprint("debug    - Toggle debug mode\n", COLOR_YELLOW_ON_BLUE);
        // DATEISYSTEM COMMANDS HINZUFÜGEN:
        kprint("ls/dir   - List files\n", COLOR_GREEN_ON_BLUE); // all g
        kprint("touch    - Create file\n", COLOR_GREEN_ON_BLUE);
        kprint("mkdir    - Create directory\n", COLOR_GREEN_ON_BLUE);
        kprint("cat      - Show file\n", COLOR_GREEN_ON_BLUE);
        kprint("rm       - Delete file\n", COLOR_GREEN_ON_BLUE);
        kprint("fsinfo/df- Filesystem info\n", COLOR_GREEN_ON_BLUE);
        kprint("format   - Format filesystem\n", COLOR_RED_ON_BLUE);
    }

    // CLEAR COMMAND
    else if (strcmp(cmd, "clear") == 0) {
        clear_screen(COLOR_DARK_GRAY);
        kprint("kons> ", COLOR_WHITE_ON_DARK_GRAY);
        set_cursor(6, cursor_y);
    }

    // ECHO COMMAND (BOTH NORMAL AND > VERSION)
    else if (strstart(cmd, "echo ")) {
        char* text = cmd + 5;  // Pointer nach "echo "

        // 1. Suche nach " > " im Text (manuell, ohne strstr)
        char* redirect_pos = NULL;
        char* current_pos = text;

        // Gehe durch den gesamten Text
        while(*current_pos) {
            // Prüfe ob an aktueller Position " > " steht
            if(current_pos[0] == ' ' &&
               current_pos[1] == '>' &&
               current_pos[2] == ' ') {
                redirect_pos = current_pos;
                break;  // Gefunden, Suche beenden
            }
            current_pos++;
        }

        // 2. Wenn " > " gefunden wurde (Redirection)
        if(redirect_pos != NULL) {
            // Teile den String an " > "
            *redirect_pos = '\0';           // Text endet vor " > "
            char* filename = redirect_pos + 3;  // Zeiger auf Dateinamen (nach " > ")

            // Überspringe Leerzeichen vor Dateinamen
            while(*filename == ' ') {
                filename++;
            }

            // Prüfe ob ein Dateiname angegeben wurde
            if(*filename == '\0') {
                kprint("Error: Missing filename after '>'\n", COLOR_RED_ON_BLUE); // g
            } else {
                // Datei suchen (existiert sie schon?)
                int inode_idx = find_file(filename);

                // Wenn nicht existiert, neu erstellen
                if(inode_idx == -1) {
                    inode_idx = kfs_create(filename, 1);  // Typ 1 = normale Datei
                }

                // Wenn Datei erstellt/gefunden wurde
                if(inode_idx != -1) {
                    // Text in Datei schreiben
                    kfs_write(inode_idx, text, strlen(text));

                    // Erfolgsmeldung // g x3
                    kprint("Written to file: '", COLOR_GREEN_ON_BLUE);
                    kprint(filename, COLOR_WHITE_ON_DARK_GRAY);
                    kprint("'\n", COLOR_GREEN_ON_BLUE);
                } else {
                    kprint("Error: Could not create/write file\n", COLOR_RED_ON_BLUE);
                }
            }
        }
        // 3. Kein " > " gefunden -> normaler echo (nur ausgeben)
        else {
            kprint("\n", COLOR_WHITE_ON_DARK_GRAY);
            kprint(text, COLOR_CYAN_ON_BLUE); // g
            kprint("\n", COLOR_WHITE_ON_DARK_GRAY);
        }
    }

    // INFO COMMAND
    else if (strcmp(cmd, "info") == 0) { // g x3
        kprint("\n=== System Information ===\n", COLOR_CYAN_ON_BLUE);
        kprint("Kernel:   KonsKernel v1.4.0\n", COLOR_WHITE_ON_DARK_GRAY);
        kprint("Shell:    KonsShell v1.1\n", COLOR_WHITE_ON_DARK_GRAY);
        kprint("Arch:     32-bit Protected Mode\n", COLOR_WHITE_ON_DARK_GRAY);
        kprint("Interrupts: Enabled\n", COLOR_GREEN_ON_BLUE);
        kprint("Memory:   Full Memory Management\n", COLOR_GREEN_ON_BLUE);
        kprint("Heap:     1MB available\n", COLOR_WHITE_ON_DARK_GRAY);
        kprint("Display:  80x25 VGA Text\n", COLOR_WHITE_ON_DARK_GRAY);
    }

    // MEMORY COMMAND
    else if (strcmp(cmd, "mem") == 0 || strcmp(cmd, "memory") == 0) {
        print_memory_info();
    }

    // MEMORY TEST COMMAND // g x3
    else if (strcmp(cmd, "mtest") == 0) {
        kprint("\n=== Memory Allocation Test ===\n", COLOR_CYAN_ON_BLUE);

        // Test 1: Kleine Allokation
        kprint("Test 1: Allocating 64 bytes... ", COLOR_WHITE_ON_DARK_GRAY);
        void* ptr1 = kmalloc_safe(64);
        if(ptr1) {
            kprint("[OK]\n", COLOR_GREEN_ON_BLUE);
        } else {
            kprint("[FAILED]\n", COLOR_RED_ON_BLUE);
        }

        // Test 2: Mittlere Allokation
        kprint("Test 2: Allocating 1024 bytes... ", COLOR_WHITE_ON_DARK_GRAY);
        void* ptr2 = kmalloc_safe(1024);
        if(ptr2) {
            kprint("[OK]\n", COLOR_GREEN_ON_BLUE); // g x2
        } else {
            kprint("[FAILED]\n", COLOR_RED_ON_BLUE);
        }

        // Test 3: Große Allokation
        kprint("Test 3: Allocating 4096 bytes... ", COLOR_WHITE_ON_DARK_GRAY);
        void* ptr3 = kmalloc_safe(4096);
        if(ptr3) {
            kprint("[OK]\n", COLOR_GREEN_ON_BLUE); // g x2
        } else {
            kprint("[FAILED]\n", COLOR_RED_ON_BLUE);
        }

        // Test 4: Zu große Allokation (sollte fehlschlagen)
        kprint("Test 4: Allocating 2MB (should fail)... ", COLOR_WHITE_ON_DARK_GRAY);
        void* ptr4 = kmalloc_safe(2 * 1024 * 1024); // g x2
        if(!ptr4) {
            kprint("[OK - Expected failure]\n", COLOR_YELLOW_ON_BLUE);
        } else {
            kprint("[FAILED - Should not succeed]\n", COLOR_RED_ON_BLUE);
        }

        // Freigabe testen
        kprint("Test 5: Freeing allocations... ", COLOR_WHITE_ON_DARK_GRAY);
        if(ptr1) kfree_safe(ptr1);
        if(ptr2) kfree_safe(ptr2);
        if(ptr3) kfree_safe(ptr3);
        kprint("[DONE]\n", COLOR_GREEN_ON_BLUE); // g

        print_memory_info();
    }

    // MEMORY DEBUG COMMAND
    else if (strcmp(cmd, "mdebug") == 0) {
        debug_memory();
    }

    // COLOR COMMAND
    else if (strstart(cmd, "color ")) {
        char* color = cmd + 6;
        kprint("\n", COLOR_WHITE_ON_DARK_GRAY);
        if (strcmp(color, "red") == 0) {
            kprint("Text color set to RED\n", COLOR_RED_ON_BLUE); // g
        }
        else if (strcmp(color, "green") == 0) {
            kprint("Text color set to GREEN\n", COLOR_GREEN_ON_BLUE); // g
        }
        else if (strcmp(color, "blue") == 0) {
            kprint("Text color set to BLUE\n", COLOR_BLUE_ON_BLUE); // g
        }
        else if (strcmp(color, "yellow") == 0) {
            kprint("Text color set to YELLOW\n", COLOR_YELLOW_ON_BLUE); // g
        }
        else if (strcmp(color, "cyan") == 0) {
            kprint("Text color set to CYAN\n", COLOR_CYAN_ON_BLUE); // g
        }
        else if (strcmp(color, "white") == 0) {
            kprint("Text color set to WHITE\n", COLOR_WHITE_ON_DARK_GRAY);
        }
        else {
            kprint("Unknown color. Available: red, green, blue, yellow, cyan, white\n", COLOR_RED_ON_BLUE); // g
        }
    }

    // REBOOT COMMAND
    else if (strcmp(cmd, "reboot") == 0) { // g x3
        kprint("\n", COLOR_YELLOW_ON_BLUE);
        kprint("Simulating system reboot...\n", COLOR_YELLOW_ON_BLUE);
        kprint("(In real hardware, this would trigger a reset)\n", COLOR_YELLOW_ON_BLUE);
        kprint("Countdown: 3... ", COLOR_YELLOW_ON_BLUE);

        // Fake delay
        for(int i = 0; i < 10000000; i++) asm volatile("nop");
        kprint("2... ", COLOR_YELLOW_ON_BLUE); // g x2
        for(int i = 0; i < 10000000; i++) asm volatile("nop");
        kprint("1...\n", COLOR_YELLOW_ON_BLUE);
        for(int i = 0; i < 10000000; i++) asm volatile("nop");

        kprint("Just kidding! System is still running.\n", COLOR_GREEN_ON_BLUE); // g x2
        kprint("Use QEMU reset (Ctrl+Alt+Del) to actually reboot.\n", COLOR_GREEN_ON_BLUE);
    }

    // ABOUT COMMAND
    else if (strcmp(cmd, "about") == 0) {
        kprint("\n=== KonsKernel v1.4.0 ===\n", COLOR_CYAN_ON_BLUE); // g
        kprint("A 32-bit hobby operating system\n", COLOR_WHITE_ON_DARK_GRAY);
        kprint("Created with help from DeepSeek AI\n", COLOR_WHITE_ON_DARK_GRAY);
        kprint("Features:\n", COLOR_WHITE_ON_DARK_GRAY);
        kprint("  • Protected Mode (GDT)\n", COLOR_GREEN_ON_BLUE); // g
        kprint("  • Full Interrupt System (PIC/IDT)\n", COLOR_GREEN_ON_BLUE); // g
        kprint("  • Timer & Keyboard drivers\n", COLOR_GREEN_ON_BLUE); // g
        kprint("  • SAFE Memory Management (Heap + Tracking)\n", COLOR_GREEN_ON_BLUE); // g
        kprint("  • Working shell with commands\n", COLOR_GREEN_ON_BLUE); // g
        kprint("  • VGA text mode display\n", COLOR_GREEN_ON_BLUE); // g
    }

    // DEBUG COMMAND
    else if (strcmp(cmd, "debug") == 0) {
        debug_mode = !debug_mode;  // Toggle
        kprint("\nDebug mode: ", COLOR_CYAN_ON_BLUE); // g
        if (debug_mode) {
            kprint("ON\n", COLOR_GREEN_ON_BLUE); // g
            kprint("Showing all keyboard scancodes...\n", COLOR_WHITE_ON_DARK_GRAY);
        } else {
            kprint("OFF\n", COLOR_RED_ON_BLUE); // g
            kprint("Normal output mode\n", COLOR_WHITE_ON_DARK_GRAY);
        }
    }

    // ============================================
    // DATEISYSTEM COMMANDS (MÜSSEN VOR "else" SEIN!)
    // ============================================

    // LS Command - Dateien auflisten
    else if (strcmp(cmd, "ls") == 0 || strcmp(cmd, "dir") == 0) {
        kprint("\n", COLOR_WHITE_ON_DARK_GRAY);

        int count = 0;
        for(int i = 1; i < MAX_FILES; i++) {
            if(inode_table[i].id != 0 &&
               inode_table[i].parent == current_dir_inode) {

                if(inode_table[i].type == 2) {
                    kprint("[DIR]  ", COLOR_CYAN_ON_BLUE); // g
                } else {
                    kprint("[FILE] ", COLOR_GREEN_ON_BLUE); // g
                }

                kprint(inode_table[i].name, COLOR_WHITE_ON_DARK_GRAY);

                // Größe anzeigen für Dateien
                if(inode_table[i].type == 1) {
                    kprint(" (", COLOR_WHITE_ON_DARK_GRAY);

                    char size_str[16];
                    char* ptr = size_str;
                    uint32_t n = inode_table[i].size;

                    if(n == 0) *ptr++ = '0';
                    else {
                        char temp[16];
                        int j = 0;
                        while(n > 0) { temp[j++] = '0' + (n % 10); n /= 10; }
                        while(j > 0) *ptr++ = temp[--j];
                    }
                    *ptr++ = 'B';
                    *ptr = '\0';

                    kprint(size_str, COLOR_YELLOW_ON_BLUE); // g
                    kprint(")", COLOR_WHITE_ON_DARK_GRAY);
                }

                kprint("\n", COLOR_WHITE_ON_DARK_GRAY);
                count++;
            }
        }

        if(count == 0) {
            kprint("Directory empty\n", COLOR_YELLOW_ON_BLUE); // g
        } else {
            kprint("\nTotal: ", COLOR_WHITE_ON_DARK_GRAY);

            char count_str[16];
            char* ptr = count_str;
            int n = count;

            if(n == 0) *ptr++ = '0';
            else {
                char temp[16];
                int i = 0;
                while(n > 0) { temp[i++] = '0' + (n % 10); n /= 10; }
                while(i > 0) *ptr++ = temp[--i];
            }
            *ptr++ = ' ';
            *ptr++ = 'e';
            *ptr++ = 'n';
            *ptr++ = 't';
            *ptr++ = 'r';
            *ptr++ = 'i';
            *ptr++ = 'e';
            *ptr++ = 's';
            *ptr = '\0';

            kprint(count_str, COLOR_CYAN_ON_BLUE); // g
            kprint("\n", COLOR_WHITE_ON_DARK_GRAY);
        }
    }

    // TOUCH COMMAND - Datei erstellen
    else if (strstart(cmd, "touch ")) {
        char* filename = cmd + 6;  // Nach "touch "

        // Leerzeichen überspringen
        while(*filename == ' ') {
            filename++;
        }

        // Prüfen ob Dateiname angegeben
        if(*filename == '\0') {
            kprint("Usage: touch <filename>\n", COLOR_RED_ON_BLUE); // g
        } else {
            // Datei erstellen (Typ 1 = normale Datei)
            int result = kfs_create(filename, 1);

            if(result == -1) {
                kprint("Error: Could not create file '", COLOR_RED_ON_BLUE); // g
                kprint(filename, COLOR_WHITE_ON_DARK_GRAY);
                kprint("'\n", COLOR_RED_ON_BLUE); // g
            } else {
                kprint("File created: '", COLOR_GREEN_ON_BLACK); // g
                kprint(filename, COLOR_WHITE_ON_DARK_GRAY);
                kprint("'\n", COLOR_GREEN_ON_BLUE); // g
            }
        }
    }

    // else if (strcmp(cmd, "mkdir") == 0) { was wrong
    else if (strstart(cmd, "mkdir ")) {
        char* dirname = cmd + 6;
        while(*dirname == ' ') dirname++;

        if(*dirname == '\0') {
            kprint("Usage: mkdir <dirname>\n", COLOR_RED_ON_BLACK); // g
        } else {
            // Verzeichnis erstellen (Typ 2 = Verzeichnis)
            int result = kfs_create(dirname, 2);

            if(result == -1) {
                kprint("Error creating directory: ", COLOR_RED_ON_BLACK); // g
                kprint(dirname, COLOR_WHITE_ON_DARK_GRAY);
                kprint("\n", COLOR_RED_ON_BLACK); // g
            } else {
                kprint("Directory created: ", COLOR_GREEN_ON_BLACK); // g
                kprint(dirname, COLOR_WHITE_ON_DARK_GRAY);
                kprint("\n", COLOR_GREEN_ON_BLACK); // g
            }
        }
    }

    // CAT Command - Datei anzeigen
    else if (strstart(cmd, "cat ")) {
        char* filename = cmd + 4;
        while(*filename == ' ') filename++;

        if(*filename == '\0') {
            kprint("Usage: cat <filename>\n", COLOR_RED_ON_BLACK); // g
        } else {
            int inode_idx = find_file(filename);
            if(inode_idx == -1) {
                kprint("File not found: ", COLOR_RED_ON_BLACK); // g
                kprint(filename, COLOR_WHITE_ON_DARK_GRAY);
                kprint("\n", COLOR_RED_ON_BLACK); // g
            } else {
                kprint("\n", COLOR_WHITE_ON_DARK_GRAY);

                // Datei lesen
                char buffer[4096];
                int bytes_read = kfs_read(inode_idx, buffer, sizeof(buffer));

                if(bytes_read > 0) {
                    // Null-Terminator für kprint
                    if(bytes_read < sizeof(buffer)) {
                        buffer[bytes_read] = '\0';
                    } else {
                        buffer[sizeof(buffer)-1] = '\0';
                    }
                    kprint(buffer, COLOR_WHITE_ON_DARK_GRAY);
                }
                kprint("\n", COLOR_WHITE_ON_DARK_GRAY);
            }
        }
    }

    // RM Command - Datei löschen
    else if (strstart(cmd, "rm ")) {
        char* filename = cmd + 3;
        while(*filename == ' ') filename++;

        if(*filename == '\0') {
            kprint("Usage: rm <filename>\n", COLOR_RED_ON_BLUE); // g
        } else {
            int result = kfs_delete(filename);
            if(result == -1) {
                kprint("Error deleting file: ", COLOR_RED_ON_BLUE); // g
                kprint(filename, COLOR_WHITE_ON_DARK_GRAY);
                kprint("\n", COLOR_RED_ON_BLUE); // g
            } else {
                kprint("File deleted: ", COLOR_GREEN_ON_BLUE); // g
                kprint(filename, COLOR_WHITE_ON_DARK_GRAY);
                kprint("\n", COLOR_GREEN_ON_BLUE); // g
            }
        }
    }

    // FILESYS Command - Dateisystem Info
    else if (strcmp(cmd, "fsinfo") == 0 || strcmp(cmd, "df") == 0) {
        kprint("\n=== KFS Information ===\n", COLOR_CYAN_ON_BLUE); // g

        kprint("Volume:     ", COLOR_WHITE_ON_DARK_GRAY);
        kprint(superblock->volume_name, COLOR_CYAN_ON_BLUE); // g
        kprint("\n", COLOR_WHITE_ON_DARK_GRAY);

        kprint("Total:      ", COLOR_WHITE_ON_DARK_GRAY);
        char total_str[32];
        char* ptr = total_str;
        uint32_t total_kb = (superblock->total_blocks * BLOCK_SIZE) / 1024;
        uint32_t n = total_kb;

        if(n == 0) *ptr++ = '0';
        else {
            char temp[32];
            int i = 0;
            while(n > 0) { temp[i++] = '0' + (n % 10); n /= 10; }
            while(i > 0) *ptr++ = temp[--i];
        }
        *ptr++ = 'K';
        *ptr++ = 'B';
        *ptr = '\0';
        kprint(total_str, COLOR_WHITE_ON_DARK_GRAY);
        kprint("\n", COLOR_WHITE_ON_DARK_GRAY);

        kprint("Free:       ", COLOR_WHITE_ON_DARK_GRAY);
        char free_str[32];
        ptr = free_str;
        uint32_t free_kb = (superblock->free_blocks * BLOCK_SIZE) / 1024;
        n = free_kb;

        if(n == 0) *ptr++ = '0';
        else {
            char temp[32];
            int i = 0;
            while(n > 0) { temp[i++] = '0' + (n % 10); n /= 10; }
            while(i > 0) *ptr++ = temp[--i];
        }
        *ptr++ = 'K';
        *ptr++ = 'B';
        *ptr++ = ' ';
        *ptr++ = '(';

        // Prozent berechnen
        uint32_t percent = (free_kb * 100) / total_kb;
        n = percent;
        if(n == 0) *ptr++ = '0';
        else {
            char temp[32];
            int i = 0;
            while(n > 0) { temp[i++] = '0' + (n % 10); n /= 10; }
            while(i > 0) *ptr++ = temp[--i];
        }
        *ptr++ = '%';
        *ptr++ = ')';
        *ptr = '\0';

        if(percent > 50) {
            kprint(free_str, COLOR_GREEN_ON_BLUE); // g
        } else if(percent > 20) {
            kprint(free_str, COLOR_YELLOW_ON_BLUE); // g
        } else {
            kprint(free_str, COLOR_RED_ON_BLUE); // g
        }
        kprint("\n", COLOR_WHITE_ON_DARK_GRAY);

        kprint("Used:       ", COLOR_WHITE_ON_DARK_GRAY);
        char used_str[32];
        ptr = used_str;
        uint32_t used_kb = total_kb - free_kb;
        n = used_kb;

        if(n == 0) *ptr++ = '0';
        else {
            char temp[32];
            int i = 0;
            while(n > 0) { temp[i++] = '0' + (n % 10); n /= 10; }
            while(i > 0) *ptr++ = temp[--i];
        }
        *ptr++ = 'K';
        *ptr++ = 'B';
        *ptr = '\0';
        kprint(used_str, COLOR_YELLOW_ON_BLACK); // g
        kprint("\n", COLOR_WHITE_ON_DARK_GRAY);

        kprint("Block size: ", COLOR_WHITE_ON_DARK_GRAY);
        char block_str[16];
        ptr = block_str;
        n = BLOCK_SIZE;
        if(n == 0) *ptr++ = '0';
        else {
            char temp[16];
            int i = 0;
            while(n > 0) { temp[i++] = '0' + (n % 10); n /= 10; }
            while(i > 0) *ptr++ = temp[--i];
        }
        *ptr++ = ' ';
        *ptr++ = 'b';
        *ptr++ = 'y';
        *ptr++ = 't';
        *ptr++ = 'e';
        *ptr++ = 's';
        *ptr = '\0';
        kprint(block_str, COLOR_CYAN_ON_BLUE); // GRay
        kprint("\n", COLOR_WHITE_ON_DARK_GRAY);
    }

    // FORMAT Command - Dateisystem formatieren
    else if (strstart(cmd, "format ")) {
        kprint("\nWARNING: This will erase ALL data!\n", COLOR_RED_ON_BLUE); // Gray
        kprint("Type 'yes' to confirm: ", COLOR_YELLOW_ON_BLACK); // GRAY

        // Hier müsstest du auf Bestätigung warten
        // Für jetzt einfach formatieren
        kfs_format("KonsKernelFS");
    }

    else if (strcmp(cmd, "testfs") == 0) {
        kprint("\n=== KFS SIMPLE TEST ===\n", COLOR_CYAN_ON_BLUE); // Gray

        // 1. Versuche Datei zu erstellen (direkter Aufruf)
        kprint("1. Direct kfs_create call... ", COLOR_WHITE_ON_DARK_GRAY);
        int result = kfs_create("testfile.tmp", 1);

        if(result == -1) {
            kprint("FAILED\n", COLOR_RED_ON_BLUE);
            kprint("   Reason: ", COLOR_RED_ON_BLUE);

            // Mögliche Gründe
            if(superblock == NULL) {
                kprint("superblock is NULL\n", COLOR_RED_ON_BLUE);
            } else if(inode_table == NULL) {
                kprint("inode_table is NULL\n", COLOR_RED_ON_BLUE);
            } else {
                kprint("unknown\n", COLOR_RED_ON_BLUE); // Gray
            }
        } else {
            kprint("SUCCESS (inode=", COLOR_GREEN_ON_BLUE); // Gray

            char num[16];
            char* p = num;
            int n = result;
            if(n == 0) *p++ = '0';
            else {
                char temp[16];
                int i = 0;
                while(n > 0) { temp[i++] = '0' + (n % 10); n /= 10; }
                while(i > 0) *p++ = temp[--i];
            }
            *p = '\0';
            kprint(num, COLOR_WHITE_ON_DARK_GRAY);
            kprint(")\n", COLOR_GREEN_ON_BLUE); // gray

            // Sofort löschen
            kfs_delete("testfile.tmp");
        }

        // 2. Zeige ob KFS initialisiert ist
        kprint("2. KFS Status: ", COLOR_WHITE_ON_DARK_GRAY);
        if(superblock && superblock->magic == KFS_MAGIC) {
            kprint("INITIALIZED (magic OK)\n", COLOR_GREEN_ON_BLACK); // gray
            kprint("   Volume: '", COLOR_WHITE_ON_BLUE);
            kprint(superblock->volume_name, COLOR_CYAN_ON_BLACK); // gray
            kprint("'\n", COLOR_WHITE_ON_DARK_GRAY);
        } else {
            kprint("NOT INITIALIZED\n", COLOR_RED_ON_BLACK); // define
        }

        // 3. Aktuelles Verzeichnis
        kprint("3. Current dir inode: ", COLOR_WHITE_ON_DARK_GRAY);

        char num[16];
        char* p = num;
        int n = current_dir_inode;
        if(n == 0) *p++ = '0';
        else {
            char temp[16];
            int i = 0;
            while(n > 0) { temp[i++] = '0' + (n % 10); n /= 10; }
            while(i > 0) *p++ = temp[--i];
        }
        *p = '\0';
        kprint(num, COLOR_WHITE_ON_DARK_GRAY);
        kprint("\n", COLOR_WHITE_ON_DARK_GRAY);
    }

    // UNKNOWN COMMAND (MUSS ALS LETZTES KOMMEN!)
    else {
        kprint("\nUnknown command: '", COLOR_RED_ON_BLACK); // define on dark gray
        kprint(cmd, COLOR_WHITE_ON_DARK_GRAY);
        kprint("'\n", COLOR_RED_ON_BLUE); //same as ln 2498
        kprint("Type 'help' for available commands\n", COLOR_RED_ON_BLUE); // same as in ln 2498
    }
}


//(milli)seconds delay
void delay_ms(int milliseconds) {
    // Sehr ungenau, aber einfach
    for(int i = 0; i < milliseconds * 1000; i++) {
        asm volatile("nop");
    }
}

// ASCII boot screen
void show_ascii_boot(void) {
    clear_screen(THEME_BACKGROUND);

    const char* art[] = {
        "  _  __            _  __                 _",
        " | |/ /___ _ _  __| |/ /___ _ _ _ _  ___| |",
        " | ' </ _ \ ' \(_-< ' </ -_) '_| ' \/ -_) |",
        " |_|\_\___/_||_/__/_|\_\___|_| |_||_\___|_|",
    };

    // Zeile für Zeile mit Delay
        for(int i = 0; i < 6; i++) {
            kprint_at(art[i], 15, 5 + i, TXT_INFO);
            delay_ms(15000);  // 150ms zwischen Zeilen
        }

        kprint_at("Initializing", 30, 12, TXT_NORMAL);

        // Drei Punkte nacheinander
        for(int i = 0; i < 3; i++) {
            kprint_at(".", 43 + i, 12, TXT_INFO);
            delay_ms(300);  // 300ms zwischen Punkten
        }

        delay_ms(150000);  // Halbe Sekunde warten
    }

    // Einfacher Renderer
void render_editor_content(char* buffer, int len, int scroll_y,
                            int start_x, int start_y, int width, int height) {
    int line = 0;
    int col = 0;

    // Editor-Bereich leeren
    for(int y = 0; y < height; y++) {
        for(int x = 0; x < width; x++) {
            print_char_color(' ', start_x + x, start_y + y, TXT_NORMAL);
        }
    }

    // Text rendern
    for(int i = 0; i < len && line < height; i++) {
        char c = buffer[i];

        if(c == '\n') {
            line++;
            col = 0;
        } else if(col < width) {
            print_char_color(c, start_x + col, start_y + line, TXT_NORMAL);
            col++;
        }

        if(line >= scroll_y + height) break;
    }
}

//KonsEdit
void konsedit(const char* filename) {
    clear_screen(THEME_BACKGROUND);

    // Header
    kprint_at("┌──────────────────────────────────────────────────────────────┐", 0, 0, TXT_INFO);
    kprint_at("│                         KonsEdit v1.0                        │", 0, 1, TXT_NORMAL);
    kprint_at("└──────────────────────────────────────────────────────────────┘", 0, 2, TXT_INFO);

    // Status Bar
    kprint_at("File: ", 2, 23, TXT_NORMAL);
    kprint_at(filename, 8, 23, TXT_SUCCESS);
    kprint_at("Ctrl+S: Save  Ctrl+Q: Quit  Ctrl+F: Find", 30, 23, TXT_WARNING);

    // Editor Bereich zeichnen
    for(int y = 3; y < 23; y++) {
        kprint_at("│", 0, y, TXT_INFO);
        kprint_at("│", 79, y, TXT_INFO);
    }
    kprint_at("└──────────────────────────────────────────────────────────────┘", 0, 23, TXT_INFO);

    // Datei laden (wenn existiert)
    char buffer[MAX_EDIT_SIZE];
    int buffer_len = 0;
    int cursor_x = 2;
    int cursor_y = 4;
    int scroll_y = 0;

    // Existierende Datei laden
    int inode_idx = find_file(filename);
    if(inode_idx != -1) {
        buffer_len = kfs_read(inode_idx, buffer, MAX_EDIT_SIZE);
        kprint_at("Loaded existing file", 50, 23, TXT_SUCCESS);
    } else {
        kprint_at("New file", 50, 23, TXT_INFO);
    }

    // Editor Loop
    int editing = 1;
    while(editing) {
        // Buffer in Editor-Fenster rendern
        render_editor_content(buffer, buffer_len, scroll_y, 2, 4, EDITOR_WIDTH, EDITOR_HEIGHT);

        // Cursor positionieren
        set_cursor(cursor_x, cursor_y);

        // Auf Tastendruck warten
        asm volatile("hlt");
        // (Tastatur-Handler muss hier speziell für Editor sein)
    }
}



// ========================
// DISPLAY FUNCTIONS
// ========================

void print_char_color(char c, int x, int y, unsigned char color) {
    unsigned short* buffer = (unsigned short*)VIDEO_MEMORY;
    int position = y * SCREEN_WIDTH + x;
    unsigned short destination = (color << 8) | c;
    buffer[position] = destination;
}

void print_char(char c, int x, int y) {
    print_char_color(c, x, y, COLOR_WHITE_ON_BLACK);
}

void kprint(const char* str, unsigned char color) {
    for (int i = 0; str[i] != '\0'; i++) {
        char c = str[i];

        if (c == '\n') {
            cursor_x = 0;
            cursor_y++;

            // Check if we need to scroll
            if (cursor_y >= SCREEN_HEIGHT) {
                scroll_screen();
                cursor_y = SCREEN_HEIGHT - 1;
            }
        } else {
            // Print character at current position
            print_char_color(c, cursor_x, cursor_y, color);
            cursor_x++;

            // Check for line wrap
            if (cursor_x >= SCREEN_WIDTH) {
                cursor_x = 0;
                cursor_y++;

                // Check if we need to scroll
                if (cursor_y >= SCREEN_HEIGHT) {
                    scroll_screen();
                    cursor_y = SCREEN_HEIGHT - 1;
                }
            }
        }
    }

    // Update hardware cursor
    set_cursor(cursor_x, cursor_y);
}

void kprint_no_scroll(const char* str, unsigned char color) {
    int saved_x = cursor_x;
    int saved_y = cursor_y;

    for (int i = 0; str[i] != '\0'; i++) {
        char c = str[i];

        if (c == '\n') {
            cursor_x = 0;
            cursor_y++;
        } else {
            print_char_color(c, cursor_x, cursor_y, color);
            cursor_x++;

            if (cursor_x >= SCREEN_WIDTH) {
                cursor_x = 0;
                cursor_y++;
            }
        }

        // NO AUTO-SCROLLING in this version
        if (cursor_y >= SCREEN_HEIGHT) {
            cursor_y = SCREEN_HEIGHT - 1;
            break;
        }
    }

    // Restore cursor
    cursor_x = saved_x;
    cursor_y = saved_y;
    set_cursor(cursor_x, cursor_y);
}

void kprint_at(const char* str, int x, int y, unsigned char color) {
    int saved_x = cursor_x;
    int saved_y = cursor_y;

    cursor_x = x;
    cursor_y = y;
    kprint_no_scroll(str, color);

    // Restore cursor
    cursor_x = saved_x;
    cursor_y = saved_y;
    set_cursor(cursor_x, cursor_y);
}

// In deinen String-Funktionen (bei strcmp, strlen, etc.):
char* xstrstr(const char* haystack, const char* needle) {
    if(!*needle) return (char*)haystack;

    for(; *haystack; haystack++) {
        const char* h = haystack;
        const char* n = needle;

        while(*h && *n && *h == *n) {
            h++;
            n++;
        }

        if(!*n) return (char*)haystack;  // Gefunden!
    }

    return NULL;  // Nicht gefunden
}

void scroll_screen(void) {
    unsigned short* buffer = (unsigned short*)VIDEO_MEMORY;

    // 1. Alle Zeilen eine Position nach oben verschieben (Zeile 1->0, 2->1, etc.)
    for (int y = 0; y < SCREEN_HEIGHT - 1; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            buffer[y * SCREEN_WIDTH + x] = buffer[(y + 1) * SCREEN_WIDTH + x];
        }
    }

    // 2. Letzte Zeile mit LEERZEICHEN + BLUE BACKGROUND + WHITE TEXT füllen
    // Dein Theme: Blue background (0x10 = 0x01 << 4), White text (0x0F)
    unsigned short clear_char = (COLOR_DARK_GRAY << 4) | COLOR_WHITE; // DARK_GRAY OR ELSE A VISUAL BUG *WILL* OCCUR
    clear_char = (clear_char << 8) | ' ';  // Farbe + Leerzeichen

    for (int x = 0; x < SCREEN_WIDTH; x++) {
        buffer[(SCREEN_HEIGHT - 1) * SCREEN_WIDTH + x] = clear_char;
    }

    // 3. Cursor anpassen falls nötig
    if (cursor_y >= SCREEN_HEIGHT) {
        cursor_y = SCREEN_HEIGHT - 1;
    }
}

void print_right(const char* str, int y, unsigned char color) {
    int length = 0;
    while (str[length] != '\0') {
        length++;
    }

    int start_x = SCREEN_WIDTH - length;

    for (int i = 0; i < length; i++) {
        print_char_color(str[i], start_x + i, y, color);
    }
}

// ========================
// CURSOR FUNCTIONS
// ========================

void set_cursor(int x, int y) {
    if (x < 0) x = 0;
    if (x >= SCREEN_WIDTH) x = SCREEN_WIDTH - 1;
    if (y < 0) y = 0;
    if (y >= SCREEN_HEIGHT) y = SCREEN_HEIGHT - 1;

    cursor_x = x;
    cursor_y = y;

    unsigned short position = (y * SCREEN_WIDTH) + x;

    // Cursor Low port
    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char)(position & 0xFF));

    // Cursor High port
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char)((position >> 8) & 0xFF));
}

void get_cursor(int* x, int* y) {
    *x = cursor_x;
    *y = cursor_y;
}

void clear_screen(unsigned char bg_color) {
    unsigned char color = (bg_color << 4) | COLOR_WHITE;

    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            print_char_color(' ', x, y, color);
        }
    }

    cursor_x = 0;
    cursor_y = 0;
    set_cursor(0, 0);
}

// ========================
// KERNEL MAIN FUNCTION WITH SAFE MEMORY
// ========================

void kernel_main(unsigned int magic, unsigned int addr) {
    debug_mode = 0;

    show_ascii_boot();

    // Dunkler Hintergrund für bessere Lesbarkeit
    clear_screen(CURRENT_BG);

    // Minimaler Header
    kprint_at("KonsKernel v1.4.0", 0, 1, COLOR_WHITE_ON_BLACK);
    kprint_at("────────────────────────────────", 0, 2, COLOR_CYAN_ON_BLACK);

    // Alles initialisieren (silent)
    read_multiboot_info(addr);
    init_simple_memory();
    init_simple_heap();
    gdt_install();
    kfs_init();
    pic_remap(0x20, 0x28);
    isr_install();
    irq_install();
    outb(0x43, 0x36);
    outb(0x40, 0xFF);
    outb(0x40, 0xFF);
    while (inb(0x64) & 0x02) { /* wait */ }
    outb(0x64, 0xAE);
    while (inb(0x64) & 0x01) { inb(0x60); }
    asm volatile("sti");

    // Kurze Statusmeldung
    unsigned int total_mb = total_memory / (1024 * 1024);
    kprint_at("System loaded. ", 0, 4, COLOR_GREEN_ON_BLACK);

    char mem_str[16];
    char* ptr = mem_str;
    unsigned int n = total_mb;
    if(n == 0) *ptr++ = '0';
    else {
        char temp[16];
        int i = 0;
        while(n > 0) { temp[i++] = '0' + (n % 10); n /= 10; }
        while(i > 0) *ptr++ = temp[--i];
    }
    *ptr++ = 'M';
    *ptr++ = 'B';
    *ptr++ = ' ';
    *ptr++ = 'R';
    *ptr++ = 'A';
    *ptr++ = 'M';
    *ptr = '\0';

    kprint(mem_str, COLOR_CYAN_ON_BLACK);

    // Prompt
    kprint_at("kons> ", 0, 6, COLOR_WHITE_ON_BLACK);

    cursor_x = 6;
    cursor_y = 6;
    set_cursor(cursor_x, cursor_y);

    // History
    for(int i = 0; i < CMD_HISTORY_SIZE; i++) {
        cmd_history[i][0] = '\0';
    }
    history_count = 0;
    history_index = -1;

    while(1) {
        asm volatile("hlt");
    }
}
