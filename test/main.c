#include "icon.h"
#include "pdp/utils.h"
#include "pdp/graphics.h"
#include "pdp/keyboard.h"

struct Application
{
	char name[50];
	void (*handler)();
	void (*key_handler)(unsigned key);
};

int running = 0;

void animation()
{
	int tt = 1;
	while (running)
	{
		int t = ((tt / 500) % 2) ? tt : (500 - tt);

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

void screensaver1()
{
	int i = 0;	
	while (running)
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

void screensaver2()
{
	int x0 = 15, y0 = 5, x1 = 20, y1 = 5, x2 = 5, y2 = 40;
	int vx0 = 2, vy0 = 1, vx1 = 1, vy1 = -2, vx2 = -1, vy2 = 1;
	int i = 0;
	while (running)
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

		Color c = ((i / 256) % 2) ? i : (255 - i);
		draw_triangle(x0, y0, x1, y1, x2, y2, 2, c, TRANSPARENT);

		i++;
	}
}

void invert_rect(int x0, int y0, int sx, int sy)
{
	int x;
	for (x = x0; x < x0 + sx; x++)
	{
		int y;
		for (y = y0; y < y0 + sy; y++)
		{
			set_pixel(x, y, 255 - get_pixel(x, y));
		}
	}
}

int te_x = 0;
int te_y = 0;
int te_t = 0;
int te_inverted = 0;

void text_editor_remove_cursor()
{
	if (te_inverted)
	{	
		int x = te_x;
		int y = te_y;
		invert_rect(1 + 6 * x, 1 + 8 * y, 7, 9);
	}
}

void text_editor_update_cursor()
{
	int x = te_x;
	int y = te_y;
	invert_rect(1 + 6 * x, 1 + 8 * y, 7, 9);
	te_t = 0;
	te_inverted = 1;
}

void text_editor_key_pressed(unsigned key)
{
	if (key == 5 /* return */)
	{
		text_editor_remove_cursor();

		te_x = 0;
		te_y++;

		if (te_y >= 12)
			te_y = 0;

		text_editor_update_cursor();
	}
	else if (key == 1 /* up */)
	{
		text_editor_remove_cursor();

		te_y--;
		if (te_y < 0)
			te_y = 11;

		text_editor_update_cursor();
	}
	else if (key == 2 /* down */)
	{
		text_editor_remove_cursor();

		te_y++;
		if (te_y >= 12)
			te_y = 0;

		text_editor_update_cursor();
	}
	else if (key == 3 /* left */)
	{
		text_editor_remove_cursor();

		te_x--;
		if (te_x < 0)
			te_x = 15;

		text_editor_update_cursor();
	}
	else if (key == 4 /* right */)
	{
		text_editor_remove_cursor();

		te_x++;
		if (te_x >= 16)
			te_x = 0;

		text_editor_update_cursor();
	}
	else if (key == 6 /* backspace */)
	{
		text_editor_remove_cursor();

		te_x--;
		if (te_x < 0)
		{
			te_x = 15;
			te_y--;

			if (te_y < 0)
				te_y = 11;
		}

		int x = te_x;
		int y = te_y;
		draw_rectangle(1 + 6 * x, 1 + 8 * y, 7 + 6 * x, 9 + 8 * y, 0, 0, 0);

		text_editor_update_cursor();
	}
	else if (key == 7 /* delete */)
	{
		text_editor_remove_cursor();

		int x = te_x;
		int y = te_y;
		x++;
		if (x >= 16)
		{
			x = 0;
			y++;

			if (y == 12)
				y = 0;
		}

		draw_rectangle(1 + 6 * x, 1 + 8 * y, 7 + 6 * x, 9 + 8 * y, 0, 0, 0);

		text_editor_update_cursor();
	}
	else
	{
		int x = te_x;
		int y = te_y;

		te_x++;
		if (te_x >= 16)
		{
			te_x = 0;
			te_y++;

			if (te_y == 12)
				te_y = 0;
		}

		draw_symbol((key & 0xFF), 2 + 6 * x, 2 + 8 * y, 255, 0, 1);

		draw_rectangle(1 + 6 * x, 1 + 8 * y, 7 + 6 * x, 9 + 8 * y, 0, 0, TRANSPARENT);

		text_editor_update_cursor();
	}
}

void text_editor()
{
	te_x = 0;
	te_y = 0;
	te_inverted = 0;

	while (running)
	{
		if (te_t == 500)
		{
			int x = te_x;
			int y = te_y;
			invert_rect(1 + 6 * x, 1 + 8 * y, 7, 9);
			te_t = 0;
			te_inverted = !te_inverted;
		}
		sleep(1);
		te_t++;
	}
}

void draw_about()
{
	draw_image(25, 15, 50, 50, ICON_DATA);
	draw_text("PDP11", 24, 5, 255, TRANSPARENT, 2);
	draw_text("NIKITENKO EVGENY", 2, 70, 255, TRANSPARENT, 1);
	draw_text("IVANOV ALEXEY", 10, 80, 255, TRANSPARENT, 1);
	draw_text("SAMARA OLEKSA", 10, 90, 255, TRANSPARENT, 1);
}

void about()
{
	draw_about();

	while(running);
}

int selection = 0;
int currentApp = -1;
int mustRun = -1;
const struct Application apps[] = {
	{ "Text editor", text_editor, text_editor_key_pressed }, 
	{ "Screensaver 1", screensaver1, NULL }, 
	{ "Screensaver 2", screensaver2, NULL }, 
	{ "Animation", animation, NULL }, 
	{ "About", about, NULL } };
const int NUM_APPS = sizeof (apps) / sizeof (apps[0]);

void draw_menu()
{
	int i;
	for (i = 0; i < NUM_APPS; i++)
	{
		int color = (i == selection) ? 255 : 100;
		int fillColor = (i == selection) ? 200 : 0;
		int textColor = (i == selection) ? 0 : 255;
		draw_rectangle(0, 20 * i, 99, 20 * (i + 1) - 1, 1, color, fillColor);
		draw_text(apps[i].name, 4, 20 * i + 6, textColor, TRANSPARENT, 1);
	}
}

void key_pressed(unsigned key)
{
	if (key == 13)
	{
		if (currentApp != -1)
			running = 0;

		return;
	}

	if (currentApp == -1)
	{
		if (key == 1 /* up */)
		{
			selection--;
			if (selection < 0)
				selection = NUM_APPS - 1;
			draw_menu();
		}
		else if (key == 2 /* down */)
		{
			selection++;
			if (selection >= NUM_APPS)
				selection = 0;
			draw_menu();
		}
		else if (key == 5 /* return */)
		{
			if (apps[currentApp].handler != NULL)
			{
				mustRun = selection;
			}
		}
	}
	else
	{
		if (apps[currentApp].key_handler != NULL)
			apps[currentApp].key_handler(key);
	}

}

void exec()
{
	draw_about();
	sleep(3000);
	clear_screen(COLOR_BLACK);

	set_kb_handler(key_pressed);

	draw_menu();
	while (1)
	{
		if (mustRun != -1)
		{
			mustRun = -1;
			
			currentApp = selection;
			running = 1;

			clear_screen(COLOR_BLACK);
			apps[currentApp].handler();
			clear_screen(COLOR_BLACK);
			draw_menu();

			currentApp = -1;
			mustRun = -1;
		}
	}
}
