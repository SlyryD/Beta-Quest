
;patch the "DISK" texture w/ the version number texture
.orga 0x1795000
.incbin("../resources/version_ia8.bin")

;patch the copyright w/ ours
.orga 0x17AE300
.incbin("../resources/copyright_ia8.bin")

;patch the japanese text texture w/ "Beta Quest"
.orga 0x17B3C40
.incbin("../resources/beta_quest_i8.bin")



.headersize(0x801EF910 - 0xE6C0D0) ;move to the title_screen file (ovl_En_Mag) 0x801EF910 is the location that this file gets loaded in RAM right now.

;patch the branch that decides whether or not to draw the japanese text below the main logo. making it always draw. regardless of language selected.
.org 0x801F1B84
nop

;patch the branch to always draw the disk drive texture (our version #)
.org 0x801F1078
beql    t6, r0, title_screen_draw_dd

.org 0x801F108C
title_screen_draw_dd:

;move the version # texture to the bottom right
.org 0x801F11F4
addiu a3, a3, 0x60

.org 0x801F1204
addiu t0, t0, 0x72