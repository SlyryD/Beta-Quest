.orga 0xE6E344 ;patch the branch that decides whether or not to draw the japanese text below the main logo. making it always draw. regardless of language selected.
nop

.orga 0x1795000
.incbin("../resources/version_ia8.bin")
.orga 0x17AE300
.incbin("../resources/copyright_ia8.bin")
.orga 0x17B3C40
.incbin("../resources/beta_quest_i8.bin")

.headersize(0x801EF910 - 0xE6C0D0)
.org 0x801F1078
beql    t6, r0, title_screen_draw_dd

.org 0x801F108C
title_screen_draw_dd:

.org 0x801F11F4
addiu a3, a3, 0x60

.org 0x801F1204
addiu t0, t0, 0x72