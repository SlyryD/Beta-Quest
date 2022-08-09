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

;==================================================================================================
; Override Chest Speed
;==================================================================================================
.headersize 0
; Replaces:
;   bltz    t2, @@after_chest_speed_check
;   nop
.orga 0xBDA2EC ; 
    b     @@after_chest_speed_check
    nop
.skip 4 * 24
@@after_chest_speed_check:

;==================================================================================================
; Song of Storms Effect Trigger Changes
;==================================================================================================
; Allow a storm to be triggered with the song in any environment
; Replaces: lui     t5, 0x800F
;           lbu     t5, 0x1648(t5)
.orga 0xE6BF4C
    li      t5, 0
    nop

; Remove the internal cooldown between storm effects (to open grottos, grow bean plants...)
; Replaces: bnez     at, 0x80AECC6C
.orga 0xE6BEFC
    nop

;==================================================================================================
; Handle total small key count for the dungeon info menu
;==================================================================================================
; Replaces:
;   lui     t0, 0x8012                 # t0 = 80120000
;   addiu   t0, t0, 0xA5D0             # t0 = 8011A5D0
;   lhu     t9, 0x1402(t0)             # 8011B9D2
;   addiu   t1, r0, 0x0001             # t1 = 00000001
;   addiu   v0, r0, 0x00FF             # v0 = 000000FF
;   addu    v1, t0, t9
;   lb      a0, 0x00BC(v1)             # 000000BC
;   bgez    a0, lbl_80070198
;   addiu   t7, a0, 0x0001             # t7 = 00000001
;   sb      t1, 0x00BC(v1)             # 000000BC
.orga 0xAE60C8 ; In Memory: 0x80070168
    nop
    jal     give_small_key
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
