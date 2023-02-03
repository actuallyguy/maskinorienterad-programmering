__attribute__((naked)) __attribute__((section(".start_section")))
void startup(void)
{
    __asm__ volatile(" LDR R0,=0x2001C000\n"); /*set stack */
    __asm__ volatile(" MOV SP,R0\n");
    __asm__ volatile(" BL main\n"); /*call main */
    __asm__ volatile(".L1: B .L1\n"); /*never return */
}
#define SIMULATOR
#define GPIO_E 0x40021000
#define GPIO_E_MODER ((volatile unsigned int *)(GPIO_E))
#define GPIO_E_OTYPER ((volatile unsigned short *)(GPIO_E + 0x4))
#define GPIO_E_OSPEEDR ((volatile unsigned int *)(GPIO_E + 0x8))
#define GPIO_E_PUPDR ((volatile unsigned int *)(GPIO_E + 0xC))
#define GPIO_E_IDRHIGH ((volatile unsigned char *)(GPIO_E + 0x11))
#define GPIO_E_ODRLOW ((volatile unsigned char *)(GPIO_E + 0x14))
#define GPIO_E_ODRHIGH ((volatile unsigned char *)(GPIO_E + 0x15))

// from slides
#define STK_CTRL ((volatile unsigned int *)(0xE000E010))
#define STK_LOAD ((volatile unsigned int *)(0xE000E014))
#define STK_VAL ((volatile unsigned int *)(0xE000E018))

// ascii display bits
#define B_E 0x40
#define B_SELECT 4
#define B_RW 2
#define B_RS 1

void init_app(void)
{
   	// copied from lab page, is it even necessary
    *((unsigned long *) 0x40023830) = 0x18;

   	// copied from lab page, medium speed???
    *((volatile unsigned int *) 0x40020C08) = 0x55555555;
    *GPIO_E_MODER = 0x55555555;	// port E output 
	*GPIO_E_PUPDR = 0x55550000;	// input bits pull-up
}

// from slides
void delay_250ns(void)
{
    /*SystemCoreClock = 168000000 */
    *STK_CTRL = 0;
    *STK_LOAD = ((168 / 4) - 1);
    *STK_VAL = 0;
    *STK_CTRL = 5;
    while ((*STK_CTRL &0x10000) == 0);
    *STK_CTRL = 0;
}

// from slides
void delay_micro(unsigned int us)
{
    while (us > 0)
    {
        delay_250ns();
        delay_250ns();
        delay_250ns();
        delay_250ns();
        us--;
    }
}

// from slides
void delay_milli(unsigned int ms)
{
    #ifdef SIMULATOR
    ms = ms / 1000;
    ms++;
    #endif
    while (ms > 0)
    {
        delay_micro(1000);
        ms--;
    }
}

// from slides
void ascii_ctrl_bit_set(char x)
{ /*x: bitmask bits are 1 to set */
    char c;
    c = *GPIO_E_ODRLOW;
    *GPIO_E_ODRLOW = B_SELECT | x | c;
}

void ascii_ctrl_bit_clear(char x)
{ /*x: bitmask bits are 1 to clear */
    char c;
    c = *GPIO_E_ODRLOW;
    c = c &~x;
    *GPIO_E_ODRLOW = B_SELECT | c;
}

// from slides
void ascii_write_controller(char c)
{
    ascii_ctrl_bit_set(B_E);
    *GPIO_E_ODRHIGH = c;
    ascii_ctrl_bit_clear(B_E);
    delay_250ns();
}

void ascii_write_cmd(char c)
{
    ascii_ctrl_bit_clear(B_RS);
    ascii_ctrl_bit_clear(B_RW);
    ascii_write_controller(c);
}

void ascii_write_data(char c)
{
    ascii_ctrl_bit_clear(B_RW);
    ascii_ctrl_bit_set(B_RS);
    ascii_write_controller(c);
}

// from slides
char ascii_read_controller(void)
{
    char c;
    ascii_ctrl_bit_set(B_E);
    delay_250ns();
    delay_250ns();
    c = *GPIO_E_IDRHIGH;
    ascii_ctrl_bit_clear(B_E);
    return c;
}

// from slides
char ascii_read_status(void)
{
    char c;
    *GPIO_E_MODER = 0x00005555;
    ascii_ctrl_bit_set(B_RW);
    ascii_ctrl_bit_clear(B_RS);
    c = ascii_read_controller();
    *GPIO_E_MODER = 0x55555555;
    return c;
}

void ascii_write_char(unsigned char c)
{
    while ((ascii_read_status() &0x80) == 0x80) {}
    delay_micro(8); /*latenstid för kommando */
	
    ascii_write_data(c);
    delay_micro(39);
}

void ascii_init()
{
   	// function
    while ((ascii_read_status() &0x80) == 0x80) {}
    delay_micro(8); /*latenstid för kommando */
	
    ascii_write_cmd(0x38);
    delay_micro(39);
    while ((ascii_read_status() &0x80) == 0x80) {}
    delay_micro(8); /*latenstid för kommando */

   	// activate display
    ascii_write_cmd(0x0C);
    delay_micro(39);
    while ((ascii_read_status() &0x80) == 0x80) {}

    delay_micro(8); /*latenstid för kommando */

   	// from slides
    /*vänta tills display är klar att ta emot kommando */
    while ((ascii_read_status() &0x80) == 0x80) {}

    delay_micro(8); /*latenstid för kommando */
    ascii_write_cmd(1); /*kommando: "Clear display" */
    delay_milli(2); /*i stället för 1,53 ms */

   	// entry mode
    ascii_write_cmd(6);
    delay_micro(39);
}

void ascii_gotoxy(char x, char y)
{
    char address;

    if (y != 1)
    {
       	// why tho??
        address = 0x40 | (x - 1);
    }
    else
    {
        address = x - 1;
    }

    ascii_write_cmd(0x80 | address);
}

int main(void)
{
    char *s;
    char test1[] = "Alfanumerisk";
    char test2[] = "Display - test";

    init_app();
    ascii_init();
    ascii_gotoxy(1, 1);
    s = test1;
    while (*s)
        ascii_write_char(*s++);
    ascii_gotoxy(1, 2);
    s = test2;
    while (*s)
        ascii_write_char(*s++);
    return 0;
}
