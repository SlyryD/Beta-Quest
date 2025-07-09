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

;==================================================================================================
; Roll Collision / Bonks Kill Player
;==================================================================================================

; Set player health to zero on last frame of bonk animation
; z_player func_80844708, conditional where this->unk_850 != 0 and temp >= 0 || sp44
; Replaces:
;   or      a0, s0, $zero
;   jal     func_80838178
;   lw      a1, 0x0054($sp)
;   b       lbl_80842AE8
;   lw      $ra, 0x0024($sp)
;   lwc1    $f4, 0x0828(s0)
;   mtc1    $at, $f6
;   nop
.orga 0xBE0228
; Load APPLY_BONK_DAMAGE address as throwaway instructions. Replacing the jump call causes
; problems when overlay relocation is applied, breaking both replacement jump calls and nop'ing
; the instruction. By chance, these two instructions (equivalent to `la APPLY_BONK_DAMAGE`) do
; not crash after relocation, and so are kept here even though they do nothing.
    lui     t8, 0x8040
    addiu   t8, t8, 0x2D04
; Replace original function call with hook to apply damage if the setting is on.
; The original function is called in the new function before applying damage.
; Since the player actor always ends up in the same location in RAM, the jump
; address there is hardcoded.
    jal     BONK_LAST_FRAME
    lw      a1, 0x0054($sp)
; The branch address is shifted to an alternate location where lw $ra... is run.
; Required as la t8, APPLY_BONK_DAMAGE gets expanded to two commands.
    b       0xBE0494
    lw      $ra, 0x0024($sp)
    lwc1    $f4, 0x0828(s0)
    mtc1    $at, $f6

; Prevent set and reset of player state3 flag 4, which is re-used for storing bonk state if the
; player manages to cancel the roll/bonk animation before the last frame.
; The flag does not appear to be used by the vanilla game.
; Replaces:
;   sb      t4, 0x0682(s0)
.orga 0xBE3798
    nop
; Replaces:
;   sb      t5, 0x0682(s0)
.orga 0xBE55E4
    nop

; Hook to set flag if player starts bonk animation
; Flag is unset on player death
; Replaces:
;   or      a0, s0, $zero
;   addiu   a1, $zero, 0x00FF
.orga 0xBE035C
    jal     SET_BONK_FLAG
    nop

; Hook into Player_UpdateCommon to check if bonk animation was canceled.
; If so, kill the dirty cheater.
; Replaces:
;   addiu   $at, $zero, 0x0002
;   lui     t1, 0x8012
.orga 0xBE5328
    jal     CHECK_FOR_BONK_CANCEL
    nop

; Hook to Game Over cutscene init in Player actor to prevent adding a subcamera
; in scenes with fixed cameras like Link's House or outside Temple of Time.
; The game crashes in these areas if the cutscene subcamera to rotate around
; Link as he dies is added.
; Replaces
;   sll     a2, v0, 16
;   sra     a2, a2, 16
.orga 0xBD200C
    jal     CHECK_ROOM_MESH_TYPE
    nop

;==================================================================================================
; Roll Collision / Bonks Kills King Dodongo
;==================================================================================================

; King Dodongo tracks the number of wall hits when rolling in order to transition from
; the rolling animation to walking. Add a hook to the actor update function to check
; this variable and set actor health to zero if bonks are not zero.
; Replaces:
;   lh      t2, 0x0032(s1)
;   mtc1    $zero, $f16
.orga 0xC3DC04
    jal     KING_DODONGO_BONKS
    nop

;==================================================================================================
; Repoint English Message Table to JP
;==================================================================================================
;To make room for more message table entries, store the jp table pointer to the english one as well.
;The rest of this hack is implemented in Messages.py
; Replaces: sw      t7, 0x00(a1)
.orga 0xB575C8
    sw      t6, 0x00(a1)

; Dynamically load the en/jp message files for text lookup. Both files are utilized to make room
; for additional text. The jp file is filled first. The segment value for the requested text ID
; is used to manipulate the language bit to tell Message_OpenText (func_800DC838) which file
; to load and search. Hook at VRAM 0x800DCB60 in message.s
.orga 0xB52AC0
    jal     set_message_file_to_search
    nop

; The message lookup function uses a fixed reference to the first entry's segment to strip it from
; the combined segment/offset word. Since we have mixed segments in the table now, this is no
; longer safe. Load the correct segment into register a2 from the current message.
.orga 0xB4C980
    j       load_correct_message_segment
    nop

; Since message lookup already occurs in the above hook, remove the lookup from both the JP and EN
; branches.
.orga 0xB52AD0 ; JP branch
    nop
    nop
    nop
    nop
.orga 0xB52B64 ; EN branch
    nop
    nop

;=========================================================================================
; Add custom message control characters
;=========================================================================================

; In Message_Decode at the last control code check (0x01 for new line)
; Replaces
;   addiu   at, r0, 0x0001
;   bne     v0, at, 0x800DC580
.headersize (0x800110A0 - 0xA87000)
.org 0x800DC568
    j       Message_Decode_Control_Code_Hook
    nop

;================================================================================
; Reset choiceNum when decoding a new message
; prevents weird text alignment when going from message box with icon to no icon
;================================================================================
; Replaces sh   $zero, 0x4c0(at)
;          lhu  a3, 0x4c0(a3)
.org 0x800DA34C
    j       Message_Decode_reset_msgCtx.textPosX
    nop
