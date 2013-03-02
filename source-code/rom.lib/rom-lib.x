OUTPUT_FORMAT("elf32-h8300")
OUTPUT_ARCH(h8300h)
ENTRY("_start")
MEMORY
{
	syscall(r)	: o = 0x0100, l = 0x0200
	rom_tbl(r)	: o = 0x5800, l = 0x0100
	rom(r)		: o = 0x5900, l = 0x2700
	ram(rwx)	: o = 0xf9c0, l = 0x0080
}
SECTIONS 				
{ 					
.syscall 0x100: {
	_sys_write	= . +  0;
	_write_string	= . +  4;
	_write_mode	= . +  8;
	_set_membuf	= . +  8;
	_sys_read	= . + 12;
	_read_string	= . + 16;
	_sio_speed	= . + 20;
	_atohex		= . + 24;
	_lcd_clear	= . + 28;
	_lcdsetup	= . + 32;
	_sleep		= . + 60;
	_write_decimal	= . + 100;
	_atodec		= . + 104;
	_int_regist	= . + 188;
	_int_unregist	= . + 192;
	_ps2_setup	= . + 232;
	_key_read	= . + 236;
	_nmi_edge	= . + 240;
	_read_mode	= . + 244;
	_eep_setup	= . + 248;
	_eep_write	= . + 252;
	_eep_read	= . + 256;
	_print		= . + 316;
	_sprint		= . + 320;
	_scan		= . + 324;
	_sscan		= . + 328;
	_rtc_setup	= . + 340;
	_rtc_read	= . + 344;
	_rtc_write	= . + 348;
	_mem_size	= . + 352;
	_sio_change	= . + 356;
	_easy_max	= . + 360;
	_easy_dir	= . + 364;
	_easy_format	= . + 368;
	_easy_find	= . + 372;
	_easy_del	= . + 376;
	_easy_create	= . + 380;
	_easy_write	= . + 384;
	_easy_read	= . + 388;
	_easy_exec	= . + 392;
 	_get_eepsize	= . + 396;
 	_putch		= . + 400;
 	_getch		= . + 404;
        }  > syscall
.rom_tbl 0x5800:{
LONG(DEFINED(_func_tbl)?ABSOLUTE(_func_tbl):ABSOLUTE(_nop))
	} > rom_tbl
.text :	{
	*(.text)
	*(.strings)
	*(.rodata)
    *(.rodata.str1.1)
   	 _etext = . ; 
	}  > rom
.tors : {
	___ctors = . ;
	*(.ctors)
	___ctors_end = . ;
	___dtors = . ;
	*(.dtors)
	___dtors_end = . ;
	}  > rom
.data : AT ( ADDR(.tors) + SIZEOF(.tors) ){
	___data = . ;
	*(.data)
	*(.tiny)
	 _edata = . ; 
	}  > ram
.bss : {
	 _bss_start = . ;
	*(.bss)
	*(COMMON)
	 _end = . ;  
	}  >ram
.stack : {
	 _stack = . ; 
	*(.stack)
	}  > stack
}
