; Speed scene after Dodongo's Cavern
.orga 0x2221E88 :: .word 0x000C003B, 0x003C003C
.orga 0x2223308 :: .word 0x00810000, 0x003A0000

; Speed scene after Jabu Jabu's Belly
.orga 0xCA3530 :: .word 0x00000000
.orga 0x2113340 :: .word 0x000D003B, 0x003C003C
.orga 0x2113C18 :: .word 0x00820000, 0x003A0000
.orga 0x21131D0 :: .word 0x00010000, 0x003C003C

; Speed scene after Forest Temple
.orga 0xD4ED68 :: .word 0x0045003B, 0x003C003C
.orga 0xD4ED78 :: .word 0x003E0000, 0x003A0000
.orga 0x207B9D4 :: .word 0xFFFFFFFF

; Speed scene after Fire Temple
.orga 0x2001848 :: .word 0x001E0001, 0x00020002
.orga 0xD100B4 :: .word 0x0062003B, 0x003C003C
.orga 0xD10134 :: .word 0x003C0000, 0x003A0000

; Speed scene after Water Temple
.orga 0xD5A458 :: .word 0x0015003B, 0x003C003C
.orga 0xD5A3A8 :: .word 0x003D0000, 0x003A0000
.orga 0x20D0D20 :: .word 0x002900C7, 0x00C800C8

; Speed scene after Shadow Temple
.orga 0xD13EC8 :: .word 0x0061003B, 0x003C003C
.orga 0xD13E18 :: .word 0x00410000, 0x003A0000

; Speed scene after Spirit Temple
.orga 0xD3A0A8 :: .word 0x0060003B, 0x003C003C
.orga 0xD39FF0 :: .word 0x003F0000, 0x003A0000

; Speed Phantom Ganon defeat scene
.orga 0xC944D8 :: .word 0x00000000
.orga 0xC94548 :: .word 0x00000000
.orga 0xC94730 :: .word 0x00000000
.orga 0xC945A8 :: .word 0x00000000
.orga 0xC94594 :: .word 0x00000000

; Speed up the Nabooru defeat cutscene
.orga 0x2F5AF84 :: .word 0x00000005
.orga 0x2F5C7DA :: .word 0x00010002
.orga 0x2F5C7A2 :: .word 0x00030004
.orga 0x2F5B369 :: .byte 0x09
.orga 0x2F5B491 :: .byte 0x04
.orga 0x2F5B559 :: .byte 0x04
.orga 0x2F5B621 :: .byte 0x04
.orga 0x2F5B761 :: .byte 0x07
.orga 0x2F5B840 :: .word 0x00050001, 0x00050005 ; shorten white flash

; Speed up twinrova defeat cutscene
.orga 0xD678CC :: .word 0x240103A2, 0xA6010142
.orga 0xD67BA4 :: .halfword 0x10, 0x00

; Speed up Zelda descending after defeating Ganondorf
.orga 0xD82AB3 :: .byte 0x66
.orga 0xD82FAF :: .byte 0x65
.orga 0xD82D2E :: .halfword 0x041F
.orga 0xD83142 :: .halfword 0x006B
.orga 0xD82DD8 :: .word 0x00000000
.orga 0xD82ED4 :: .word 0x00000000
.orga 0xD82FDF :: .byte 0x33

; Speed completion of the trials in Ganon's Castle
.orga 0x31A8090 :: .halfword 0x006B, 0x0001, 0x0002, 0x0002 ; Forest
.orga 0x31A9E00 :: .halfword 0x006E, 0x0001, 0x0002, 0x0002 ; Fire
.orga 0x31A8B18 :: .halfword 0x006C, 0x0001, 0x0002, 0x0002 ; Water
.orga 0x31A9430 :: .halfword 0x006D, 0x0001, 0x0002, 0x0002 ; Shadow
.orga 0x31AB200 :: .halfword 0x0070, 0x0001, 0x0002, 0x0002 ; Spirit
.orga 0x31AA830 :: .halfword 0x006F, 0x0001, 0x0002, 0x0002 ; Light

; Speed obtaining Fairy Ocarina
.orga 0x2151230 :: .word 0x0072003C, 0x003D003D
.orga 0x2151240 :: .word 0x004A0000, 0x003A0000, 0xFFFFFFFF, 0xFFFF003C, 0x0081FFFF
.orga 0x2150E20 :: .word 0xFFFFFA4C

; Remove remaining owls - Note: Might be good to label these
.orga 0x1FE30CE :: .byte 0x01, 0x4B
.orga 0x1FE30DE :: .byte 0x01, 0x4B
.orga 0x1FE30EE :: .byte 0x01, 0x4B
.orga 0x205909E :: .byte 0x00, 0x3F
.orga 0x2059094 :: .byte 0x80

; Speed up opening the royal tomb for both child and adult
.orga 0x2025026 :: .byte 0x00, 0x01
.orga 0x2023C86 :: .byte 0x00, 0x01
.orga 0x2025159 :: .byte 0x02
.orga 0x2023E19 :: .byte 0x02

; Speed opening of Door of Time
.orga 0xE0A176 :: .byte 0x00, 0x02
.orga 0xE0A35A :: .byte 0x00, 0x01, 0x00, 0x02

; Speed up owl flights
.orga 0x20E60D2 :: .byte 0x00, 0x01 ; Lake Hylia
.orga 0x223B6B2 :: .byte 0x00, 0x01 ; Death Mountain Trail

; Remove disruptive text from Gerudo Training Ground and early Shadow Temple (vanilla)
.orga 0x27C00BC :: .byte 0xFB
.orga 0x27C00CC :: .byte 0xFB
.orga 0x27C00DC :: .byte 0xFB
.orga 0x27C00EC :: .byte 0xFB
.orga 0x27C00FC :: .byte 0xFB
.orga 0x27C010C :: .byte 0xFB
.orga 0x27C011C :: .byte 0xFB
.orga 0x27C012C :: .byte 0xFB
.orga 0x27CE080 :: .byte 0xFB
.orga 0x27CE090 :: .byte 0xFB
.orga 0x2887070 :: .byte 0xFB
.orga 0x2887080 :: .byte 0xFB
.orga 0x2887090 :: .byte 0xFB
.orga 0x2897070 :: .byte 0xFB
.orga 0x28C7134 :: .byte 0xFB
.orga 0x28D91BC :: .byte 0xFB
.orga 0x28A60F4 :: .byte 0xFB
.orga 0x28AE084 :: .byte 0xFB
.orga 0x28B9174 :: .byte 0xFB
.orga 0x28BF168 :: .byte 0xFB
.orga 0x28BF178 :: .byte 0xFB
.orga 0x28BF188 :: .byte 0xFB
.orga 0x28A1144 :: .byte 0xFB
.orga 0x28A6104 :: .byte 0xFB
.orga 0x28D0094 :: .byte 0xFB







; TODO save context from lines 910 to 977 from Patches.py


; Not ported
; anything before the next one
; Speed scene after Deku Tree
; Darunia won't dance
; Zora moves quickly
; Speed Jabu Jabu swallowing Link
; Ruto no longer points to Zora Sapphire
; Speed up Epona race start
; Speed up Epona escape
; Speed up draining the well
; Poacher's Saw no longer messes up Forest Stage
; Tell Sheik at Ice Cavern we are always an Adult