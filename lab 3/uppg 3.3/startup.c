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
    return;
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

typedef struct
{
    char x;
    char y;
}

POINT;

#define MAX_POINTS 40

typedef struct
{
    int numpoints;
    int sizex;
    int sizey;
    POINT px[MAX_POINTS];
}

GEOMETRY, *PGEOMETRY;

// from book
GEOMETRY ball_geometry = { 12,
    4, 4,
    {
        { 0, 1 },
        { 0, 2 },
        { 1, 0 },
        { 1, 1 },
        { 1, 2 },
        { 1, 3 },
        { 2, 0 },
        { 2, 1 },
        { 2, 2 },
        { 2, 3 },
        { 3, 1 },
        { 3, 2 }
    }
};

// from book
typedef struct tObj
{
    PGEOMETRY geo;
    int dirx, diry;
    unsigned int posx, posy;
    void(*draw)(struct tObj *);
    void(*clear)(struct tObj *);
    void(*move)(struct tObj *);
    void(*set_speed)(struct tObj *, int, int);
}

OBJECT, *POBJECT;

static void draw_object(POBJECT o)
{
    int i;
    for (i = 0; i < o->geo->numpoints; i++)
    {
        graphic_pixel_set(o->geo->px[i].x + o->posx, o->geo->px[i].y + o->posy);
    }
}

static void clear_object(POBJECT o)
{
    int i;
    for (i = 0; i < o->geo->numpoints; i++)
    {
        graphic_pixel_clear(o->geo->px[i].x + o->posx, o->geo->px[i].y + o->posy);
    }
}

void set_ballobject_speed(POBJECT o, int speedx, int speedy)
{
    o->dirx = speedx;
    o->diry = speedy;
}

void move_ballobject(OBJECT *o)
{
    int new_posx, new_posy;

   	// clear object
    o->clear(o);

   	// calculate new position
    new_posx = o->posx + o->dirx;
    new_posy = o->posy + o->diry;

   	// check if ball should bounce
    if (new_posy + o->geo->sizey > 64)
    {
        o->diry = -o->diry;
        unsigned int height = o->geo->sizey;
       	// need to do it this way, strange bug otherwise!?!?
        new_posy = 64;
        new_posy = new_posy - height;
    }

   	// roof
    else if (new_posy < 1)
    {
        o->diry = -o->diry;
        new_posy = 1;
    }

   	// right wall
    else if (new_posx + o->geo->sizex > 128)
    {
        new_posx = 124;
        o->dirx = -o->dirx;
    }

   	// left wall
    else if (new_posx < 1)
    {
        o->dirx = -o->dirx;
        new_posx = 1;
    }

   	// update object position
    o->posx = new_posx;
    o->posy = new_posy;
    o->draw(o);
}

int between_points(POINT test, POINT upper_left, POINT bottom_right)
{
    if (test.x <= bottom_right.x && test.x >= upper_left.x)
    {
        if (test.y <= bottom_right.y && test.y >= upper_left.y)
        {
            return 1;
        }
    }

    return 0;
}

int objects_contact(POBJECT o1, POBJECT o2)
{
    POINT o1_upper_left = { o1->posx, o1->posy
    };

    POINT o1_bottom_right = { o1->posx + o1->geo->sizex, o1->posy + o1->geo->sizey
    };

    POINT o2_upper_left = { o2->posx, o2->posy
    };

    POINT o2_upper_right = { o2->posx + o2->geo->sizex, o2->posy
    };

    POINT o2_bottom_left = { o2->posx, o2->posy + o2->geo->sizey
    };

    POINT o2_bottom_right = { o2->posx + o2->geo->sizex, o2->posy + o2->geo->sizey
    };

    if (between_points(o2_upper_left, o1_upper_left, o1_bottom_right))
    {
        return 1;
    }

    if (between_points(o2_upper_right, o1_upper_left, o1_bottom_right))
    {
        return 1;
    }

    if (between_points(o2_bottom_left, o1_upper_left, o1_bottom_right))
    {
        return 1;
    }

    if (between_points(o2_bottom_right, o1_upper_left, o1_bottom_right))
    {
        return 1;
    }

    return 0;
}

int pixel_overlap(POBJECT o1, POBJECT o2)
{
    for (int i = 0; i < o1->geo->numpoints; i++)
    {
        for (int j = 0; j < o2->geo->numpoints; j++)
            if ((o1->posx + o1->geo->px[i].x == o2->posx + o2->geo->px[j].x) &&
                (o1->posy + o1->geo->px[i].y == o2->posy + o2->geo->px[j].y))
                return 1;
    }

    return 0;
}

GEOMETRY spider_g = { 22,
    6, 8,
    {
        { 0, 2 },
        { 0, 3 },
        { 0, 7 },
        { 1, 1 },
        { 1, 2 },
        { 1, 4 },
        { 1, 6 },
        { 2, 0 },
        { 2, 2 },
        { 2, 3 },
        { 2, 5 },
        { 3, 0 },
        { 3, 2 },
        { 3, 3 },
        { 3, 5 },
        { 4, 1 },
        { 4, 2 },
        { 4, 4 },
        { 4, 6 },
        { 5, 2 },
        { 5, 3 },
        { 3, 7 }
    }
};

static OBJECT spider = { &spider_g,
    0, 0,
    50, 50,
    draw_object,
    clear_object,
    move_ballobject,
    set_ballobject_speed
};

void init_app(void)
{
    /*starta klockor port D och E */
    *((unsigned long *) 0x40023830) = 0x18;

    *GPIO_MODER = 0x55005555;
   	// pull down inputs 
    *GPIO_PUPDR = 0x00AA0000;
}

static OBJECT ball_object = { &ball_geometry,
    2, 1,
    1, 1,
    draw_object,
    clear_object,
    move_ballobject,
    set_ballobject_speed
};

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

int main()
{
    char c;
    POBJECT victim = &ball_object;
    POBJECT creature = &spider;
    init_app();
    graphic_initalize();
    graphic_clear_screen();
    victim->set_speed(victim, 4, 1);

    while (1)
    {
        victim->move(victim);
        creature->move(creature);
        c = keyb();
        switch (c)
        {
            case 6:
                creature->set_speed(creature, 2, 0);
                break;
            case 4:
                creature->set_speed(creature, -2, 0);
                break;
            case 5:
                creature->set_speed(creature, 0, 0);
                break;
            case 2:
                creature->set_speed(creature, 0, -2);
                break;
            case 8:
                creature->set_speed(creature, 0, 2);
                break;
            default:
                creature->set_speed(creature, 0, 0);
                break;
        }

        if (objects_contact(victim, creature))
        {
           	// Game over
            break;
        }

        delay_milli(40);
    }
}
