from Patches.Rom import Rom
from Patches.SaveContext import SaveContext

def default_save_flags(rom):
    save_context = SaveContext()
    
    save_context.write_bits(0x0EE9, 0x40) # "Entered Goron City"
    save_context.write_bits(0x00D4 + 0x5F * 0x1C + 0x04 + 0x3, 0x20) # Hyrule Castle switch flag (Owl)
    save_context.write_bits(0x0EDA, 0x08) # "Began Nabooru Battle"


    save_context.write_save_table(rom)