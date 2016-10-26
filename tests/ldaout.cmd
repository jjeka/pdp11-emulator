OUTPUT_FORMAT("a.out-pdp11")
ENTRY(_start)

MEMORY
{
	rom(RX)		: ORIGIN = 0x0010, LENGTH = 0x7FF0
	ram(WIA)	: ORIGIN = 0xA710, LENGTH = 0x58F0
}

SECTIONS
{
	.text :
	{
		*(.text)
		_DATA_SECTION_SIZE = SIZEOF (.data);
		_DATA_SECTION_START = SIZEOF (.text);
		_DATA_SECTION_RAM_START = 0xA710 - 16;
	} > rom

	.data :
	{
		*(.data)
	} > ram AT> rom

	.bss :
	{
		*(.bss)
	} > ram
}
