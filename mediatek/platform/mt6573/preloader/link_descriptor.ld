OUTPUT_ARCH(arm)

ENTRY(_start)

SECTIONS {

    . = 0x90007300;    
    .start ALIGN(4) : {	
        *(.text.start)
    }
    
    . = 0x900074FC;
    .rom_info ALIGN(4) : {	    	    	    	     
    	*(.text.rom_info)
    }

    .text ALIGN(4) : {
        *(.text)
    }


    .rodata ALIGN(4) : {
        *(.rodata)
    }

    .data ALIGN(4) : {
        *(.data)
    }


    .got ALIGN(4) : {
        *(.got)
    }

    __boot_end = .;


    .bss ALIGN(16) : {
        _bss_start = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        _bss_end = .;
    }


    .comment ALIGN(16) : {
        *(.comment)
    }

}
