__attribute__((naked))
__attribute__((section(".start_section")))
void startup(void)
{
    __asm__ volatile(" LDR R0,=0x2001C000\n");
    __asm__ volatile(" MOV SP,R0\n");
    __asm__ volatile(" BL main\n");
    __asm__ volatile(" B .\n");
}

#define SIMULATOR

// definitions from slides
#define PORT_D 0x40020C00
#define GPIO_MODER ((volatile unsigned int *)(PORT_D))
#define GPIO_OTYPER ((volatile unsigned short *)(PORT_D + 0x4))
#define GPIO_SPEEDR ((volatile unsigned int *)(PORT_D + 0x8))
#define GPIO_PUPDR ((volatile unsigned int *)(PORT_D + 0xC))
#define GPIO_IDR_LOW ((volatile unsigned char *)(PORT_D + 0x10))
#define GPIO_IDR_HIGH ((volatile unsigned char *)(PORT_D + 0x11))
#define GPIO_ODR_LOW ((volatile unsigned char *)(PORT_D + 0x14))
#define GPIO_ODR_HIGH ((volatile unsigned char *)(PORT_D + 0x15))

// from slides
#define STK_CTRL ((volatile unsigned int *)(0xE000E010))
#define STK_LOAD ((volatile unsigned int *)(0xE000E014))
#define STK_VAL ((volatile unsigned int *)(0xE000E018))

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

void init_app(void)
{
   	// starta klockor port D och E 
   	// why tho
    *((unsigned long *) 0x40023830) = 0x18;

   	// port D output 
	*GPIO_MODER = 0x55555555;

   	// port D medium speed do we need this??
    *((volatile unsigned int *) 0x40020C08) = 0x55555555;
}

void main(void)
{
    init_app();
    while (1)
    {
       	// blinky 
       	*GPIO_ODR_LOW = 0;
        delay_milli(500);
        *GPIO_ODR_LOW = 0xFF;
        delay_milli(500);
    }
}
