/* 内存区域定义 */
MEMORY {
  flash (rx)  : ORIGIN = 0x0000, LENGTH = 64K  /* Flash存储空间 */
  ram   (rwx) : ORIGIN = RAM_BEGIN, LENGTH = 0xEFFF - RAM_BEGIN + 1  /* 实际RAM区 */
  sfr   (rw)  : ORIGIN = 0xF000, LENGTH = 4K   /* 特殊功能寄存器区 */
}

/* 段布局定义 */
SECTIONS {
  /* 代码段 - 存储在Flash，执行在ROM镜像区 */
  .text ORIGIN(flash) : {
    *(.text .text.*)           /* 程序代码 */
    KEEP(*(SORT_NONE(.init)))  /* 保留初始化函数 */
    KEEP(*(SORT_NONE(.fini)))  /* 保留终止函数 */
    *(.rodata .rodata.*)       /* 只读数据 */
    *(.strings)                /* 字符串常量 */
    _etext = .;                /* 代码段结束标记 */
  } > flash AT > flash         /* 物理存储和执行地址均在Flash */

  /* 构造函数/析构函数表 */
  .tors : {
    ___ctors = .;
    KEEP(*(.ctors .ctors.*))
    ___ctors_end = .;
    ___dtors = .;
    KEEP(*(.dtors .dtors.*))
    ___dtors_end = .;
  } > flash AT > flash

  /* 初始化数据段 - 存储在Flash，运行时在RAM */
  .data : ALIGN(4) {
    _data_load = LOADADDR(.data);  /* Flash中的加载地址 */
    _data_start = .;               /* RAM中的运行地址 */
    *(.data .data.*)               /* 初始化数据 */
    . = ALIGN(4);
    _data_end = .;                 /* 数据段结束标记 */
  } > ram AT > flash

  /* 未初始化数据段 - 位于RAM */
  .bss (NOLOAD) : ALIGN(4) {
    _bss_start = .;                /* BSS段起始 */
    *(.bss .bss.*)                 /* 未初始化数据 */
    *(COMMON)                      /* 公共变量 */
    . = ALIGN(4);
    _end = .;                      /* 所有程序数据结束 */
  } > ram

  /* 栈空间 - 位于RAM高端 */
  .stack (NOLOAD) : ALIGN(4) {
    _stack_bottom = .;
    . += 0x1000;                   /* 保留4KB栈空间 */
    _stack_top = .;
  } > ram

  /* 特殊功能寄存器区 - 禁止分配 */
  .sfr (NOLOAD) : {
    *(.sfr .sfr.*)                 /* SFR区域 */
  } > sfr

  /* 调试信息 - 不加载到内存 */
  .stab 0 (NOLOAD) : { *(.stab) }
  .stabstr 0 (NOLOAD) : { *(.stabstr) }

  /* 符号定义 */
  _heap_start = _end;               /* 堆起始地址 */
  _heap_end = ORIGIN(ram) + LENGTH(ram) - SIZEOF(.stack); /* 堆结束地址 */
}
