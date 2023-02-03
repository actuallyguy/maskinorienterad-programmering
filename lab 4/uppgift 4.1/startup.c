__attribute__((naked)) __attribute__((section(".start_section")))
void startup(void)
{
	__asm__ volatile(" LDR R0,=0x2001C000\n"); /*set stack */
	__asm__ volatile(" MOV SP,R0\n");
	__asm__ volatile(" BL main\n"); /*call main */
	__asm__ volatile(".L1: B .L1\n"); /*never return */
}
#define SIMULATOR

// from slides
#define STK_CTRL ((volatile unsigned int *)(0xE000E010))
#define STK_LOAD ((volatile unsigned int *)(0xE000E014))
#define STK_VAL ((volatile unsigned int *)(0xE000E018))

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

volatile int systick_flag;
volatile int delay_count;

void delay_1mikro(void)
{
	*STK_CTRL = 0;
	*STK_LOAD = (168 - 1);
	*STK_VAL = 0;
	*STK_CTRL = 7;
}

void systick_irq_handler(void)
{
	// deactivate counter *STK_CTRL = 0;
	delay_count--;
	if (delay_count > 0)
	{
		delay_1mikro();
	}
	else
	{
		systick_flag = 1;
	}
}

void init_app(void)
{
	
	// copy paste lab page
	/* starta klockor port D och E */
	* ( (unsigned long *) 0x40023830) = 0x18;
	/* starta klockor för SYSCFG */
	* ((unsigned long *)0x40023844) |= 0x4000; 	
	/* Relokera vektortabellen */
	* ((unsigned long *)0xE000ED08) = 0x2001C000;
	
	// port d output 
    *GPIO_MODER = 0x55555555;
	*GPIO_OTYPER = 0x00000000;
	*GPIO_SPEEDR = 0x55555555;

	*((void(**)(void)) 0x2001C03C) = systick_irq_handler;
}

#ifdef SIMULATOR
#define DELAY_COUNT 1000
#else
#define DELAY_COUNT 100000
#endif

void delay(unsigned int count)
{
	delay_count = count;
	delay_1mikro();
}

void main()
{
	init_app();
	*GPIO_ODR_LOW = 0;
	delay(DELAY_COUNT);
	*GPIO_ODR_LOW = 0xFF;
	while (1)
	{
		if (systick_flag)
			break;
		// do something here while waiting idk?
	}

	*GPIO_ODR_LOW = 0;
}
