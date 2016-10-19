#define PDP_ADD_SOURCE
#include "pdp/main.h"

void anim()
{
	int tt = 1;
	while (1)
	{
		int at = tt % 500;
		int bt = tt / 500;
		int t = (bt%2) ? tt : (500 - tt);

		int x;
		for (x = 0; x < 100; x++)
		{
			int y;
			for (y = 0; y < 100; y++)
			{
				int a = t * ((50 - x) * (50 - x) + (50 - y) * (50 - y)) % 255;
				int b = t * ((50 - x) * (50 - x) + (50 - y) * (50 - y)) / 255;
				set_pixel(x, y, (b%2) ? a : (255 - a));
			}
		}

		tt++;
	}
}

int f(int x)
{
	return 98 - (x - 50) * (x - 50) / 8;
}

void graph()
{
	draw_circle(50, 50, 20, 2, 255, 128);

	int x;
	for (x = 1; x < 100; x++)
	{
		if ((f(x - 1) < SCREEN_SIZE_Y || f(x) < SCREEN_SIZE_Y) && (f(x - 1) >= 0 || f(x) >= 0))
		{
			draw_line(x - 1, f(x - 1), x, f(x), 2, 255);
			sleep(100);
		}
	}
}

void lol(char c, int x0, int y0, Color foreground, Color background, int zoom)
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

void exec()
{
	//anim();
	//graph();

	int a[] = { 'H', 'e', 'l', 'l', 'o', ',', ' ', 'w', 'o', 'r', 'l', 'd', '!', 0 };
	draw_text(a, 12, 2, 255, 128, 1);

	int x0 = 15, y0 = 5, x1 = 20, y1 = 5, x2 = 5, y2 = 40;
	int vx0 = 2, vy0 = 1, vx1 = 1, vy1 = -2, vx2 = -1, vy2 = 1;
	int i = 0;
	while (1)
	{
		x0 += vx0;
		y0 += vy0;
		x1 += vx1;
		y1 += vy1;
		x2 += vx2;
		y2 += vy2;

		if (x0 <= 0 || x0 > SCREEN_SIZE_X)
			vx0 = -vx0;
		if (y0 <= 0 || y0 > SCREEN_SIZE_Y)
			vy0 = -vy0;
		if (x1 <= 0 || x1 > SCREEN_SIZE_X)
			vx1 = -vx1;
		if (y1 <= 0 || y1 > SCREEN_SIZE_Y)
			vy1 = -vy1;
		if (x2 <= 0 || x2 > SCREEN_SIZE_X)
			vx2 = -vx2;
		if (y2 <= 0 || y2 > SCREEN_SIZE_Y)
			vy2 = -vy2;

		int at = i % 256;
		int bt = i / 256;
		Color c = (bt % 2) ? i : (255 - i);
		draw_triangle(x0, y0, x1, y1, x2, y2, 2, c, TRANSPARENT);

		i++;
		//sleep(500);
	}
}
