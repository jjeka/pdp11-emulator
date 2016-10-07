void x(int a);

void _start()
{
	int a=2+4;
	int b =1 * 2 *3 * a;

	x(a);
}

void x(int a)
{
	asm("nop\n"
		"nop\n"
		"nop");
}
