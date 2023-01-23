__attribute__((naked))
__attribute__((section(".start_section")))
void startup(void)
{
    __asm__ volatile(" LDR R0,=0x2001C000\n");
    __asm__ volatile(" MOV SP,R0\n");
    __asm__ volatile(" BL main\n");
    __asm__ volatile(" B .\n");
}

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

void init_app(void)
{
   	// copied from lab page, is it even necessary
    *((unsigned long *) 0x40023830) = 0x18;

   	// copied from lab page, medium speed???
    *((volatile unsigned int *) 0x40020C08) = 0x55555555;

*GPIO_MODER = 0x55005555;
*GPIO_OTYPER = 0x00000000;
*GPIO_PUPDR = 0x00AA0000;

*GPIO_ODR_LOW = 0;
*GPIO_ODR_HIGH = 0;
}

unsigned char segCode[] = { 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07,
    0x7F, 0x67, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71
};

void out7seg(unsigned char c)
{
    if (c > 15)
    {
        *GPIO_ODR_LOW = 0;
        return;
    }

    *GPIO_ODR_LOW = segCode[c];
}

// from slides
void kbdActivate(unsigned int row)
{
    switch (row)
    {
        case 1:
            *GPIO_ODR_HIGH = 0x10;
            break;
        case 2:
            *GPIO_ODR_HIGH = 0x20;
            break;
        case 3:
            *GPIO_ODR_HIGH = 0x40;
            break;
        case 4:
            *GPIO_ODR_HIGH = 0x80;
            break;
        case 0:
            *GPIO_ODR_HIGH = 0x00;
            break;
    }
}

// from slides
int kbdGetCol(void)
{
    unsigned int c;
    c = *GPIO_IDR_HIGH;
    if (c & 0x8) return 4;
    if (c & 0x4) return 3;
    if (c & 0x2) return 2;
    if (c & 0x1) return 1;
    return 0;
}

unsigned char keyb(void)
{
    unsigned char key[] = { 1, 2, 3, 0xA, 4, 5, 6, 0xB, 7, 8, 9, 0xC, 0xE, 0, 0xF, 0xD
    };

    int row, col;
    for (row = 1; row <= 4; row++)
    {
        kbdActivate(row);
        if ((col = kbdGetCol()))
        {
            return key[4 *(row - 1) + (col - 1)];
        }
    }

    kbdActivate(0);
    return 0xFF;
}

void main(void)
{
    init_app();
    while (1)
    {
        char key = keyb();
        out7seg(key);
    }
}
