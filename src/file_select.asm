; Load Bonko immediately after vanilla loads Sound and Z-targeting from SRAM.
.headersize(0x800110A0 - 0x00A87000)
.org 0x800912AC
    j       load_bonko_setting_hook
    nop

; File select overlay (ovl_file_choose)
.headersize(0x80803880 - 0x00BA12C0)

; Replace the options-menu update function table entry.
.org 0x80812E74
    .word update_file_select_options

; Extend the native options draw wrapper. The hook resolves the overlay's
; runtime relocation before calling FileSelect_DrawOptionsImpl.
.org 0x8080BCA0
    j       draw_file_select_options
    nop

; This entry now references the external payload, so the overlay loader must
; not relocate it as an overlay-local address.
.org 0x80813748
    .word 0
