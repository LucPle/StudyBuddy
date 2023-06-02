
#define LINE1  0x80 // 1st line
#define LINE2  0xC0 // 2nd line

void lcd_init(void);
void lcd_byte(int bits, int mode);
void lcd_toggle_enable(int bits);

void lcdLoc(int line); //move cursor
void ClrLcd(void); // clr LCD return home
void typeln(const char *s);