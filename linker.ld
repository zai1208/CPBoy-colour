ENTRY(_main);

SECTIONS {
	start_address = 0x8CFE6000;
	.init start_address : AT(start_address) {
		*(.init)
	}
	info_address = 0x8CFE6010;
	. = info_address;
	.hollyhock_name : {
		*(.hollyhock_name)
	}
	.hollyhock_description : {
		*(.hollyhock_description)
	}
	.hollyhock_author : {
		*(.hollyhock_author)
	}
	.hollyhock_version : {
		*(.hollyhock_version)
	}

	.text : {
		*(.text)
		*(.rodata*)
  }

	.data : {
		*(.data)
	}

	.bss : {
		*(.bss)
    *(COMMON)
	}
  
  x_mem_addr = 0xE5007000;
  . = x_mem_addr;

  .oc_mem.x : {
    *(.oc_mem.x)
  }
  
  y_mem_addr = 0xE5017000;
  . = y_mem_addr;

  .oc_mem.y : {
    *(.oc_mem.y)
  }
  
  il_mem_addr = 0xE5200000;
  . = il_mem_addr;

  .oc_mem.il : {
    *(.oc_mem.il.data)
    *(.oc_mem.il.text)
  }
}