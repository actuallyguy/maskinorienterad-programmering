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

// definitions from slides
#define GPIO_E 0x40021000
#define GPIO_E_MODER ((volatile unsigned int *)(GPIO_E))
#define GPIO_E_OTYPER ((volatile unsigned short *)(GPIO_E + 0x4))
#define GPIO_E_OSPEEDR ((volatile unsigned int *)(GPIO_E + 0x8))
#define GPIO_E_PUPDR ((volatile unsigned int *)(GPIO_E + 0xC))
#define GPIO_E_IDRHIGH ((volatile unsigned char *)(GPIO_E + 0x11))
#define GPIO_E_ODRLOW ((volatile unsigned char *)(GPIO_E + 0x14))
#define GPIO_E_ODRHIGH ((volatile unsigned char *)(GPIO_E + 0x15))

// ascii display bits
#define B_E 0x40
#define B_SELECT 4
#define B_RW 2
#define B_RS 1

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
    __asm volatile(" .HWORD 0xDFF3\n");
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

GEOMETRY paddle_geometry = { 27,
    3, 6,
    {
        { 0, 0 },
        { 0, 1 },
        { 0, 2 },
        { 0, 3 },
        { 0, 4 },
        { 0, 5 },
        { 0, 6 },
        { 0, 7 },
        { 0, 8 },
        { 1, 0 },
        { 1, 8 },
        { 2, 0 },
        { 2, 3 },
        { 2, 4 },
        { 2, 5 },
        { 2, 8 },
        { 3, 0 },
        { 3, 8 },
        { 4, 0 },
        { 4, 1 },
        { 4, 2 },
        { 4, 3 },
        { 4, 4 },
        { 4, 5 },
        { 4, 6 },
        { 4, 7 },
        { 4, 8 },
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

    // left wall
    if (new_posx < 1)
    {
        o->dirx = -o->dirx;
        new_posx = 1;
    }

    // update object position
    o->posx = new_posx;
    o->posy = new_posy;
    o->draw(o);
}

void move_paddle_object(POBJECT o)
{
    int new_posx, new_posy;

    o->clear(o);

    new_posy = o->posy + o->diry;

    if (new_posy + o->geo->sizey > 64)
    {
        new_posy = 64 - o->geo->sizey;
    }
    else if (new_posy < 1)
    {
        new_posy = 1;
    }

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



void init_app(void)
{
    /* starta klockor port D och E */
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

static OBJECT paddle1_object = { &paddle_geometry,
    0, 0,
    122, 30,
    draw_object,
    clear_object,
    move_paddle_object,
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
    unsigned short s;
    POBJECT ball = &ball_object;
    POBJECT paddle1 = &paddle1_object;

    init_app();
    graphic_initalize();
    graphic_clear_screen();

    // main game loop
    while(1){
        int points = 0;
        
        // reset ball position and speed
        ball->posx = 1;
        ball->posy = 1;
        ball->set_speed(ball, 2, 1);

        // write welcome message
        ascii_init();
        ascii_gotoxy(1, 1);
        char test1[] = "Welcome to pong!";
        char test2[] = "points:";
        char *string = test1;
        while (*string)
            ascii_write_char(*string++);
        ascii_gotoxy(1, 2);
        string = test2;
        while (*string)
            ascii_write_char(*string++);

        ball->set_speed(ball, 4, 1);


        // repeat until game over
        while (1)
        {
            ball->move(ball);
            paddle1->move(paddle1);
            s = keyb();

            if (s == 0xA)
            {
                paddle1->set_speed(paddle1, 0, -4);
            }
            else if (s == 0xD)
            {
                paddle1->set_speed(paddle1, 0, 4);
            }

            if (objects_contact(ball, paddle1) && pixel_overlap(ball, paddle1))
            {
                ball->clear(ball);
                ball->dirx = -ball->dirx;
                // need to do it this way, strange bug otherwise!?!?
                ball->posx = 128;
                ball->posx -= paddle1->geo->sizex;
                ball->posx -= ball->geo->sizex;
                ball->draw(ball);
                points++;
            }
            else if (ball->posx > (128 + ball->geo->sizex))
            {
                // show game over message
                ascii_gotoxy(1, 1);
                char game_over[] = "Game over pal!   ";
                string = game_over;
                while(*string){
                    ascii_write_char(*string++);
                }
                // wait before starting over
                delay_milli(2000);
                break;
            }

            // write points
            ascii_gotoxy(9, 2);
            ascii_write_char(points + '0');


            delay_milli(20);
        }
    }
}
