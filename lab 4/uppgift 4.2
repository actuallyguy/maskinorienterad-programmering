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
#define NVIC_ISER0 0xE000E100

#define SYSCFG_EXTICR1 ((int*) 0x40013808)
#define NVIC_EXTI3_IRQ_BPOS (1<<9)
#define EXTI3_IRQ_BPOS (1<<3)



int count;

void irq_handler ( void )
{
    char input = 0xF & *GPIO_E_IDR_LOW;
    
    if(   *EXTI_PR & EXTI3_IRQ_BPOS )
    {
        // clear interrupt
        *EXTI_PR |= EXTI3_IRQ_BPOS; 
        
        if( input & 4 )
        { 
            if(count)
                count = 0;
            else
                count = 0xFF;
            // reset
            *GPIO_E_ODR_LOW = 0x40;
        }  
        
        if ( input & 2 )
        {
            *GPIO_E_ODR_LOW = 0x20;
            count = 0;
        }
        
        if ( input & 1 )
        { 
            *GPIO_E_ODR_LOW = 0x10;
            count++;
        }
        
        // reset all flipflops
        *GPIO_E_ODR_LOW |= 0x70; 
        *GPIO_E_ODR_LOW &= ~0x70;   
    }
}


void app_init ( void )
{
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

    count = 1;
    
    // toggle irq
    *GPIO_E_ODR_LOW = 0x70; 
    *GPIO_E_ODR_LOW = ~0x70;

    *SYSCFG_EXTICR1 |= 0x4000; 

    // interrupt mask
    *EXTI_IMR |= EXTI3_IRQ_BPOS; 
     
    // enable rising edge
    *EXTI_RTSR |= EXTI3_IRQ_BPOS;
    // disable falling edge
    *EXTI_FTSR |= EXTI3_IRQ_BPOS;

    // interrupt vector
    *((void (**)(void) ) EXTI3_IRQVEC ) = irq_handler;
   
    // enable in nvic
    *((unsigned int *) NVIC_ISER0) |= NVIC_EXTI3_IRQ_BPOS;  
 }

void main(void)
{
    app_init();
    while(1){
        *GPIO_ODR_LOW = count;
    }
 }
