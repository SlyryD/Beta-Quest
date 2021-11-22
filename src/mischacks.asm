; Move fire/forest temple switches down 1 unit to make it easier to press
.orga 0x24860A8 :: .halfword 0xFCF4 ; forest basement 1
.orga 0x24860C8 :: .halfword 0xFCF4 ; forest basement 2
.orga 0x24860E8 :: .halfword 0xFCF4 ; forest basement 3
.orga 0x236C148 :: .halfword 0x1193 ; fire hammer room

; Allow Warp Songs in additional places
.orga 0xB6D3D2 :: .byte 0x00 ; Gerudo Training Ground
.orga 0xB6D42A :: .byte 0x00 ; Inside Ganon's Castle

; Allow Farore's Wind in dungeons where it's normally forbidden
.orga 0xB6D3D3 :: .byte 0x00 ; Gerudo Training Ground
.orga 0xB6D42B :: .byte 0x00 ; Inside Ganon's Castle

; Fix stupid alcove cameras in Ice Cavern -- thanks to krim and mzx for the help
.orga 0x2BECA25 :: .byte 0x01
.orga 0x2BECA2D :: .byte 0x01