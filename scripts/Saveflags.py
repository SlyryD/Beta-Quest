from Rom import Rom
from SaveContext import SaveContext

def default_save_flags(rom):
    save_context = SaveContext()
    
    # save_context.write_bits(0x00D4 + 0x03 * 0x1C + 0x04 + 0x0, 0x08) # Forest Temple switch flag (Poe Sisters cutscene)
    save_context.write_bits(0x00D4 + 0x05 * 0x1C + 0x04 + 0x1, 0x01) # Water temple switch flag (Ruto)
    save_context.write_bits(0x00D4 + 0x51 * 0x1C + 0x04 + 0x2, 0x08) # Hyrule Field switch flag (Owl)
    save_context.write_bits(0x00D4 + 0x55 * 0x1C + 0x04 + 0x0, 0x80) # Kokiri Forest switch flag (Owl)
    save_context.write_bits(0x00D4 + 0x56 * 0x1C + 0x04 + 0x2, 0x40) # Sacred Forest Meadow switch flag (Owl)
    save_context.write_bits(0x00D4 + 0x5B * 0x1C + 0x04 + 0x2, 0x01) # Lost Woods switch flag (Owl)
    save_context.write_bits(0x00D4 + 0x5B * 0x1C + 0x04 + 0x3, 0x80) # Lost Woods switch flag (Owl)
    save_context.write_bits(0x00D4 + 0x5C * 0x1C + 0x04 + 0x0, 0x80) # Desert Colossus switch flag (Owl)
    save_context.write_bits(0x00D4 + 0x5F * 0x1C + 0x04 + 0x3, 0x20) # Hyrule Castle switch flag (Owl)

    # save_context.write_bits(0x0ED4, 0x10) # "Met Deku Tree"
    # save_context.write_bits(0x0ED5, 0x20) # "Deku Tree Opened Mouth"
    # save_context.write_bits(0x0ED6, 0x08) # "Rented Horse From Ingo"
    # save_context.write_bits(0x0ED6, 0x10) # "Spoke to Mido After Deku Tree's Death"
    save_context.write_bits(0x0EDA, 0x08) # "Began Nabooru Battle"
    save_context.write_bits(0x0EDC, 0x80) # "Entered the Master Sword Chamber"
    # save_context.write_bits(0x0EDD, 0x20) # "Pulled Master Sword from Pedestal"
    save_context.write_bits(0x0EE0, 0x80) # "Spoke to Kaepora Gaebora by Lost Woods"
    # save_context.write_bits(0x0EE7, 0x20) # "Nabooru Captured by Twinrova"
    save_context.write_bits(0x0EE7, 0x10) # "Spoke to Nabooru in Spirit Temple"
    save_context.write_bits(0x0EED, 0x20) # "Sheik, Spawned at Master Sword Pedestal as Adult"
    save_context.write_bits(0x0EED, 0x01) # "Nabooru Ordered to Fight by Twinrova"
    # save_context.write_bits(0x0EED, 0x80) # "Watched Ganon's Tower Collapse / Caught by Gerudo"
    # save_context.write_bits(0x0EF9, 0x01) # "Greeted by Saria"
    save_context.write_bits(0x0F0A, 0x04) # "Spoke to Ingo Once as Adult"
    # save_context.write_bits(0x0F0F, 0x40) # "Met Poe Collector in Ruined Market"
    # save_context.write_bits(0x0F1A, 0x04) # "Met Darunia in Fire Temple"

    # save_context.write_bits(0x0ED7, 0x01) # "Spoke to Child Malon at Castle or Market"
    # save_context.write_bits(0x0ED7, 0x20) # "Spoke to Child Malon at Ranch"
    # save_context.write_bits(0x0ED7, 0x40) # "Invited to Sing With Child Malon"
    # save_context.write_bits(0x0F09, 0x10) # "Met Child Malon at Castle or Market"
    # save_context.write_bits(0x0F09, 0x20) # "Child Malon Said Epona Was Scared of You"

    save_context.write_bits(0x0F21, 0x04) # "Ruto in JJ (M3) Talk First Time"
    save_context.write_bits(0x0F21, 0x02) # "Ruto in JJ (M2) Meet Ruto"

    save_context.write_bits(0x0EE2, 0x01) # "Began Ganondorf Battle"
    save_context.write_bits(0x0EE3, 0x80) # "Began Bongo Bongo Battle"
    save_context.write_bits(0x0EE3, 0x40) # "Began Barinade Battle"
    save_context.write_bits(0x0EE3, 0x20) # "Began Twinrova Battle"
    save_context.write_bits(0x0EE3, 0x10) # "Began Morpha Battle"
    save_context.write_bits(0x0EE3, 0x08) # "Began Volvagia Battle"
    save_context.write_bits(0x0EE3, 0x04) # "Began Phantom Ganon Battle"
    save_context.write_bits(0x0EE3, 0x02) # "Began King Dodongo Battle"
    save_context.write_bits(0x0EE3, 0x01) # "Began Gohma Battle"

    save_context.write_bits(0x0EE8, 0x01) # "Entered Deku Tree"
    save_context.write_bits(0x0EE9, 0x80) # "Entered Temple of Time"
    save_context.write_bits(0x0EE9, 0x40) # "Entered Goron City"
    save_context.write_bits(0x0EE9, 0x20) # "Entered Hyrule Castle"
    save_context.write_bits(0x0EE9, 0x10) # "Entered Zora's Domain"
    save_context.write_bits(0x0EE9, 0x08) # "Entered Kakariko Village"
    save_context.write_bits(0x0EE9, 0x02) # "Entered Death Mountain Trail"
    save_context.write_bits(0x0EE9, 0x01) # "Entered Hyrule Field"
    save_context.write_bits(0x0EEA, 0x04) # "Entered Ganon's Castle (Exterior)"
    save_context.write_bits(0x0EEA, 0x02) # "Entered Death Mountain Crater"
    save_context.write_bits(0x0EEA, 0x01) # "Entered Desert Colossus"
    save_context.write_bits(0x0EEB, 0x80) # "Entered Zora's Fountain"
    save_context.write_bits(0x0EEB, 0x40) # "Entered Graveyard"
    save_context.write_bits(0x0EEB, 0x20) # "Entered Jabu-Jabu's Belly"
    save_context.write_bits(0x0EEB, 0x10) # "Entered Lon Lon Ranch"
    save_context.write_bits(0x0EEB, 0x08) # "Entered Gerudo's Fortress"
    save_context.write_bits(0x0EEB, 0x04) # "Entered Gerudo Valley"
    save_context.write_bits(0x0EEB, 0x02) # "Entered Lake Hylia"
    save_context.write_bits(0x0EEB, 0x01) # "Entered Dodongo's Cavern"
    save_context.write_bits(0x0F08, 0x08) # "Entered Hyrule Castle"


    save_context.write_save_table(rom)