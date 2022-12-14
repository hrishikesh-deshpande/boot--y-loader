#include "kernel.h"
#define KEYBOARD_PORT 0x60
#define VIDEO_MEM 0xb8000

#define KEY_A 0x1E
#define KEY_B 0x30
#define KEY_C 0x2E
#define KEY_D 0x20
#define KEY_E 0x12
#define KEY_F 0x21
#define KEY_G 0x22
#define KEY_H 0x23
#define KEY_I 0x17
#define KEY_J 0x24
#define KEY_K 0x25
#define KEY_L 0x26
#define KEY_M 0x32
#define KEY_N 0x31
#define KEY_O 0x18
#define KEY_P 0x19
#define KEY_Q 0x10
#define KEY_R 0x13
#define KEY_S 0x1F
#define KEY_T 0x14
#define KEY_U 0x16
#define KEY_V 0x2F
#define KEY_W 0x11
#define KEY_X 0x2D
#define KEY_Y 0x15
#define KEY_Z 0x2C
#define KEY_1 0x02
#define KEY_2 0x03
#define KEY_3 0x04
#define KEY_4 0x05
#define KEY_5 0x06
#define KEY_6 0x07
#define KEY_7 0x08
#define KEY_8 0x09
#define KEY_9 0x0A
#define KEY_0 0x0B
#define KEY_MINUS 0x0C
#define KEY_EQUAL 0x0D
#define KEY_SQUARE_OPEN_BRACKET 0x1A
#define KEY_SQUARE_CLOSE_BRACKET 0x1B
#define KEY_SEMICOLON 0x27
#define KEY_BACKSLASH 0x2B
#define KEY_COMMA 0x33
#define KEY_DOT 0x34
#define KEY_FORESLHASH 0x35
#define KEY_F1 0x3B
#define KEY_F2 0x3C
#define KEY_F3 0x3D
#define KEY_F4 0x3E
#define KEY_F5 0x3F
#define KEY_F6 0x40
#define KEY_F7 0x41
#define KEY_F8 0x42
#define KEY_F9 0x43
#define KEY_F10 0x44
#define KEY_F11 0x85
#define KEY_F12 0x86
#define KEY_BACKSPACE 0x0E
#define KEY_DELETE 0x53
#define KEY_DOWN 0x50
#define KEY_END 0x4F
#define KEY_ENTER 0x1C
#define KEY_ESC 0x01
#define KEY_HOME 0x47
#define KEY_INSERT 0x52
#define KEY_KEYPAD_5 0x4C
#define KEY_KEYPAD_MUL 0x37
#define KEY_KEYPAD_Minus 0x4A
#define KEY_KEYPAD_PLUS 0x4E
#define KEY_KEYPAD_DIV 0x35
#define KEY_LEFT 0x4B
#define KEY_PAGE_DOWN 0x51
#define KEY_PAGE_UP 0x49
#define KEY_PRINT_SCREEN 0x37
#define KEY_RIGHT 0x4D
#define KEY_SPACE 0x39
#define KEY_TAB 0x0F
#define KEY_UP 0x48
// index for video buffer array
uint32 vga_index;
// counter to store new lines
static uint32 next_line_index = 1; 
// fore & back color values
uint8 g_fore_color = WHITE, g_back_color = BLUE;

/*
16 bit video buffer elements(register ax)
8 bits(ah) higher :
  lower 4 bits - forec olor
  higher 4 bits - back color

8 bits(al) lower :
  8 bits : ASCII character to print
*/
uint16 vga_entry(unsigned char ch, uint8 fore_color, uint8 back_color)
{
    uint16 ax = 0;
    uint8 ah = 0, al = 0;

    ah = back_color;
    ah <<= 4;
    ah |= fore_color;
    ax = ah;
    ax <<= 8;
    al = ch;
    ax |= al;

    return ax;
}

// clear video buffer array
void clear_vga_buffer(uint16 **buffer, uint8 fore_color, uint8 back_color)
{
    uint32 i;
    for (i = 0; i < BUFSIZE; i++)
    {
        (*buffer)[i] = vga_entry(NULL, fore_color, back_color);
    }
    next_line_index = 1;
    vga_index = 0;
}

// initialize vga buffer
void init_vga(uint8 fore_color, uint8 back_color)
{
    vga_buffer = (uint16 *)VGA_ADDRESS;
    clear_vga_buffer(&vga_buffer, fore_color, back_color);
    g_fore_color = fore_color;
    g_back_color = back_color;
}

/*
increase vga_index by width of row(80)
*/
void print_new_line()
{
    if (next_line_index >= 55)
    {
        next_line_index = 0;
        clear_vga_buffer(&vga_buffer, g_fore_color, g_back_color);
    }
    vga_index = 80 * next_line_index;
    next_line_index++;
}

// assign ascii character to video buffer
void print_char(char ch)
{
    // vga_buffer[0] = 'a';
    // // vid += 1;
    // vga_buffer[1] = 0x1f;
    // vid += 1;
    vga_buffer[vga_index] = vga_entry(ch, g_fore_color, g_back_color);
    vga_index++;
}

// print string by calling print_char
void print_string()
{
    // vga_buffer[0] = 'a';
    // // vid += 1;
    // vga_buffer[1] = 0x1f;
    const char WelcomeStr[100] = "Congratulations!!! Kernel is booted succesffully";
    uint32 index = 0;
    while (WelcomeStr[index] != '\0')
    {
        print_char(WelcomeStr[index]);
        index++;
    }
    print_new_line();
    print_new_line();

    const char KeyInStr[50] = "Welcome to IIIT Hyderabad";
    index = 0;
    while (WelcomeStr[index] != '\0')
    {
        print_char(KeyInStr[index]);
        index++;
    }
}

uint8 inb(uint16 port)
{
    uint8 ret=0;
    asm volatile("inb %1, %0"
                 : "=a"(ret)
                 : "d"(port));
    return ret;
}

char get_input_keycode()
{
    char ch = 0;
    while ((ch = inb(KEYBOARD_PORT)) != 0)
    {
        if (ch > 0)
            return ch;
    }
    return ch;
}

/*
keep the cpu busy for doing nothing(nop)
so that io port will not be processed by cpu
here timer can also be used, but lets do this in looping counter
*/
void wait_for_io(uint32 timer_count)
{
    while (1)
    {
        asm volatile("nop");
        timer_count--;
        if (timer_count <= 0)
            break;
    }
}

void sleep(uint32 timer_count)
{
    wait_for_io(timer_count);
}

char get_ascii_char(uint8 key_code)
{
    switch (key_code)
    {
    case KEY_A:
        return 'A';
    case KEY_B:
        return 'B';
    case KEY_C:
        return 'C';
    case KEY_D:
        return 'D';
    case KEY_E:
        return 'E';
    case KEY_F:
        return 'F';
    case KEY_G:
        return 'G';
    case KEY_H:
        return 'H';
    case KEY_I:
        return 'I';
    case KEY_J:
        return 'J';
    case KEY_K:
        return 'K';
    case KEY_L:
        return 'L';
    case KEY_M:
        return 'M';
    case KEY_N:
        return 'N';
    case KEY_O:
        return 'O';
    case KEY_P:
        return 'P';
    case KEY_Q:
        return 'Q';
    case KEY_R:
        return 'R';
    case KEY_S:
        return 'S';
    case KEY_T:
        return 'T';
    case KEY_U:
        return 'U';
    case KEY_V:
        return 'V';
    case KEY_W:
        return 'W';
    case KEY_X:
        return 'X';
    case KEY_Y:
        return 'Y';
    case KEY_Z:
        return 'Z';
    case KEY_1:
        return '1';
    case KEY_2:
        return '2';
    case KEY_3:
        return '3';
    case KEY_4:
        return '4';
    case KEY_5:
        return '5';
    case KEY_6:
        return '6';
    case KEY_7:
        return '7';
    case KEY_8:
        return '8';
    case KEY_9:
        return '9';
    case KEY_0:
        return '0';
    case KEY_MINUS:
        return '-';
    case KEY_EQUAL:
        return '=';
    case KEY_SQUARE_OPEN_BRACKET:
        return '[';
    case KEY_SQUARE_CLOSE_BRACKET:
        return ']';
    case KEY_SEMICOLON:
        return ';';
    case KEY_BACKSLASH:
        return '\\';
    case KEY_COMMA:
        return ',';
    case KEY_DOT:
        return '.';
    case KEY_FORESLHASH:
        return '/';
    case KEY_SPACE:
        return ' ';
    default:
        return 0;
    }
}

void test_input()
{
    char ch = 1;
    char keycode = 0;
    int limit = 80;
    while (limit > 0)
    {
        print_char('_');
        limit--;
    }
    print_new_line();
    do
    {
        keycode = get_input_keycode();
        if (keycode == KEY_ENTER)
        {
            print_new_line();
        }
        else
        {
            ch = get_ascii_char(keycode);
            print_char(ch);
        }
        sleep(0x02FFFFFF);
    } while (ch > 0);

    print_new_line();
    limit = 80;
    while (limit>0)
    {
        print_char('_');
        limit--;
    }
    print_new_line();
    limit = 35;
    while (limit > 0)
    {
        print_char(' ');
        limit--;
    }
    print_char('B');
    print_char('U');
    print_char('B');
    print_char('B');
    print_char('Y');
    print_char('E');
    print_char(' ');
    print_char(':');
    print_char(')');
}

void main()
{
    // volatile unsigned char *vid = (unsigned char *)VIDEO_MEM;
    // *vid = 'A';
    init_vga(WHITE, BLUE);
    print_string();
    print_new_line();
    test_input();
}