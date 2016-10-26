#define PDP_ADD_SOURCE
#include "pdp/main.h"

void key_pressed(unsigned data)
{
	//sleep(300);
	draw_text("emulator", 28, 30, 210, TRANSPARENT, 1);
}

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

void screen_saver1()
{
	int i = 0;	
	while(1)
	{
		int at = i % 16;
		int bt = i / 16;
		Color c = (bt % 2) ? at : (15 - at);
		c *= 16;
		i++;
		draw_line(i%100, 0, 99, i%100, 2, c);
		draw_line(i%100, 0, 0, 99 - i%100, 2, c);
		draw_line(99 - i%100, 99, 0, 99 - i%100, 2, c);
		draw_line(99, i%100, 99 - i%100, 99, 2, c);
	}
}

void screen_saver2()
{
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
	}
}

void exec()
{
	_set_kb_handler(key_pressed);
	draw_text("PDP11", 24, 10, 255, TRANSPARENT, 2);
	draw_text("emulator", 28, 30, 210, TRANSPARENT, 1);
	draw_text("BY", 40, 45, 150, TRANSPARENT, 2);
	draw_text("NIKITENKO EVGENY", 2, 70, 255, TRANSPARENT, 1);
	draw_text("IVANOV ALEXEY", 10, 80, 255, TRANSPARENT, 1);
	draw_text("SAMARA OLEKSA", 10, 90, 255, TRANSPARENT, 1);
	sleep(3000);
	clear_screen(COLOR_BLACK);

	screen_saver2();
}
