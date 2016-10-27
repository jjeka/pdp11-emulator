#ifndef PDP_GRAPHICS
#define PDP_GRAPGICS

#include "font.h"

typedef int Color; /* 0 - 255 */
#define TRANSPARENT -1

#define COLOR_BLACK 0
#define COLOR_WHITE 255

#define SCREEN_SIZE_X	100
#define SCREEN_SIZE_Y	100

void clear_screen(Color color);
void set_pixel(int x, int y, Color c);
Color get_pixel(int x, int y);
void draw_line(int x1, int y1, int x2, int y2, int size, Color c);
void draw_circle(int x0, int y0, int r, int size, Color color, Color fillColor);
void draw_rectangle(int x1, int y1, int x2, int y2, int size, Color color, Color fillColor);
void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, int size, Color color, Color fillColor);
void draw_symbol(char c, int x0, int y0, Color foreground, Color background, int zoom);
void draw_text(const char str[], int x0, int y0, Color foreground, Color background, int zoom);
void draw_image(int x0, int y0, int sx, int sy, const unsigned char data[]);

#endif // PDP_GRAPHICS
