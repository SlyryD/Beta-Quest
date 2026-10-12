; Load custom options immediately after vanilla loads Sound and Z-targeting from SRAM.
.headersize(0x800110A0 - 0x00A87000)
.org 0x800912AC
    j       load_bonko_setting_hook
    nop

; File select overlay (ovl_file_choose)
.headersize(0x80803880 - 0x00BA12C0)

; Replace the options-menu update function table entry.
.org 0x80812E74
    .word update_file_select_options

; Add an MQ choice to the name-entry button row.
.org 0x80812E64
    .word update_mq_name_entry

; Persist the per-file MQ setting after the retail new-save initialization.
.org 0x808098E8
    jal     create_file_with_mq_config

; Draw the MQ creation button after the native name-entry controls.
.org 0x80810630
    jal     draw_file_select_mq

; Draw per-file MQ markers after the native window contents on both the
; configuration screens and the Open File confirmation screen.
.org 0x8081040C
    jal     draw_file_select_mq_markers

.org 0x80811710
    jal     draw_file_select_mq_markers

; These entries now reference the external payload and must not be relocated
; as overlay-local addresses.
.org 0x80813618
    .word 0

.org 0x80813738
    .word 0

.org 0x8081360C
    .word 0

.org 0x80813664
    .word 0

; Extend the native options draw wrapper. The hook resolves the overlay's
; runtime relocation before calling FileSelect_DrawOptionsImpl.
.org 0x8080BCA0
    j       draw_file_select_options
    nop

; Only pulse Z Targeting when the native selection is exactly 1. Vanilla
; treats every nonzero value as Z Targeting, including our custom rows.
.org 0x8080B45C
    addiu   t9, t9, -1
    bnez    t9, 0x8080B510

; This entry now references the external payload, so the overlay loader must
; not relocate it as an overlay-local address.
.org 0x80813748
    .word 0
