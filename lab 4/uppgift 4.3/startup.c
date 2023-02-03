__attribute__((naked)) __attribute__((section(".start_section")))
void startup(void)
{
    __asm__ volatile(" LDR R0,=0x2001C000\n"); /*set stack */
    __asm__ volatile(" MOV SP,R0\n");
    __asm__ volatile(" BL main\n"); /*call main */
    __asm__ volatile(".L1: B .L1\n"); /*never return */
}

// definitions from slides
#define PORT_E 0x40021000
#define GPIO_E_MODER ((volatile unsigned int *)(PORT_E))
#define GPIO_E_OTYPER ((volatile unsigned short *)(PORT_E + 0x4))
#define GPIO_E_SPEEDR ((volatile unsigned int *)(PORT_E + 0x8))
#define GPIO_E_PUPDR ((volatile unsigned int *)(PORT_E + 0xC))
#define GPIO_E_IDR_LOW ((volatile unsigned char *)(PORT_E + 0x10))
#define GPIO_E_IDR_HIGH ((volatile unsigned char *)(PORT_E + 0x11))
#define GPIO_E_ODR_LOW ((volatile unsigned char *)(PORT_E + 0x14))
#define GPIO_E_ODR_HIGH ((volatile unsigned char *)(PORT_E + 0x15))
    

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

#define EXTI_IMR ((unsigned int *) 0x40013C00)
#define EXTI_FTSR ((unsigned int *) 0x40013C0C)
#define EXTI_RTSR ((unsigned int *) 0x40013C08)
#define EXTI_PR ((unsigned int *)  0x40013C14)

#define EXTI3_IRQVEC 0x2001C064
#define EXTI2_IRQVEC 0x2001C060
#define EXTI1_IRQVEC 0x2001C05C
#define EXTI0_IRQVEC 0x2001C058
#define NVIC_ISER0 0xE000E100

#define SYSCFG_EXTICR1 ((int*) 0x40013808)

#define NVIC_EXTI3_IRQ_BPOS (1<<9)
#define NVIC_EXTI2_IRQ_BPOS (1<<8)
#define NVIC_EXTI1_IRQ_BPOS (1<<7)
#define NVIC_EXTI0_IRQ_BPOS (1<<6)

#define EXTI3_IRQ_BPOS (1<<3)
#define EXTI2_IRQ_BPOS (1<<2)
#define EXTI1_IRQ_BPOS (1<<1)
#define EXTI0_IRQ_BPOS (1<<0)

int count = 0;

void irq_handler_exti2 ( void )
{
	*GPIO_E_ODR_LOW |= 0x40;
	*GPIO_E_ODR_LOW &= ~0x40;
	if(count != 0)
		count = 0;
	else
		count = 0xFF;

    *EXTI_PR |= EXTI2_IRQ_BPOS;
}

void irq_handler_exti1 ( void )
{
	count = 0;
	*GPIO_E_ODR_LOW |= 0x20;
	*GPIO_E_ODR_LOW &= ~0x20;
    *EXTI_PR |= EXTI1_IRQ_BPOS;
}
void irq_handler_exti0 ( void )
{
	count++;
	*GPIO_E_ODR_LOW |= 0x10;
	*GPIO_E_ODR_LOW &= ~0x10;
    *EXTI_PR |= EXTI0_IRQ_BPOS;
}


void app_init ( void )
{
    // from lab page
    /* starta klockor port D och E */
    * ( (unsigned long *) 0x40023830) = 0x18;
    /* starta klockor för SYSCFG */
    * ((unsigned long *)0x40023844) |= 0x4000;  
    /* Relokera vektortabellen */
    * ((unsigned long *)0xE000ED08) = 0x2001C000;

    // port e ouput bit 4-7
    *GPIO_E_MODER = 0x00005500;
    
    // port d output bit 0-7
    *GPIO_MODER = 0x00005555;
    
    // enable irqs
    *SYSCFG_EXTICR1 |= 0x0444;
    
    *EXTI_IMR |= (EXTI2_IRQ_BPOS |  EXTI1_IRQ_BPOS | EXTI0_IRQ_BPOS); 

    // enable rising edge 
    *EXTI_RTSR |= (EXTI2_IRQ_BPOS |  EXTI1_IRQ_BPOS | EXTI0_IRQ_BPOS);
    // disable falling edge
	*EXTI_FTSR &= ~(EXTI2_IRQ_BPOS |  EXTI1_IRQ_BPOS | EXTI0_IRQ_BPOS);

    *((void (**)(void) )EXTI2_IRQVEC) = irq_handler_exti2;
	*((void (**)(void) )EXTI1_IRQVEC) = irq_handler_exti1;
	*((void (**)(void) )EXTI0_IRQVEC) = irq_handler_exti0;

    // enable in nvic
    *((unsigned int *) NVIC_ISER0) |= (NVIC_EXTI2_IRQ_BPOS | NVIC_EXTI1_IRQ_BPOS | NVIC_EXTI0_IRQ_BPOS);
 
 }

void main(void)
{
    app_init();
    while(1){
        *GPIO_ODR_LOW = count;
    }
}
