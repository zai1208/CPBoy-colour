ENTRY(_main)
SECTIONS
{
    . = 0x8C200080;
    .text :
    {
        *(.text.main);
        *(.text*);
    }
}