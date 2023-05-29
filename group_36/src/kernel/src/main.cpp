// define entry point
extern "C"{
    void kernel_main();
}


#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "../cpu/i386/gdt.h"
#include "../cpu/i386/idt.h"
// #include "../cpu/i386/isr.h"
#include <stdlib/c/libc.h>
#include "../drivers/_include/driver.h"
#include <../cpu/include/cpu.h>
#include <../cpu/i386/timer/timer.h>
#include "../memory/paging.h"
#include "boot.h"
#include "print.h"
// #include "../memory/kmalloc.h"
extern uint32_t end;


 
#if !defined(__i386__)
#error "This needs to be compiled with a ix86-elf compiler"
#endif

/* Hardware text mode color constants. */
enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};


 
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) 
{
	return fg | bg << 4;
}
 
static inline uint16_t vga_entry(unsigned char uc, uint8_t color) 
{
	return (uint16_t) uc | (uint16_t) color << 8;
}
 
size_t strlen(const char* str) 
{
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}
 
static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
 
size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;
 
void terminal_initialize(void) 
{
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_RED, VGA_COLOR_BLACK);
	terminal_buffer = (uint16_t*) 0xB8000;
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}
 
void terminal_setcolor(uint8_t color) 
{
	terminal_color = color;
}
 
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) 
{
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}
 
void terminal_putchar(char c) 
{
	if (c == '\n') {
		terminal_column = 0;
		if (++terminal_row == VGA_HEIGHT) {
			terminal_row = 0;
		}
	} else {
	
		terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
		if (++terminal_column == VGA_WIDTH) {
			terminal_column = 0;
			if (++terminal_row == VGA_HEIGHT) {
				terminal_row = 0;
			}
		}
	}
}
 
void terminal_write(const char* data, size_t size) 
{
	for (size_t i = 0; i < size; i++)
		terminal_putchar(data[i]);
}
 
void printf(const char* data) 
{
	terminal_write(data, strlen(data));
}

void print_uint8(uint8_t scancode) {
    char buffer[12];  // Buffer for up to 10 digits plus null terminator
    uint8_t n = scancode;
    int i = 0;

    // Handle 0 explicitly (as the loop below doesn't do that)
    if (n == 0) {
        buffer[i++] = '0';
    }
    else {
        // Convert number to string (in reverse order)
        while (n > 0) {
            buffer[i++] = '0' + n % 10;
            n /= 10;
        }

        // Reverse the string to get it in the correct order
        for (int j = 0; j < i / 2; ++j) {
            char temp = buffer[j];
            buffer[j] = buffer[i - j - 1];
            buffer[i - j - 1] = temp;
        }
    }

    buffer[i] = '\0';  // Null-terminate the string

    printf(buffer);
}

void print_int(int n) {
    char buffer[12];  // Buffer for up to 10 digits plus null terminator
    int i = 0;

    // Handle 0 explicitly (as the loop below doesn't do that)
    if (n == 0) {
        buffer[i++] = '0';
    }
    else {
        // Convert number to string (in reverse order)
        while (n > 0) {
            buffer[i++] = '0' + n % 10;
            n /= 10;
        }

        // Reverse the string to get it in the correct order
        for (int j = 0; j < i / 2; ++j) {
            char temp = buffer[j];
            buffer[j] = buffer[i - j - 1];
            buffer[i - j - 1] = temp;
        }
    }

    buffer[i] = '\0';  // Null-terminate the string

    printf(buffer);
}

char hex_char(uint32_t val) {
    val &= 0xF; // keep only lowest 4 bits
    if (val <= 9) {
        return '0' + val;
    } else {
        return 'A' + (val - 10);
    }
}

void print_hex(uint32_t d)
{
    printf("0x");
    for(int i = 28; i >= 0; i-=4)
    {
        terminal_putchar(hex_char(d>>i));
    }

}




class OperatingSystem {
    int tick = 0;
    int seconds = 0;

public:
    OperatingSystem(vga_color color) {


    }

    void init() {

        // printf("Initializing Operating System....\n");
    }

    void debug_print(char *str) {
        printf(str);
    }

    void interrupt_handler_1(registers_t regs) {
        printf("Called Interrupt Handler 1!\n");
    }

    void interrupt_handler_3(registers_t regs) {
        printf("Called Interrupt Handler 3!\n");
    }

    void interrupt_handler_4(registers_t regs) {
        printf("Called Interrupt Handler 4!\n");
    }

    void timer() {
        tick++;
        if (tick % 100 == 0) {
            seconds++;
            // printf("(Every Second) Tick: ");
            // print_uint8(tick);
            // printf("\n");
        }
    }

    int get_tick(){
        return tick;
    }

    void print_time() {
        printf("[Time: ");
        print_uint8(seconds);
        printf("]:  $");
    }

    void sleep_busy(int seconds) {
        int start_tick = get_tick();
        int ticks_to_wait = seconds*1193180;
        int elapsed_ticks = 0;
        while(elapsed_ticks < ticks_to_wait){
            if (get_tick() != start_tick + elapsed_ticks){
                break;
            }
            else{
                elapsed_ticks += (get_tick()-start_tick)-elapsed_ticks;
            }
        }
    }

    void sleep_interrupt(int seconds) {
        int current_tick = get_tick();
        int ticks_to_wait = seconds*1193180;
        int end_ticks = current_tick + ticks_to_wait;
        while(current_tick < end_ticks){
            asm volatile("sti");
            asm volatile("hlt");
            current_tick = get_tick();
        }
    }
};

void* operator new[](size_t size) {
    return reinterpret_cast<void*>(kmalloc(size));
}

void kernel_main(void) 
{

	/* Initialize terminal interface */
	terminal_initialize();

    printf("Starting OS...\n");

    // initialize paging
    printf("Initializing paging...\n");
    init_paging();

 
	// Initialize GDT
	printf("\nInitializing GDT...\n");
    init_gdt();
    
    // initialize IDT
    printf("Initializing IDT...\n");
    init_idt();



    // initialize ISRs ##########################################
    printf("Initializing ISRs...");
    init_isr();

	// Create operating system object
    auto os = OperatingSystem(VGA_COLOR_RED);
    os.init();
    
    // Create interrupt handler for ISR1
    register_interrupt_handler(1,[](registers_t* regs, void* context){
        auto* os = (OperatingSystem*)context;
        os->interrupt_handler_1(*regs);
    }, (void*)&os);

    // Create interrupt handler for ISR3
    register_interrupt_handler(3,[](registers_t* regs, void* context){
        auto* os = (OperatingSystem*)context;
        os->interrupt_handler_3(*regs);
    }, (void*)&os);
    
    // Create interrupt handler for ISR4
    register_interrupt_handler(4,[](registers_t* regs, void* context){
        auto* os = (OperatingSystem*)context;
        os->interrupt_handler_4(*regs);
    }, (void*)&os);

    // Create a timer on IRQ0 - System Timer
    PIT::init_timer(1, [](registers_t*regs, void* context){
        auto* os = (OperatingSystem*)context;
        os->timer();
    }, &os);
	printf("Initialized!\n");

	printf("Hello World!! \n");
    
    // Fire interrupts! Should trigger callback above
    asm volatile ("int $0x1");
    asm volatile ("int $0x3");
    asm volatile ("int $0x4");

    // Enable interrutps
    asm volatile("sti");


    // int memory_1 = kmalloc(12345);
    // char* memory4 = new char[2]();


    int phys = kmalloc_ap(54321, 0x000);
    print_int(phys);
    printf("\nmemory address^\n");

    os.print_time();

    // Hook Keyboard
   Keyboard::hook_keyboard([](uint8_t scancode, void* context){
        auto* os = (OperatingSystem*)context;
        // for assignment:
        printf("Keyboard interrupt triggered: ");

        char ascii[2] = {Keyboard::scancode_to_ascii(scancode), '\0'};
        printf(ascii);
        printf("\n");

        if(ascii[0] == 10){
            os->print_time();
        } // if newline
        
        // printf(" (");
        // print_uint8(scancode);
        // printf(")\n");
    }, &os);

    int counter = 0;
    while(true){
        printf("Sleeping with busy-waiting (HIGH CPU):");
        print_int(counter);
        printf("\n");
        os.sleep_busy(1000);
        printf("Slept using busy-waiting:");
        print_int(counter++);
        printf("\n");
        printf("Sleeping with interrupts (LOW CPU):");
        print_int(counter);
        printf("\n");
        os.sleep_interrupt(1000);
        printf("Slept using interrupts:");
        print_int(counter++);
        printf("\n");
    };

    // while(1){}
}