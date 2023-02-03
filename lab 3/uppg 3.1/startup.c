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

// from the book
__attribute__((naked))
void graphic_initalize(void)
{
    __asm volatile(" .HWORD 0xDFF0\n");
    __asm volatile(" BX LR\n");
}

__attribute__((naked))
void graphic_clear_screen(void)
{
    __asm volatile(" .HWORD 0xDFF1\n");
    __asm volatile(" BX LR\n");
}

__attribute__((naked))
void graphic_pixel_set(int x, int y)
{
    __asm volatile(" .HWORD 0xDFF2\n");
    __asm volatile(" BX LR\n");
}

__attribute__((naked))
void graphic_pixel_clear(int x, int y)
{
    __asm volatile(" .HWORD	0xDFF3\n");
    __asm volatile(" BX LR\n");
}

void init_app(void)
{
   	// not sure if we need this?
    /*starta klockor port D och E */
    *((unsigned long *) 0x40023830) = 0x18;
}

typedef struct
{
    char x, y;
}

POINT, *PPOINT;

typedef struct
{
    POINT p0;
    POINT p1;
}

LINE, *PLINE;

int abs(int i)
{
    return i < 0 ? -i : i;
}

void swap(int *a, int *b)
{
    int t = *a;
    *a = *b;
    *b = t;
}

int draw(PLINE l)
{
    int x0, x1, y0, y1;
    int steep;

    x0 = l->p0.x;
    x1 = l->p1.x;
    y0 = l->p0.y;
    y1 = l->p1.y;

    if (abs(y1 - y0) > abs(x1 - x0))
    {
        steep = 1;
    }
    else
    {
        steep = 0;
    }

    if (steep)
    {
        swap(&x0, &y0);
        swap(&x1, &y1);
    }

    if (x0 > x1)
    {
        swap(&x0, &x1);
        swap(&y0, &y1);
    }

   	// omg found the bug these need to be ints!?!?
    int deltax = x1 - x0;
    int deltay = abs(y1 - y0);
    int error = 0;
    int y = y0;

    int ystep;

    if (y0 < y1)
    {
        ystep = 1;
    }
    else
    {
        ystep = -1;
    }

    for (int x = x0; x <= x1; x++)
    {
        if (steep)
        {
            graphic_pixel_set(y, x);
        }
        else
        {
            graphic_pixel_set(x, y);
        }

        error = error + deltay;
        if ( 2 * error >= deltax)
        {
            y += ystep;
            error -= deltax;
        }
    }
}

typedef struct poly_tag
{
    POINT p;
    struct poly_tag * next;
}

POLY, *PPOLY;

int draw_polygon(POLY *poly)
{
    LINE line;
    line.p0.x = poly->p.x;
    line.p0.y = poly->p.y;
    poly = poly->next;
    while (poly != 0)
    {
        line.p1.x = poly->p.x;
        line.p1.y = poly->p.y;
        draw(&line);
        line.p0.x = poly->p.x;
        line.p0.y = poly->p.y;
        poly = poly->next;
    }

    return 1;
}

POLY pg8 = { 20,
    20,
    0
};

POLY pg7 = { 20,
    55, &pg8
};

POLY pg6 = { 70,
    60, &pg7
};

POLY pg5 = { 80,
    35, &pg6
};

POLY pg4 = { 100,
    25, &pg5
};

POLY pg3 = { 90,
    10, &pg4
};

POLY pg2 = { 40,
    10, &pg3
};

POLY pg1 = { 20,
    20, &pg2
};

void main(void)
{
    init_app();
    graphic_initalize();
    graphic_clear_screen();

    while (1)
    {
        draw_polygon(&pg1);
        delay_milli(500);
        graphic_clear_screen();
        delay_milli(500);
    }
}
