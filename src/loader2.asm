.org 0x8004BE30

.definelabel VROM_CODE, 0x3480000	// addr of code //MUST BE 0x01 0000 aligned!
.definelabel VROM_CODE_SIZE, 0x0B00	// MUST NOT BE GREATER THAN 0x7FFF!
.definelabel VROM_CODE_ADDR, 0x001DAFA0
.definelabel VROM_CODE_VADDR, 0x80000000 | VROM_CODE_ADDR

// a0 = Global Context

	addiu	$sp, $sp, 0xFFD0 //safety stack push //add immediate unsigned, sp = sp + FFD0
	sw  	a0, 0x0030($sp) //store word memory[sp+0x0030] = a0
	sw		$ra, 0x002C($sp) //store word memory[sp + 0x002C] = ra
	li		t0, VROM_CODE_VADDR //load immediate t0 = VROM_CODE_VADDR
	sw		t0, 0x0028($sp) //store word memory[sp+0x0028] = t0
	
	li  	a0, VROM_CODE_VADDR //load immediate a0 = VROM_CODE_VADDR
	li  	a1, VROM_CODE   //load immediate a1 = VROM_CODE
	jal 	0x0DF0  //jump and link 0x0DF0 (what's this?) 
	li		a2, VROM_CODE_SIZE  //load immediate a2 = VROM_CODE_SIZE
	
	lw 		a0, 0x0028($sp)     //load word a0 = memory[sp + 0x0028]
	jal 	0x3440 // osWritebackDCache
	li		a1, VROM_CODE_SIZE  //load immediate a1 = VROM_CODE_SIZE
	
	lw 		a0, 0x0028($sp)     //load word a0 = memory[sp + 0x0028]
	jal		0x41A0 // osInvalICache 
	li		a1, VROM_CODE_SIZE  //load immediate a1 = VROM_CODE_SIZE
	
	jal		VROM_CODE_ADDR  //jump and link VROM_CODE_ADDR
	lw  	a0, 0x0030($sp) //load word a0 = memory[sp + 0x0030]
	
	lw  	a0, 0x0030($sp) //load word a0 = memory[sp + 0x0030] // why do we need this twice?
	lw		$ra, 0x002C($sp)    //load word ra = memory[sp + 0x002C]
	addiu   $sp, $sp, 0x30      //add immediate unsigned sp = sp + 0x30
// hook back
	addiu   $sp, $sp, 0xFF70    //add immediate unsigned sp = sp + 0xFF70
	j		0x9A758             //jump 0x9A758
	sw  	s2, 0x0028($sp)     //store word memory[sp + 0x0028] = s2
	
// for convenience, here's the hook
//	j		0x4BE30



