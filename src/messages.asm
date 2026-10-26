; Manipulates the save context language bit based
; on the segment value for the requested text ID.
; Message lookup is moved here from the vanilla
; branches.
; t6 used for branching to JP/EN file loading
; in parent function func_800DC838 at VRAM 0x800DCB68
; 0 == JP
; 1 == EN
set_message_file_to_search:
    ; displaced code
    lhu     a1, 0x0046($sp)
    lw      a0, 0x0040($sp)

    ; Saved by vanilla code prior to jump to
    ; message lookup function. Called here so
    ; we can safely store $ra.
    sw      t0, 0x002C($sp)

    la      t0, message_lookup_return_address
    sw      $ra, 0x0000(t0)

    addiu   t0, r0, 0x00B4
    beq     a1, t0, @dungeon_token_message
    nop
    addiu   t0, r0, 0x00B5
    beq     a1, t0, @area_token_message
    nop
    addiu   t0, r0, 0x00C2
    beq     a1, t0, @heart_piece_message
    nop
    addiu   t0, r0, 0x00C3
    beq     a1, t0, @heart_piece_message
    nop
    addiu   t0, r0, 0x00C4
    beq     a1, t0, @heart_piece_message
    nop
    addiu   t0, r0, 0x00C5
    beq     a1, t0, @heart_piece_message
    nop
    li      t0, 0x90C2
    beq     a1, t0, @heart_piece_message
    nop
    addiu   t0, r0, 0x00FA
    beq     a1, t0, @winner_heart_piece_message
    nop
    addiu   t0, r0, 0x00FB
    beq     a1, t0, @winner_heart_piece_message
    nop
    addiu   t0, r0, 0x00FC
    beq     a1, t0, @winner_heart_piece_message
    nop
    addiu   t0, r0, 0x00FD
    beq     a1, t0, @winner_heart_piece_message
    nop
    addiu   t0, r0, 0x0060
    beq     a1, t0, @small_key_message
    nop
    addiu   t0, r0, 0x0066
    beq     a1, t0, @map_message
    nop
    addiu   t0, r0, 0x0067
    beq     a1, t0, @compass_message
    nop
    addiu   t0, r0, 0x00C7
    bne     a1, t0, @lookup_message
    nop
    la      t2, boss_key_message_ids
    b       @remap_dungeon_message
    nop

@small_key_message:
    la      t2, small_key_message_ids
    b       @remap_dungeon_message
    nop

@dungeon_token_message:
    la      t2, dungeon_token_message_ids
    li      t3, 0x000A
    b       @remap_bounded_dungeon_message
    nop

@area_token_message:
    jal     get_current_area_index
    nop
    la      t2, area_token_message_ids
    b       @remap_area_message
    nop

@heart_piece_message:
    li      t3, 0x912F
    b       @select_heart_piece_location
    nop

@winner_heart_piece_message:
    li      a1, 0x9128
    b       @lookup_message
    nop

@select_heart_piece_location:
    li      t0, SAVE_CONTEXT
    lbu     t0, 0x1403(t0)
    addiu   t1, r0, 0x0009
    beq     t0, t1, @lookup_dungeon_heart_piece_message
    nop
    jal     get_current_area_index
    nop
    la      t2, area_heart_piece_message_ids
    b       @remap_area_message
    nop

@lookup_dungeon_heart_piece_message:
    or      a1, t3, r0
    b       @lookup_message
    nop

@map_message:
    la      t2, map_message_ids
    b       @remap_dungeon_message
    nop

@compass_message:
    la      t2, compass_message_ids

@remap_dungeon_message:
    li      t3, 0x0011

@remap_bounded_dungeon_message:
    li      t0, SAVE_CONTEXT
    lbu     t0, 0x1403(t0)
    sltu    t1, t0, t3
    beqz    t1, @lookup_message
    sll     t0, t0, 1

    addu    t1, t2, t0
    lhu     a1, 0x0000(t1)

    b       @lookup_message
    nop

@remap_area_message:
    sltiu   t1, v0, 0x000C
    beqz    t1, @lookup_message
    sll     t0, v0, 1

    addu    t1, t2, t0
    lhu     a1, 0x0000(t1)

@lookup_message:
    ; call JP message lookup function exclusively
    ; since the JP/EN tables are merged.
    jal     0x800D69EC
    nop

    ; a1 contains the segment/offset word, formatted as
    ; ssoooooo
    ; where "s" is the segment and "o" is the offset.
    ; Vanilla crashes on a failed text lookup already,
    ; so we don't need to worry about bad values.
    srl     t0, a1, 0x18
    andi    t0, t0, 0x08
    sltiu   t6, t0, 0x0001

    la      t0, message_lookup_return_address
    lw      $ra, 0x0000(t0)
    jr      $ra
    nop

.align 2
dungeon_token_message_ids:
    .halfword 0x9101, 0x9102, 0x9103, 0x9104, 0x9105
    .halfword 0x9106, 0x9107, 0x9108, 0x9109, 0x910A

area_token_message_ids:
    .halfword 0x910B, 0x910C, 0x910D, 0x910E
    .halfword 0x910F, 0x9110, 0x9111, 0x9112
    .halfword 0x9113, 0x9114, 0x9115, 0x9116

area_heart_piece_message_ids:
    .halfword 0x9117, 0x9118, 0x9119, 0x911A
    .halfword 0x911B, 0x911C, 0x911D, 0x911E
    .halfword 0x911F, 0x9120, 0x9121, 0x9122

small_key_message_ids:
    .halfword 0x0060, 0x0060, 0x0060, 0x921A
    .halfword 0x921B, 0x921C, 0x921D, 0x921E
    .halfword 0x921F, 0x0060, 0x0060, 0x9220
    .halfword 0x9221, 0x9222, 0x0060, 0x0060
    .halfword 0x00F3

map_message_ids:
    .halfword 0x9210, 0x9211, 0x9212, 0x9213
    .halfword 0x9214, 0x9215, 0x9216, 0x9217
    .halfword 0x9218, 0x9219, 0x0066, 0x0066
    .halfword 0x0066, 0x0066, 0x0066, 0x0066
    .halfword 0x0066

compass_message_ids:
    .halfword 0x9206, 0x9207, 0x9208, 0x9209
    .halfword 0x920A, 0x920B, 0x920C, 0x920D
    .halfword 0x920E, 0x920F, 0x0067, 0x0067
    .halfword 0x0067, 0x0067, 0x0067, 0x0067
    .halfword 0x0067

boss_key_message_ids:
    .halfword 0x00C7, 0x00C7, 0x00C7, 0x9200
    .halfword 0x9201, 0x9202, 0x9203, 0x9204
    .halfword 0x00C7, 0x00C7, 0x00C7, 0x00C7
    .halfword 0x00C7, 0x9205, 0x00C7, 0x00C7
    .halfword 0x00C7

.align 4
message_lookup_return_address:
    .word 0x00000000

load_correct_message_segment:
    ; displaced code
    lbu     t6, 0x0002(v1)
    lw      a1, 0x0004(v1)

    ; shift out the offset bits
    srl     a2, a1, 0x18
    sll     a2, a2, 0x18

    ; Remaining code from func_800D69EC since
    ; it doesn't save $ra
    lw      a1, 0x0004(v1)
    addiu   v0, a0, 0x2200
    sb      t6, 0x0008(v0)
    lw      a3, 0x000C(v1)
    subu    t7, a1, a2
    addiu   v1, v1, 0x0008
    subu    t8, a3, a1
    sw      t7, 0x0000(v0)
    jr      $ra
    sw      t8, 0x0004(v0)

Message_Decode_Control_Code_Hook:
; Replaced code
    addiu   at, r0, 0x0001
    bne     v0, at, @not_newline
    lh      t6, 0x008C(sp)
    addiu   t4, t6, 0x0001
    j       0x800DC7E4
    sh      t4, 0x008C(sp)

; New Control Codes go here

; V0 = Current character
; 0x54(sp) = MessageContext
; s5 = decodedBufPos
; s4 = charTexIdx
@not_newline:
    addiu   sp, sp, -0x60
    sw      a0, 0x10(sp)
    sw      a1, 0x14(sp)
    sw      a2, 0x18(sp)
    sw      v0, 0x1C(sp)
    sw      a3, 0x20(sp)

    addiu   sp, sp, -0x20
    sw      s5, 0x10(sp) ; store decoded buffer position on the stack
    sw      s4, 0x14(sp) ; store charTexIdx on the stack
;   lw      a0, 0xD4(sp) ; load message context pointer into a0
    or      a0, r0, v0 ; load current character into a1
    addiu   a1, sp, 0x10
    addiu   a2, sp, 0x14
    jal     Message_Decode_Additional_Control_Codes
    nop
    lw      s5, 0x10(sp) ; store decoded buffer position back into s5
    lw      s4, 0x14(sp)
    addiu   sp, sp, 0x20

    lw      a0, 0x10(sp)
    lw      a1, 0x14(sp)
    lw      a2, 0x18(sp)
    lw      a3, 0x20(sp)

    beqz    v0, @no_match
    lw      v0, 0x1C(sp)

    j       0x800DC7E4
    addiu   sp, sp, 0x60

@no_match:
    j      0x800DC580
    addiu   sp, sp, 0x60

Message_Decode_reset_msgCtx.textPosX:
    sb      $zero, 0x4BE(at)
    sh      $zero, 0x4C0(at)
    lhu     a3, 0x4C0(a3)
    j       0x800DA354
    nop
