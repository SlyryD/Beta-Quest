;==================================================================================================
; main.c hooks
;==================================================================================================
.headersize (0x800110A0 - 0xA87000)

; Runs before the game state update function
; Replaces:
;   lw      t6, 0x0018 (sp)
;   lui     at, 0x8010
.org 0x8009CAD4
    jal     before_game_state_update_hook
    nop

; Runs after the game state update function
; Replaces:
;   jr      ra
;   nop
.org 0x8009CB00
    j       after_game_state_update
    nop

; Runs before skybox init
.org 0x8009CED0
    jal     before_skybox_init

.org 0x8009CDA0
Gameplay_InitSkybox:

; Runs after scene init
; Replaces:
;   jr      ra
;   nop
.org 0x8009CEE4
    j       after_scene_init
    nop

;==================================================================================================
; World Map Hacks
;==================================================================================================

; Draws all world map points
; Replaces:
; lbl_BC4368:
;   addu    t8, s0, v1
;   addiu   v1, v1, 0x0001
;   andi    v1, v1, 0xFFFF
;   slti    at, v1, 0x0014
;   bne     at, r0, lbl_BC4368
;   sb      r0, 0x0266(t8)
; 0xBB11E0 + 0x13178
.orga 0xBC4358
    jal     set_world_map_points
    nop
    nop
    nop
    nop
    nop

;==================================================================================================
; DPAD Display
;==================================================================================================
;
; Replaces lw    t6, 0x1C44(s6)
;          lui   t8, 0xDB06
.orga 0xAEB67C ; In Memory: 0x8007571C
    jal     dpad_draw
    nop

;==================================================================================================
; Initial save - from rando
;==================================================================================================
; Replaces:
;   sb      t0, 32(s1)
;   sb      a1, 33(s1)
.orga 0xB06C2C ; In memory: ???
    jal     write_initial_save
    sb      t0, 32(s1)