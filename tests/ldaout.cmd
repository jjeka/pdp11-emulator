OUTPUT_FORMAT("binary")
ENTRY(__start)

MEMORY
{
	rom(RX)		: ORIGIN = 0x0000, LENGTH = 0x8000
	ram(WIA)	: ORIGIN = 0xA710, LENGTH = 0x58F0
}

SECTIONS
{
	.text :
	{
		*(.text)
		_DATA_SECTION_SIZE = SIZEOF (.data);
		_DATA_SECTION_START = SIZEOF (.text) + 16;
		_DATA_SECTION_RAM_START = 0xA710 + 16;
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
