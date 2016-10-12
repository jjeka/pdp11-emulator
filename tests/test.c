volatile unsigned char* vram = (volatile unsigned char*) (32768);

volatile unsigned char* get_vram();
void exec();

void _start()
{
	asm volatile("mov $0177776, sp\nmov $0177776, r5");

	exec();

	int __end = 0;
	asm volatile("halt");
	asm volatile("mov %0, r4\nhalt\njmp $0" :: "r"(__end));
}

volatile unsigned char* get_vram()
{
	return vram;
}

void exec()
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
				*(get_vram() + (y * 100 + x)) = (b%2) ? a : (255 - a);
			}
		}

		tt++;
	}
}
