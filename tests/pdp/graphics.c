#include "graphics.h"

volatile unsigned char* const VRAM = (volatile unsigned char*) (32768);

void clear_screen(Color color)
{
	int i;
	for (i = 0; i < SCREEN_SIZE_X * SCREEN_SIZE_Y; i++)
		*(VRAM + i) = color;
}

void set_pixel(int x, int y, Color c)
{
	if (x >= 0 && x < SCREEN_SIZE_X && y >= 0 && y < SCREEN_SIZE_Y)
		*(VRAM + (y * SCREEN_SIZE_X + x)) = c;
}

Color get_pixel(int x, int y)
{
	if (x >= 0 && x < SCREEN_SIZE_X && y >= 0 && y < SCREEN_SIZE_Y)
		return *(VRAM + (y * SCREEN_SIZE_X + x));
	else
		return COLOR_BLACK;
}

void draw_line(int x1, int y1, int x2, int y2, int size, Color c)
{
	int xmin = max(min(x1, x2), 0);
	int xmax = min(max(x1, x2), SCREEN_SIZE_X);
	int ymin = max(min(y1, y2), 0);
	int ymax = min(max(y1, y2), SCREEN_SIZE_Y);
	unsigned size2 = ((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1)) * size * size;

	int x;
	for (x = xmin; x <= xmax; x++)
	{
		int y;
		for (y = ymin; y <= ymax; y++)
		{
			unsigned s = (x1 - x) * (y2 - y) - (x2 - x) * (y1 - y);
			if (abs(s) < 128 && 4 * s * s <= size2)
			{
				set_pixel(x, y, c);
			}
		}
	}
}

void draw_circle(int x0, int y0, int r, int size, Color color, Color fillColor)
{
	int x;
	for (x = x0 - r - size; x <= x0 + r + size; x++)
	{
		int y;
		for (y = y0 - r - size; y <= y0 + r + size; y++)
		{
			int len = (x-x0) * (x-x0) + (y-y0) * (y-y0);
			if (len >= (r - size) * (r - size) && len <= r * r && color != TRANSPARENT)
				set_pixel(x, y, color);
			else if (len < (r - size) * (r - size) && fillColor != TRANSPARENT)
				set_pixel(x, y, fillColor);
		}
	}
}

void draw_rectangle(int x1, int y1, int x2, int y2, int size, Color color, Color fillColor)
{
	if (color == TRANSPARENT && fillColor == TRANSPARENT)
		return;

	int xmin = max(min(x1, x2), 0);
	int xmax = min(max(x1, x2), SCREEN_SIZE_X);
	int ymin = max(min(y1, y2), 0);
	int ymax = min(max(y1, y2), SCREEN_SIZE_Y);

	int x;
	for (x = xmin; x <= xmax; x++)
	{
		int y;
		for (y = ymin; y <= ymax; y++)
		{
			if ((x - xmin <= size || xmax - x <= size) && color != TRANSPARENT)
				set_pixel(x, y, color);
			else if ((y - ymin <= size || ymax - y <= size) && color != TRANSPARENT)
				set_pixel(x, y, color);
			else
				set_pixel(x, y, fillColor);
		}
	}
}

void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, int size, Color color, Color fillColor)
{
	if (fillColor != TRANSPARENT)
	{
		int minx = min(x0, min(x1, x2));
		int miny = min(y0, min(y1, y2));
		int maxx = max(x0, max(x1, x2));
		int maxy = max(y0, max(y1, y2)); 
		int s = abs((x1 - x0) * (y2 - y0) - (x2 - x0) * (y1 - y0));
		int x, y;
		for (x = minx; x <= maxx; x++)
		{
			for (y = miny; y <= maxy; y++)
			{
				int s0 = abs((x - x1) * (y - y2) - (x - x2) * (y - y1));
				int s1 = abs((x - x0) * (y - y2) - (x - x2) * (y - y0));
				int s2 = abs((x - x1) * (y - y0) - (x - x0) * (y - y1));
				int ss = s0 + s1 + s2;
			
				if (ss == s)
					set_pixel(x, y, fillColor);
			}	
		}
	}
	
	if (color != TRANSPARENT)
	{
		draw_line(x0, y0, x1, y1, size, color);
		draw_line(x0, y0, x2, y2, size, color);
		draw_line(x1, y1, x2, y2, size, color);
	}
}

void draw_symbol(int c, int x0, int y0, Color foreground, Color background, int zoom)
{
	if (c < 0)
		return;

	int start = ((int) c) * 5;
	for (int x = 0; x < 5; x++)
	{
		unsigned char data = FONT_DATA[start + x];

		for (int y = 1; y < 8; y++)
		{
			if (data & (1 << y))
			{
				if (foreground != TRANSPARENT)
				{
					for (int xx = 0; xx < zoom; xx++)
						for (int yy = 0; yy < zoom; yy++)
							set_pixel(x0 + x * zoom + xx, y0 + y * zoom + yy - 1, foreground);
				}
			}
			else
			{
				if (background != TRANSPARENT)
				{
					for (int xx = 0; xx < zoom; xx++)
						for (int yy = 0; yy < zoom; yy++)
							set_pixel(x0 + x * zoom + xx, y0 + y * zoom + yy - 1, background);
				}
			}
		}
	}
}

void draw_text(int str[], int x0, int y0, Color foreground, Color background, int zoom)
{
	int i;
	for (i = 0; str[i]; i++)
	{
		if (str[i + 1] && background != TRANSPARENT)
		{
			for (int xx = 0; xx < zoom; xx++)
				for (int yy = 0; yy < zoom * 7; yy++)
					set_pixel(x0 + 6 * zoom * (i + 1) - zoom + xx, y0 + yy, background);
		}
		draw_symbol(str[i], x0 + 6 * zoom * i, y0, foreground, background, zoom);
	}
}
