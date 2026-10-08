load_bonko_setting_hook:
    addiu   sp, sp, -0x18
    sw      ra, 0x10(sp)

    jal     load_file_select_settings
    lw      a0, 0x00(s2)

    lw      ra, 0x10(sp)
    addiu   sp, sp, 0x18

    ; Displaced Sram_InitSram epilogue.
    lw      ra, 0x24(sp)
    lw      s0, 0x18(sp)
    j       0x800912B4
    nop
