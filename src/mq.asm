adjust_mq_room_allocation_size:
    li      t8, CFG_MASTER_QUEST
    lw      t8, 0x00(t8)
    addiu   t6, zero, 0x01
    bne     t8, t6, @@return
    nop
    addiu   s0, s0, 0x800

@@return:
    move    a1, s0
    jr      ra
    nop

adjust_mq_room_buffer_stride:
    addiu   t9, t2, 0x08
    li      t8, CFG_MASTER_QUEST
    lw      t8, 0x00(t8)
    addiu   t6, zero, 0x01
    bne     t8, t6, @@multiply
    nop
    addiu   t9, t9, 0x400

@@multiply:
    multu   t9, v1
    jr      ra
    nop
