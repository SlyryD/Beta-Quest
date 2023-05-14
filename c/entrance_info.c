#include "entrance_info.h"

#include "gfx.h"
#include "text.h"
#include "util.h"
#include "z64.h"

typedef enum display_mode_t {
    DISPLAYMODE_NONE = 0,
    DISPLAYMODE_BY_VISITED,
    DISPLAYMODE_BY_VISITED_DEST,
    DISPLAYMODE_ALL,
    DISPLAYMODE_ALL_DEST,
    DISPLAYMODE_LAST,
} DisplayMode;

#define GAMEMODE_NORMAL 0
#define TITLE_SCREEN 1
#define FILE_SELECT 2

#define NUM_LINES 3

#define PAUSE_QUEST 2

#define NO_RAND (0x801C6E60 + 0x30 + 0x15D4)
#define PREV_ENTS (NO_RAND + 4)
#define FAIRY_CUSTOM_ENT (PREV_ENTS + 12)
#define VAR_SPACE (FAIRY_CUSTOM_ENT + 4)

// Must be the same as in the asm!
static short VAR_Entrances[] = { 0x0371, 0x04BE, 0x0588, 0x04C2, 0x0578, 0x0394, 0x0482, 0x057C, 0x0340, 0x0000, 0x0004, 0x0008, 0x000C, 0x0010, 0x0028, 0x002D, 0x0033, 0x0037, 0x003B, 0x0043, 0x004B, 0x004F, 0x0053, 0x0063, 0x0067, 0x0072, 0x007A, 0x007E, 0x0082, 0x0088, 0x008D, 0x0098, 0x009C, 0x00AD, 0x00B1, 0x00B7, 0x00BB, 0x00C1, 0x00C5, 0x00C9, 0x00CD, 0x00DB, 0x00E4, 0x00EA, 0x00FC, 0x0102, 0x0108, 0x010E, 0x0117, 0x011E, 0x0123, 0x0129, 0x0130, 0x0134, 0x0138, 0x013D, 0x0147, 0x014D, 0x0157, 0x0165, 0x0169, 0x016D, 0x0171, 0x0179, 0x017D, 0x0181, 0x0185, 0x0189, 0x018D, 0x0191, 0x0195, 0x019D, 0x01A1, 0x01A9, 0x01B5, 0x01B9, 0x01BD, 0x01C1, 0x01C9, 0x01CD, 0x01D1, 0x01D5, 0x01D9, 0x01DD, 0x01E1, 0x01E5, 0x01E9, 0x01F1, 0x01F9, 0x01FD, 0x0201, 0x0205, 0x0209, 0x020D, 0x0211, 0x0215, 0x0219, 0x021D, 0x0221, 0x0225, 0x022D, 0x0231, 0x0235, 0x0239, 0x023D, 0x0242, 0x0246, 0x024A, 0x0252, 0x0256, 0x025A, 0x025E, 0x0262, 0x0266, 0x026A, 0x026E, 0x0272, 0x0276, 0x027E, 0x0286, 0x028A, 0x028E, 0x0292, 0x0296, 0x029A, 0x029E, 0x02A2, 0x02A6, 0x02AA, 0x02BA, 0x02BE, 0x02C2, 0x02C6, 0x02CA, 0x02D2, 0x02D6, 0x02DA, 0x02DE, 0x02F9, 0x02FD, 0x0301, 0x0305, 0x0309, 0x030D, 0x0311, 0x031C, 0x0328, 0x032C, 0x0330, 0x0334, 0x033C, 0x0345, 0x0349, 0x034D, 0x0351, 0x0355, 0x0359, 0x035D, 0x0361, 0x0365, 0x0378, 0x037C, 0x0380, 0x0384, 0x0388, 0x038C, 0x0398, 0x039C, 0x03A0, 0x03A4, 0x03A8, 0x03AC, 0x03B0, 0x03B8, 0x03BC, 0x03C0, 0x03C4, 0x03CC, 0x03D0, 0x03D4, 0x03DC, 0x03E0, 0x03E4, 0x03EC, 0x03F0, 0x03F4, 0x03FC, 0x0400, 0x040B, 0x040F, 0x0413, 0x0417, 0x041B, 0x041F, 0x042F, 0x0433, 0x0437, 0x043B, 0x043F, 0x0443, 0x0447, 0x044B, 0x044F, 0x0453, 0x045F, 0x0463, 0x0467, 0x0472, 0x0476, 0x047E, 0x0486, 0x048A, 0x048E, 0x0492, 0x0496, 0x049A, 0x049E, 0x04A2, 0x04A6, 0x04AA, 0x04AE, 0x04B2, 0x04B6, 0x04BA, 0x04D6, 0x04DA, 0x04DE, 0x04E2, 0x04E6, 0x04EE, 0x04F6, 0x04FA, 0x04FF, 0x0503, 0x0507, 0x050B, 0x050F, 0x051C, 0x0524, 0x0528, 0x052C, 0x0530, 0x0534, 0x0538, 0x053C, 0x0540, 0x0544, 0x0548, 0x054C, 0x0550, 0x0554, 0x0560, 0x0564, 0x0568, 0x056C, 0x0570, 0x0580, 0x05C8, 0x05D0, 0x05D4, 0x05DC, 0x05E0, 0x05E4, 0x05F4, 0x0600, 0x0604, 0x0608, 0x060C, 0x0610, 0x0598, 0x059C, 0x05A0, 0x05A4, 0x05A8, 0x05AC, 0x05B0, 0x05B4, 0x05B8, 0x05BC, 0x05C0, 0x05C4, 0x05FC, 0x003F, 0x036D, 0x00EE, 0x0153, 0x0175, 0x01A5, 0x01AD, 0x01ED, 0x01F5, 0x024E, 0x02B2, 0x02CE, 0x02E2, 0x02E6, 0x02F5, 0x0315, 0x0320, 0x0324, 0x0390, 0x03B4, 0x0423, 0x0457, 0x045B, 0x047A, 0x04C6, 0x0513, 0x0558, 0x055C, 0x0574, 0x0584, 0x058C, 0x0590, 0x0594, 0x05CC, 0x05EC, 0x05F8, 0x00A0, 0x006B, 0x02AE, 0x04CA, 0x04CE, 0x0517, 0x05E8, 0x05F0};

// pointer to the shuffled entrances
static short *ptr_VAR_SPACE = ((short*)VAR_SPACE);

typedef struct entrance_table_entry {
    short entrance;
    char *name;
    char *fromName;
} entrance_table_entry_t;

// Keep this alphabetically ordered!
entrance_table_entry_t grotto_table[] =
{
    { 0x01F5, "DMC Bomb Grotto", 0},
    { 0x01ED, "DMT Storms Grotto", 0},
    { 0x02B2, "GF Storms Grotto", 0},
    { 0x02E6, "HF Across River Bomb Grotto", 0},
    { 0x003F, "HF Bomb Grotto near Market Entrance", 0 },
    { 0x00EE, "HF Open Grotto", 0},
    { 0x036D, "HF Remote Bomb Grotto", 0},
    { 0x0153, "Kak Open Grotto", 0},
    { 0x01A5, "KF Storms Grotto", 0},
    { 0x01AD, "LW Bomb Grotto", 0},
    { 0x02CE, "SFM Storms Grotto", 0},
    { 0x024E, "ZD Storms Grotto", 0},
    { 0x02E2, "ZR Bomb Grotto", 0},
    { 0x0175, "ZR Open Grotto", 0},
};

// Keep this alphabetically ordered!
entrance_table_entry_t entrance_table_dest[] =
{
    { 0x05BC, "2 Scrub Grotto", 0},
    { 0x05A4, "3 Scrub Grotto", 0},
    { 0x0301, "Barinade Boss Room", 0},
    { 0x0413, "Bongo Bongo Boss Room", 0},
    { 0x0098, "Bottom of the Well", 0},
    { 0x007A, "Castle Courtyard", 0},
    { 0x0296, "Castle Courtyard", 0},
    { 0x0138, "Castle Grounds", 0},
    { 0x0340, "Castle Grounds", 0},
    { 0x047E, "Castle Grounds", 0},
    { 0x04FA, "Castle Grounds", 0},
    { 0x023D, "Castle Grounds From Ganons Castle", 0},
    { 0x0588, "Colossus GFF", 0},
    { 0x013D, "Death Mountain", 0},
    { 0x01B9, "Death Mountain", 0},
    { 0x0242, "Death Mountain", 0},
    { 0x01BD, "Death Mountain Summit", 0},
    { 0x05C4, "Deku Theater", "Leaving Deku Theather"},
    { 0x0252, "Deku Tree Boss Door", 0},
    { 0x0000, "Deku Tree Lobby", 0},
    { 0x0123, "Desert Colossus", 0},
    { 0x01F1, "Desert Colossus", 0},
    { 0x057C, "Desert Colossus", 0},
    { 0x01E1, "Desert Colossus From Spirit Lobby", 0},
    { 0x0610, "Desert Colossus From Spirit Lobby", 0},
    { 0x01E9, "Desert Colossus Mirror Shield Chest", 0},
    { 0x01E5, "Desert Colossus Silver Gauntlets Chest", 0},
    { 0x04F6, "DMC Central Local", 0},
    { 0x024A, "DMC Fire Temple Entrance", 0},
    { 0x0564, "DMC Fire Temple Entrance", 0},
    { 0x04BE, "DMC GFF", 0},
    { 0x0246, "DMC Lower Local", 0},
    { 0x0482, "DMC Lower Local", 0},
    { 0x0147, "DMC Upper Local", 0},
    { 0x05FC, "DMT Cow Grotto", "Leaving DMT Cow Grotto"},
    { 0x0004, "Dodongos Cavern Beginning", 0},
    { 0x00C5, "Dodongos Cavern Boss Door", 0},
    { 0x0398, "Dog Lady House", 0},
    { 0x036D, "Fairy Grotto", 0},
    { 0x0165, "Fire Temple Lower", 0},
    { 0x0169, "Forest Temple Lobby", 0},
    { 0x041F, "Ganondorf Boss Room", 0},
    { 0x0467, "Ganons Castle Lobby", 0},
    { 0x0534, "Ganons Castle Lobby", 0},
    { 0x0538, "Ganons Castle Lobby", 0},
    { 0x053C, "Ganons Castle Lobby", 0},
    { 0x0540, "Ganons Castle Lobby", 0},
    { 0x0544, "Ganons Castle Lobby", 0},
    { 0x0548, "Ganons Castle Lobby", 0},
    { 0x054C, "Ganons Castle Lobby", 0},
    { 0x041B, "Ganons Tower Bottom", 0},
    { 0x01C1, "GC Darunias Chamber", 0},
    { 0x037C, "GC Shop", 0},
    { 0x04E2, "GC Woods Warp", 0},
    { 0x003F, "Generic Grotto", 0},
    { 0x0129, "Gerudo Fortress", 0},
    { 0x03A8, "Gerudo Fortress", 0},
    { 0x03B0, "Gerudo Fortress", 0},
    { 0x02C2, "Gerudo Fortress Above GTG Center", 0},
    { 0x02AA, "Gerudo Fortress Above GTG Left", 0},
    { 0x02C6, "Gerudo Fortress Above Vines Back", 0},
    { 0x02D2, "Gerudo Fortress Above Vines Left", 0},
    { 0x02DE, "Gerudo Fortress Balcony", 0},
    { 0x0239, "Gerudo Fortress Behind Crates Center", 0},
    { 0x0235, "Gerudo Fortress Behind Crates Left", 0},
    { 0x0231, "Gerudo Fortress Bottom Left", 0},
    { 0x02DA, "Gerudo Fortress Left Alcove", 0},
    { 0x02BA, "Gerudo Fortress Left of GTG", 0},
    { 0x02BE, "Gerudo Fortress Mid Across from Vines", 0},
    { 0x03A4, "Gerudo Fortress Near Top", 0},
    { 0x02D6, "Gerudo Fortress Top", 0},
    { 0x0008, "Gerudo Training Ground Lobby", 0},
    { 0x0117, "Gerudo Valley", 0},
    { 0x03AC, "GF Outside Gate", 0},
    { 0x014D, "Goron City", 0},
    { 0x03FC, "Goron City", 0},
    { 0x00E4, "Graveyard", 0},
    { 0x0355, "Graveyard", 0},
    { 0x0359, "Graveyard", 0},
    { 0x035D, "Graveyard", 0},
    { 0x0361, "Graveyard", 0},
    { 0x050B, "Graveyard", 0},
    { 0x044F, "Graveyard Dampes Grave", 0},
    { 0x030D, "Graveyard Dampes House", 0},
    { 0x031C, "Graveyard Heart Piece Grave", 0},
    { 0x002D, "Graveyard Royal Familys Tomb", 0},
    { 0x004B, "Graveyard Shield Grave", 0},
    { 0x0205, "Graveyard Warp Pad Region", 0},
    { 0x0568, "Graveyard Warp Pad Region", 0},
    { 0x0580, "Graveyard Warp Pad Region", 0},
    { 0x03A0, "GV Carpenter Tent", 0},
    { 0x022D, "GV Fortress Side", 0},
    { 0x03D0, "GV Fortress Side", 0},
    { 0x05AC, "GV Octorok Grotto", "Leaving GV Grotto Ledge"},
    { 0x0578, "HC GFF", 0},
    { 0x05B8, "HC Storms Grotto", "Leaving HC Storms Grotto"},
    { 0x05A8, "HF Cow Grotto", "Leaving HF Cow Grotto"},
    { 0x059C, "HF Inside Fence Grotto", "Leaving HF Inside Fence Grotto"},
    { 0x0598, "HF Near Kak Grotto", "Leaving HF Near Kak Grotto"},
    { 0x05C0, "HF Tektite Grotto", "Leaving HF Tektite Grotto"},
    { 0x00CD, "Hyrule Field", 0},
    { 0x017D, "Hyrule Field", 0},
    { 0x0181, "Hyrule Field", 0},
    { 0x0185, "Hyrule Field", 0},
    { 0x0189, "Hyrule Field", 0},
    { 0x018D, "Hyrule Field", 0},
    { 0x01F9, "Hyrule Field", 0},
    { 0x01FD, "Hyrule Field", 0},
    { 0x027E, "Hyrule Field", 0},
    { 0x028A, "Hyrule Field", 0},
    { 0x028E, "Hyrule Field", 0},
    { 0x0292, "Hyrule Field", 0},
    { 0x0311, "Hyrule Field", 0},
    { 0x0476, "Hyrule Field", 0},
    { 0x050F, "Hyrule Field after Ocarina of Time", 0},
    { 0x0088, "Ice Cavern Beginning", 0},
    { 0x0028, "Jabu Jabus Belly Beginning", 0},
    { 0x00DB, "Kakariko Village", 0},
    { 0x0195, "Kakariko Village", 0},
    { 0x0201, "Kakariko Village", 0},
    { 0x02A6, "Kakariko Village", 0},
    { 0x0345, "Kakariko Village", 0},
    { 0x0349, "Kakariko Village", 0},
    { 0x0351, "Kakariko Village", 0},
    { 0x044B, "Kakariko Village", 0},
    { 0x0463, "Kakariko Village", 0},
    { 0x04EE, "Kakariko Village", 0},
    { 0x034D, "Kak Backyard", 0},
    { 0x04FF, "Kak Backyard", 0},
    { 0x00B7, "Kak Bazaar", 0},
    { 0x0191, "Kak Behind Gate", 0},
    { 0x02FD, "Kak Carpenter Boss House", 0},
    { 0x0550, "Kak House of Skulltula", 0},
    { 0x039C, "Kak Impas House", 0},
    { 0x05C8, "Kak Impas House Back", 0},
    { 0x05DC, "Kak Impas Ledge", 0},
    { 0x0554, "Kak Impas Rooftop", 0},
    { 0x0072, "Kak Odd Medicine Building", 0},
    { 0x03EC, "Kak Potion Shop Back", 0},
    { 0x0384, "Kak Potion Shop Front", 0},
    { 0x05A0, "Kak Redead Grotto", "Leaving Kak Redead Grotto"},
    { 0x003B, "Kak Shooting Gallery", 0},
    { 0x0453, "Kak Windmill", 0},
    { 0x009C, "KF House of Twins", 0},
    { 0x00C9, "KF Know It All House", 0},
    { 0x00C1, "KF Kokiri Shop", 0},
    { 0x00BB, "KF Links House", 0},
    { 0x0272, "KF Links House", 0},
    { 0x0433, "KF Midos House", 0},
    { 0x0209, "KF Outside Deku Tree", 0},
    { 0x0437, "KF Sarias House", 0},
    { 0x040B, "King Dodongo Boss Room", 0},
    { 0x020D, "Kokiri Forest", 0},
    { 0x0211, "Kokiri Forest", 0},
    { 0x0266, "Kokiri Forest", 0},
    { 0x026A, "Kokiri Forest", 0},
    { 0x0286, "Kokiri Forest", 0},
    { 0x033C, "Kokiri Forest", 0},
    { 0x0443, "Kokiri Forest", 0},
    { 0x0447, "Kokiri Forest", 0},
    { 0x0102, "Lake Hylia", 0},
    { 0x0219, "Lake Hylia", 0},
    { 0x021D, "Lake Hylia", 0},
    { 0x03CC, "Lake Hylia", 0},
    { 0x0560, "Lake Hylia", 0},
    { 0x0604, "Lake Hylia", 0},
    { 0x060C, "Lake Hylia", 0},
    { 0x04E6, "Lake Hylia after Sheik Cutscene", 0},
    { 0x045F, "LH Fishing Hole", 0},
    { 0x0309, "LH Fishing Island", 0},
    { 0x0043, "LH Lab", 0},
    { 0x02F9, "LLR Stables", 0},
    { 0x004F, "LLR Talons House", 0},
    { 0x05E4, "LLR Talons House", 0},
    { 0x05D0, "LLR Tower", 0},
    { 0x0157, "Lon Lon Ranch", 0},
    { 0x0378, "Lon Lon Ranch", 0},
    { 0x042F, "Lon Lon Ranch", 0},
    { 0x05D4, "Lon Lon Ranch", 0},
    { 0x011E, "Lost Woods", 0},
    { 0x04D6, "Lost Woods", 0},
    { 0x01A9, "LW Beyond Mido", 0},
    { 0x04DE, "LW Bridge", 0},
    { 0x05E0, "LW Bridge From Forest", 0},
    { 0x05B0, "LW Scrubs Grotto", "Leaving LW Scrubs Grotto"},
    { 0x04DA, "LW Underwater Entrance", 0},
    { 0x00B1, "Market", 0},
    { 0x01CD, "Market", 0},
    { 0x01D1, "Market", 0},
    { 0x01D5, "Market", 0},
    { 0x025A, "Market", 0},
    { 0x025E, "Market", 0},
    { 0x0262, "Market", 0},
    { 0x029E, "Market", 0},
    { 0x02A2, "Market", 0},
    { 0x03B8, "Market", 0},
    { 0x03BC, "Market", 0},
    { 0x0067, "Market Back Alley", 0},
    { 0x038C, "Market Back Alley", 0},
    { 0x03C0, "Market Back Alley", 0},
    { 0x029A, "Market Back Alley Left Side", 0},
    { 0x00AD, "Market Back Alley Right Side", 0},
    { 0x052C, "Market Bazaar", 0},
    { 0x0507, "Market Bombchu Bowling", 0},
    { 0x0528, "Market Bombchu Shop", 0},
    { 0x0033, "Market Entrance", 0},
    { 0x026E, "Market Entrance", 0},
    { 0x0276, "Market Entrance", 0},
    { 0x007E, "Market Guard House", 0},
    { 0x043B, "Market Man in Green House", 0},
    { 0x0530, "Market Mask Shop", 0},
    { 0x0388, "Market Potion Shop", 0},
    { 0x016D, "Market Shooting Gallery", 0},
    { 0x0063, "Market Treasure Chest Game", 0},
    { 0x0417, "Morpha Boss Room", 0},
    { 0x04C2, "OGC GFF", 0},
    { 0x000C, "Phantom Ganon Boss Room", 0},
    { 0x040F, "Queen Gohma Boss Room", 0},
    { 0x0600, "Sacred Forest Meadow", 0},
    { 0x00FC, "SFM Entryway", 0},
    { 0x0215, "SFM Forest Temple Entrance Ledge", 0},
    { 0x0608, "SFM Forest Temple Entrance Ledge", 0},
    { 0x05B4, "SFM Wolfos Grotto", "Leaving SFM Wolfos Grotto"},
    { 0x0037, "Shadow Temple Entryway", 0},
    { 0x05CC, "(Special) Bottom of the Well by Entrance", 0},
    { 0x006B, "(Special) Chamber of Sages", 0},
    { 0x01ED, "(Special) Desert Colossus after Requiem Cutscene", 0},
    { 0x01F5, "(Special) Desert Colossus after Silver Gauntlets Cutscene", 0},
    { 0x047A, "(Special) DMT after Darunia Cutscene", 0},
    { 0x0315, "(Special) DMT GFF", 0},
    { 0x045B, "(Special) DMT Outside GFF", 0},
    { 0x0175, "(Special) Fire Temple by Boss Door", 0},
    { 0x02CE, "(Special) Forest Medallion Cutscene", 0},
    { 0x024E, "(Special) Forest Temple by Boss Door", 0},
    { 0x0584, "(Special) Forest Temple Falling Ceiling Room", 0},
    { 0x0517, "(Special) Ganon Battle", 0},
    { 0x05F8, "(Special) GF Caught by Gerudos Again", 0},
    { 0x03B4, "(Special) GF Caught by Gerudos First Time", 0},
    { 0x01A5, "(Special) GV Caught by Gerudos without Hookshot", 0},
    { 0x0594, "(Special) Hyrule Field after Impa Escort Cutscene", 0},
    { 0x0513, "(Special) Kakariko Village after Nocturne Cutscene", 0},
    { 0x05E8, "(Special) KF Deku Sprout Cutscene", 0},
    { 0x0457, "(Special) KF Outside Deku Tree", 0},
    { 0x00EE, "(Special) Kokiri Forest by Links House", 0},
    { 0x02AE, "(Special) LLR after Eponas Song Cutscene", 0},
    { 0x04CE, "(Special) LLR Beat Ingo", 0},
    { 0x0558, "(Special) LLR Lose to Ingo", 0},
    { 0x055C, "(Special) LLR Near Ingo", 0},
    { 0x02E2, "(Special) LLR Near South Gate", 0},
    { 0x02E6, "(Special) LLR Near West Gate", 0},
    { 0x04CA, "(Special) LLR Pay Ingo to Ride", 0},
    { 0x04C6, "(Special) LW by Target in Woods", 0},
    { 0x01AD, "(Special) LW Mushroom Timeout", 0},
    { 0x0390, "(Special) Market Bombchu Shop", 0},
    { 0x0574, "(Special) Royal Familys Tomb after Sun Song Cutscene", 0},
    { 0x02B2, "(Special) Shadow Temple by Boss Door", 0},
    { 0x00A0, "(Special) Sky Temple", 0},
    { 0x02F5, "(Special) Spirit Temple by Boss Door", 0},
    { 0x05EC, "(Special) Spirit Temple Twinrova Room", 0},
    { 0x058C, "(Special) ToT after LACS", 0},
    { 0x0320, "(Special) ToT Facing Away From Pedestal", 0},
    { 0x0590, "(Special) ToT in front of Warp Pad", 0},
    { 0x0324, "(Special) ToT Pull Sword First Time", 0},
    { 0x0423, "(Special) Water Temple by Boss Door", 0},
    { 0x0153, "(Special) ZD Eyeball Frog Timeout", 0},
    { 0x05F0, "(Special) Zeldas Courtyard after Triforce Cutscene", 0},
    { 0x03F4, "Spirit Temple Left Iron Knuckle", 0},
    { 0x0082, "Spirit Temple Lobby", 0},
    { 0x03F0, "Spirit Temple Right Iron Knuckle", 0},
    { 0x0053, "Temple of Time", 0},
    { 0x02CA, "Temple of Time", 0},
    { 0x05F4, "Temple of Time", 0},
    { 0x048A, "Thieves Hideout 1 Torch Jail Back", 0},
    { 0x0486, "Thieves Hideout 1 Torch Jail Front", 0},
    { 0x049E, "Thieves Hideout 2 Torch Jail", 0},
    { 0x04A2, "Thieves Hideout 2 Torch Jail", 0},
    { 0x0570, "Thieves Hideout 3 Torch Jail", 0},
    { 0x0496, "Thieves Hideout 4 Torch Jail", 0},
    { 0x049A, "Thieves Hideout 4 Torch Jail", 0},
    { 0x04AE, "Thieves Hideout Break Room Bottom", 0},
    { 0x04B2, "Thieves Hideout Break Room Top", 0},
    { 0x04AA, "Thieves Hideout Kitchen Back", 0},
    { 0x04A6, "Thieves Hideout Kitchen Front", 0},
    { 0x048E, "Thieves Hideout Kitchen Hallway Bottom", 0},
    { 0x0492, "Thieves Hideout Kitchen Hallway Top", 0},
    { 0x0171, "ToT Entrance", 0},
    { 0x0472, "ToT Entrance", 0},
    { 0x01C9, "Tower Collapse Exterior 0 Gaps Bottom", 0},
    { 0x0334, "Tower Collapse Exterior 0 Gaps Top", 0},
    { 0x0524, "Tower Collapse Exterior 1 Gap Bottom", 0},
    { 0x051C, "Tower Collapse Exterior 1 Gap Top", 0},
    { 0x0330, "Tower Collapse Exterior 2 Gaps Bottom", 0},
    { 0x032C, "Tower Collapse Exterior 2 Gaps Top", 0},
    { 0x043F, "Tower Collapse Exterior Top", 0},
    { 0x04BA, "Tower Collapse Exterior Top", 0},
    { 0x0134, "Tower Collapse Interior Boss Room Floor Back", 0},
    { 0x0179, "Tower Collapse Interior Boss Room Floor Front", 0},
    { 0x056C, "Tower Collapse Interior Bridge Front", 0},
    { 0x0256, "Tower Collapse Interior Burning Rocks Back", 0},
    { 0x01B5, "Tower Collapse Interior Burning Rocks Front", 0},
    { 0x04B6, "Tower Collapse Interior Staircase Bottom", 0},
    { 0x03E4, "Tower Collapse Interior Staircase Top", 0},
    { 0x03DC, "Tower Collapse Interior Stalfos Fight Back", 0},
    { 0x03E0, "Tower Collapse Interior Stalfos Fight Front", 0},
    { 0x008D, "Twinrova Boss Room", 0},
    { 0x0305, "Volvagia Boss Room", 0},
    { 0x0365, "Wasteland Near Colossus", 0},
    { 0x0130, "Wasteland Near Fortress", 0},
    { 0x0010, "Water Temple Lobby", 0},
    { 0x0503, "Windmill after Song of Storms", 0},
    { 0x01A1, "ZD Behind King Zora", 0},
    { 0x0380, "ZD Shop", 0},
    { 0x0400, "Zeldas Courtyard", 0},
    { 0x0371, "ZF GFF", 0},
    { 0x03D4, "ZF Ice Ledge", 0},
    { 0x01DD, "Zora River", 0},
    { 0x0108, "Zoras Domain", 0},
    { 0x0328, "Zoras Domain", 0},
    { 0x03C4, "Zoras Domain", 0},
    { 0x010E, "Zoras Fountain", 0},
    { 0x0221, "Zoras Fountain", 0},
    { 0x0225, "Zoras Fountain", 0},
    { 0x0394, "Zoras Fountain", 0},
    { 0x019D, "ZR Behind Waterfall", 0},
    { 0x00EA, "ZR Front Land", 0},
    { 0x01D9, "ZR Front Water", 0},
};

// Keep this alphabetically ordered!
entrance_table_entry_t entrance_table[] =
{
    { 0x05BC, "2 Scrub Grotto", 0},
    { 0x05A4, "3 Scrub Grotto", 0},
    { 0x010E, "Barinade Boss Room Blue Warp -> Zoras Fountain", 0},
    { 0x04F6, "Bolero of Fire Warp -> DMC Central Local", 0},
    { 0x0580, "Bongo Bongo Boss Room Blue Warp -> Graveyard Warp Pad Region", 0},
    { 0x02A6, "Bottom of the Well -> Kakariko Village", 0},
    { 0x0400, "Castle Courtyard -> Zeldas Courtyard", 0},
    { 0x0340, "Castle GFF -> Castle Grounds", 0},
    { 0x025A, "Castle Grounds -> Market", 0},
    { 0x047E, "Caught by Castle Grounds Guards -> Castle Grounds", 0},
    { 0x04FA, "Caught by Courtyard Guards -> Castle Grounds", 0},
    { 0x00BB, "Child Spawn -> KF Links House", 0},
    { 0x0544, "Clear Fire Trial -> Ganons Castle Lobby", 0},
    { 0x0538, "Clear Forest Trial -> Ganons Castle Lobby", 0},
    { 0x0548, "Clear Light Trial -> Ganons Castle Lobby", 0},
    { 0x0540, "Clear Shadow Trial -> Ganons Castle Lobby", 0},
    { 0x054C, "Clear Spirit Trial -> Ganons Castle Lobby", 0},
    { 0x053C, "Clear Water Trial -> Ganons Castle Lobby", 0},
    { 0x057C, "Colossus GFF -> Desert Colossus", 0},
    { 0x0004, "Death Mountain -> Dodongos Cavern Beginning", 0},
    { 0x014D, "Death Mountain -> Goron City", 0},
    { 0x0191, "Death Mountain -> Kak Behind Gate", 0},
    { 0x0147, "Death Mountain Summit -> DMC Upper Local", 0},
    { 0x05FC, "Death Mountain Summit -> DMT Cow Grotto", "Leaving DMT Cow Grotto"},
    { 0x040F, "Deku Tree Boss Door -> Queen Gohma Boss Room", 0},
    { 0x0209, "Deku Tree Lobby -> KF Outside Deku Tree", 0},
    { 0x0588, "Desert Colossus -> Colossus GFF", 0},
    { 0x03F4, "Desert Colossus Mirror Shield Chest -> Spirit Temple Left Iron Knuckle", 0},
    { 0x03F0, "Desert Colossus Silver Gauntlets Chest -> Spirit Temple Right Iron Knuckle", 0},
    { 0x0082, "Desert Colossus -> Spirit Temple Lobby", 0},
    { 0x0365, "Desert Colossus -> Wasteland Near Colossus", 0},
    { 0x0165, "DMC Fire Temple Entrance -> Fire Temple Lower", 0},
    { 0x0482, "DMC GFF -> DMC Lower Local", 0},
    { 0x04BE, "DMC Lower Nearby -> DMC GFF", 0},
    { 0x01C1, "DMC Lower Nearby -> GC Darunias Chamber", 0},
    { 0x01BD, "DMC Upper Nearby -> Death Mountain Summit", 0},
    { 0x0554, "DMT Owl Flight -> Kak Impas Rooftop", 0},
    { 0x0242, "Dodongos Cavern Beginning -> Death Mountain", 0},
    { 0x040B, "Dodongos Cavern Boss Door -> King Dodongo Boss Room", 0},
    { 0x038C, "Dog Lady House -> Market Back Alley", 0},
    { 0x036D, "Fairy Grotto", 0},
    { 0x0305, "Fire Temple Boss Door -> Volvagia Boss Room", 0},
    { 0x024A, "Fire Temple Lower -> DMC Fire Temple Entrance", 0},
    { 0x000C, "Forest Temple Boss Door -> Phantom Ganon Boss Room", 0},
    { 0x0215, "Forest Temple Lobby -> SFM Forest Temple Entrance Ledge", 0},
    { 0x043F, "Ganondorf Boss Room -> Tower Collapse Exterior Top", 0},
    { 0x0467, "Ganons Castle Grounds -> Ganons Castle Lobby", 0},
    { 0x04C2, "Ganons Castle Grounds -> OGC GFF", 0},
    { 0x023D, "Ganons Castle Lobby -> Castle Grounds From Ganons Castle", 0},
    { 0x041B, "Ganons Castle Lobby -> Ganons Tower Bottom", 0},
    { 0x0534, "Ganons Tower Bottom -> Ganons Castle Lobby", 0},
    { 0x041F, "Ganons Tower Top -> Ganondorf Boss Room", 0},
    { 0x0246, "GC Darunias Chamber -> DMC Lower Local", 0},
    { 0x03FC, "GC Shop -> Goron City", 0},
    { 0x04D6, "GC Woods Warp -> Lost Woods", 0},
    { 0x003F, "Generic Grotto", 0},
    { 0x049E, "Gerudo Fortress Above GTG Center -> Thieves Hideout 2 Torch Jail", 0},
    { 0x0492, "Gerudo Fortress Above GTG Left -> Thieves Hideout Kitchen Hallway Top", 0},
    { 0x04A2, "Gerudo Fortress Above Vines Back -> Thieves Hideout 2 Torch Jail", 0},
    { 0x04A6, "Gerudo Fortress Above Vines Left -> Thieves Hideout Kitchen Front", 0},
    { 0x04B2, "Gerudo Fortress Balcony -> Thieves Hideout Break Room Top", 0},
    { 0x048E, "Gerudo Fortress Behind Crates Center -> Thieves Hideout Kitchen Hallway Bottom", 0},
    { 0x048A, "Gerudo Fortress Behind Crates Left -> Thieves Hideout 1 Torch Jail Back", 0},
    { 0x0486, "Gerudo Fortress Bottom Left -> Thieves Hideout 1 Torch Jail Front", 0},
    { 0x0008, "Gerudo Fortress -> Gerudo Training Ground Lobby", 0},
    { 0x022D, "Gerudo Fortress -> GV Fortress Side", 0},
    { 0x04AE, "Gerudo Fortress Left Alcove -> Thieves Hideout Break Room Bottom", 0},
    { 0x0496, "Gerudo Fortress Left of GTG -> Thieves Hideout 4 Torch Jail", 0},
    { 0x049A, "Gerudo Fortress Mid Across from Vines -> Thieves Hideout 4 Torch Jail", 0},
    { 0x0570, "Gerudo Fortress Near Top -> Thieves Hideout 3 Torch Jail", 0},
    { 0x04AA, "Gerudo Fortress Top -> Thieves Hideout Kitchen Back", 0},
    { 0x03A8, "Gerudo Training Ground Lobby -> Gerudo Fortress", 0},
    { 0x018D, "Gerudo Valley -> Hyrule Field", 0},
    { 0x03B0, "GF Horse Back Archery -> Gerudo Fortress", 0},
    { 0x0130, "GF Outside Gate -> Wasteland Near Fortress", 0},
    { 0x01B9, "Goron City -> Death Mountain", 0},
    { 0x037C, "Goron City -> GC Shop", 0},
    { 0x0359, "Graveyard Dampes Grave -> Graveyard", 0},
    { 0x0355, "Graveyard Dampes House -> Graveyard", 0},
    { 0x044F, "Graveyard -> Graveyard Dampes Grave", 0},
    { 0x030D, "Graveyard -> Graveyard Dampes House", 0},
    { 0x031C, "Graveyard -> Graveyard Heart Piece Grave", 0},
    { 0x002D, "Graveyard -> Graveyard Royal Familys Tomb", 0},
    { 0x004B, "Graveyard -> Graveyard Shield Grave", 0},
    { 0x0361, "Graveyard Heart Piece Grave -> Graveyard", 0},
    { 0x0195, "Graveyard -> Kakariko Village", 0},
    { 0x050B, "Graveyard Royal Familys Tomb -> Graveyard", 0},
    { 0x035D, "Graveyard Shield Grave -> Graveyard", 0},
    { 0x0037, "Graveyard Warp Pad Region -> Shadow Temple Entryway", 0},
    { 0x03D0, "GV Carpenter Tent -> GV Fortress Side", 0},
    { 0x0129, "GV Fortress Side -> Gerudo Fortress", 0},
    { 0x03A0, "GV Fortress Side -> GV Carpenter Tent", 0},
    { 0x05AC, "GV Grotto Ledge -> GV Octorok Grotto", "Leaving GV Grotto Ledge"},
    { 0x0219, "GV Lower Stream -> Lake Hylia", 0},
    { 0x007A, "Hyrule Castle Grounds -> Castle Courtyard", 0},
    { 0x0578, "Hyrule Castle Grounds -> HC GFF", 0},
    { 0x05B8, "Hyrule Castle Grounds -> HC Storms Grotto", "Leaving HC Storms Grotto"},
    { 0x050F, "Hyrule Field after Ocarina of Time", 0},
    { 0x0117, "Hyrule Field -> Gerudo Valley", 0},
    { 0x05A8, "Hyrule Field -> HF Cow Grotto", "Leaving HF Cow Grotto"},
    { 0x059C, "Hyrule Field -> HF Inside Fence Grotto", "Leaving HF Inside Fence Grotto"},
    { 0x0598, "Hyrule Field -> HF Near Kak Grotto", "Leaving HF Near Kak Grotto"},
    { 0x05C0, "Hyrule Field -> HF Tektite Grotto", "Leaving HF Tektite Grotto"},
    { 0x00DB, "Hyrule Field -> Kakariko Village", 0},
    { 0x0102, "Hyrule Field -> Lake Hylia", 0},
    { 0x0157, "Hyrule Field -> Lon Lon Ranch", 0},
    { 0x04DE, "Hyrule Field -> LW Bridge", 0},
    { 0x0276, "Hyrule Field -> Market Entrance", 0},
    { 0x00EA, "Hyrule Field -> ZR Front Land", 0},
    { 0x01D9, "Hyrule Field -> ZR Front Water", 0},
    { 0x03D4, "Ice Cavern Beginning -> ZF Ice Ledge", 0},
    { 0x0221, "Jabu Jabus Belly Beginning -> Zoras Fountain", 0},
    { 0x0301, "Jabu Jabus Belly Boss Door -> Barinade Boss Room", 0},
    { 0x0098, "Kakariko Village -> Bottom of the Well", 0},
    { 0x00E4, "Kakariko Village -> Graveyard", 0},
    { 0x017D, "Kakariko Village -> Hyrule Field", 0},
    { 0x00B7, "Kakariko Village -> Kak Bazaar", 0},
    { 0x02FD, "Kakariko Village -> Kak Carpenter Boss House", 0},
    { 0x0550, "Kakariko Village -> Kak House of Skulltula", 0},
    { 0x039C, "Kakariko Village -> Kak Impas House", 0},
    { 0x0384, "Kakariko Village -> Kak Potion Shop Front", 0},
    { 0x05A0, "Kakariko Village -> Kak Redead Grotto", "Leaving Kak Redead Grotto"},
    { 0x003B, "Kakariko Village -> Kak Shooting Gallery", 0},
    { 0x0453, "Kakariko Village -> Kak Windmill", 0},
    { 0x0072, "Kak Backyard -> Kak Odd Medicine Building", 0},
    { 0x03EC, "Kak Backyard -> Kak Potion Shop Back", 0},
    { 0x0201, "Kak Bazaar -> Kakariko Village", 0},
    { 0x013D, "Kak Behind Gate -> Death Mountain", 0},
    { 0x0349, "Kak Carpenter Boss House -> Kakariko Village", 0},
    { 0x04EE, "Kak House of Skulltula -> Kakariko Village", 0},
    { 0x05DC, "Kak Impas House Back -> Kak Impas Ledge", 0},
    { 0x0345, "Kak Impas House -> Kakariko Village", 0},
    { 0x05C8, "Kak Impas Ledge -> Kak Impas House Back", 0},
    { 0x034D, "Kak Odd Medicine Building -> Kak Backyard", 0},
    { 0x04FF, "Kak Potion Shop Back -> Kak Backyard", 0},
    { 0x044B, "Kak Potion Shop Front -> Kakariko Village", 0},
    { 0x0463, "Kak Shooting Gallery -> Kakariko Village", 0},
    { 0x0351, "Kak Windmill -> Kakariko Village", 0},
    { 0x033C, "KF House of Twins -> Kokiri Forest", 0},
    { 0x026A, "KF Know It All House -> Kokiri Forest", 0},
    { 0x0266, "KF Kokiri Shop -> Kokiri Forest", 0},
    { 0x0211, "KF Links House -> Kokiri Forest", 0},
    { 0x0443, "KF Midos House -> Kokiri Forest", 0},
    { 0x0000, "KF Outside Deku Tree -> Deku Tree Lobby", 0},
    { 0x0447, "KF Sarias House -> Kokiri Forest", 0},
    { 0x00C5, "King Dodongo Boss Room -> Dodongos Cavern Boss Door", 0},
    { 0x009C, "Kokiri Forest -> KF House of Twins", 0},
    { 0x00C9, "Kokiri Forest -> KF Know It All House", 0},
    { 0x00C1, "Kokiri Forest -> KF Kokiri Shop", 0},
    { 0x0272, "Kokiri Forest -> KF Links House", 0},
    { 0x0433, "Kokiri Forest -> KF Midos House", 0},
    { 0x0437, "Kokiri Forest -> KF Sarias House", 0},
    { 0x011E, "Kokiri Forest -> Lost Woods", 0},
    { 0x05E0, "Kokiri Forest -> LW Bridge From Forest", 0},
    { 0x04E6, "Lake Hylia after Sheik Cutscene", 0},
    { 0x0189, "Lake Hylia -> Hyrule Field", 0},
    { 0x0043, "Lake Hylia -> LH Lab", 0},
    { 0x0010, "Lake Hylia -> Water Temple Lobby", 0},
    { 0x0328, "Lake Hylia -> Zoras Domain", 0},
    { 0x0309, "LH Fishing Hole -> LH Fishing Island", 0},
    { 0x045F, "LH Fishing Island -> LH Fishing Hole", 0},
    { 0x03CC, "LH Lab -> Lake Hylia", 0},
    { 0x027E, "LH Owl Flight -> Hyrule Field", 0},
    { 0x0292, "LLR Eastern Fence Jump -> Hyrule Field", 0},
    { 0x0476, "LLR Front Gate Jump -> Hyrule Field", 0},
    { 0x028A, "LLR Southern Fence Jump -> Hyrule Field", 0},
    { 0x042F, "LLR Stables -> Lon Lon Ranch", 0},
    { 0x05E4, "LLR Talons House after Cucco Game -> LLR Talons House", 0},
    { 0x0378, "LLR Talons House -> Lon Lon Ranch", 0},
    { 0x05D4, "LLR Tower -> Lon Lon Ranch", 0},
    { 0x028E, "LLR Western Fence Jump -> Hyrule Field", 0},
    { 0x01F9, "Lon Lon Ranch -> Hyrule Field", 0},
    { 0x02F9, "Lon Lon Ranch -> LLR Stables", 0},
    { 0x004F, "Lon Lon Ranch -> LLR Talons House", 0},
    { 0x05D0, "Lon Lon Ranch -> LLR Tower", 0},
    { 0x04E2, "Lost Woods -> GC Woods Warp", 0},
    { 0x01DD, "Lost Woods -> Zora River", 0},
    { 0x05C4, "LW Beyond Mido -> Deku Theater", "Leaving Deku Theather"},
    { 0x05B0, "LW Beyond Mido -> LW Scrubs Grotto", "Leaving LW Scrubs Grotto"},
    { 0x00FC, "LW Beyond Mido -> SFM Entryway", 0},
    { 0x0185, "LW Bridge -> Hyrule Field", 0},
    { 0x020D, "LW Bridge -> Kokiri Forest", 0},
    { 0x0286, "LW Forest Exit -> Kokiri Forest", 0},
    { 0x0398, "Market Back Alley -> Dog Lady House", 0},
    { 0x029E, "Market Back Alley Left Side -> Market", 0},
    { 0x0528, "Market Back Alley -> Market Bombchu Shop", 0},
    { 0x043B, "Market Back Alley -> Market Man in Green House", 0},
    { 0x0262, "Market Back Alley Right Side -> Market", 0},
    { 0x03B8, "Market Bazaar -> Market", 0},
    { 0x03BC, "Market Bombchu Bowling -> Market", 0},
    { 0x03C0, "Market Bombchu Shop -> Market Back Alley", 0},
    { 0x0138, "Market -> Castle Grounds", 0},
    { 0x01FD, "Market Entrance -> Hyrule Field", 0},
    { 0x00B1, "Market Entrance -> Market", 0},
    { 0x007E, "Market Entrance -> Market Guard House", 0},
    { 0x026E, "Market Guard House -> Market Entrance", 0},
    { 0x0067, "Market Man in Green House -> Market Back Alley", 0},
    { 0x029A, "Market -> Market Back Alley Left Side", 0},
    { 0x00AD, "Market -> Market Back Alley Right Side", 0},
    { 0x052C, "Market -> Market Bazaar", 0},
    { 0x0507, "Market -> Market Bombchu Bowling", 0},
    { 0x0033, "Market -> Market Entrance", 0},
    { 0x0530, "Market -> Market Mask Shop", 0},
    { 0x0388, "Market -> Market Potion Shop", 0},
    { 0x016D, "Market -> Market Shooting Gallery", 0},
    { 0x0063, "Market -> Market Treasure Chest Game", 0},
    { 0x01D1, "Market Mask Shop -> Market", 0},
    { 0x02A2, "Market Potion Shop -> Market", 0},
    { 0x01CD, "Market Shooting Gallery -> Market", 0},
    { 0x0171, "Market -> ToT Entrance", 0},
    { 0x01D5, "Market Treasure Chest Game -> Market", 0},
    { 0x0600, "Minuet of Forest Warp -> Sacred Forest Meadow", 0},
    { 0x060C, "Morpha Boss Room Blue Warp -> Lake Hylia", 0},
    { 0x0568, "Nocturne of Shadow Warp -> Graveyard Warp Pad Region", 0},
    { 0x0608, "Phantom Ganon Boss Room Blue Warp -> SFM Forest Temple Entrance Ledge", 0},
    { 0x05F4, "Prelude of Light Warp -> Temple of Time", 0},
    { 0x0252, "Queen Gohma Boss Room -> Deku Tree Boss Door", 0},
    { 0x01F1, "Requiem of Spirit Warp -> Desert Colossus", 0},
    { 0x0604, "Serenade of Water Warp -> Lake Hylia", 0},
    { 0x01A9, "SFM Entryway -> LW Beyond Mido", 0},
    { 0x05B4, "SFM Entryway -> SFM Wolfos Grotto", "Leaving SFM Wolfos Grotto"},
    { 0x0169, "SFM Forest Temple Entrance Ledge -> Forest Temple Lobby", 0},
    { 0x0413, "Shadow Temple Boss Door -> Bongo Bongo Boss Room", 0},
    { 0x0205, "Shadow Temple Entryway -> Graveyard Warp Pad Region", 0},
    { 0x05CC, "(Special) Bottom of the Well by Entrance", 0},
    { 0x006B, "(Special) Chamber of Sages", 0},
    { 0x01ED, "(Special) Desert Colossus after Requiem Cutscene", 0},
    { 0x01F5, "(Special) Desert Colossus after Silver Gauntlets Cutscene", 0},
    { 0x047A, "(Special) DMT after Darunia Cutscene", 0},
    { 0x0315, "(Special) DMT GFF", 0},
    { 0x045B, "(Special) DMT Outside GFF", 0},
    { 0x0175, "(Special) Fire Temple by Boss Door", 0},
    { 0x02CE, "(Special) Forest Medallion Cutscene", 0},
    { 0x024E, "(Special) Forest Temple by Boss Door", 0},
    { 0x0584, "(Special) Forest Temple Falling Ceiling Room", 0},
    { 0x0517, "(Special) Ganon Battle", 0},
    { 0x05F8, "(Special) GF Caught by Gerudos Again", 0},
    { 0x03B4, "(Special) GF Caught by Gerudos First Time", 0},
    { 0x01A5, "(Special) GV Caught by Gerudos without Hookshot", 0},
    { 0x0594, "(Special) Hyrule Field after Impa Escort Cutscene", 0},
    { 0x0513, "(Special) Kakariko Village after Nocturne Cutscene", 0},
    { 0x05E8, "(Special) KF Deku Sprout Cutscene", 0},
    { 0x0457, "(Special) KF Outside Deku Tree", 0},
    { 0x00EE, "(Special) Kokiri Forest by Links House", 0},
    { 0x02AE, "(Special) LLR after Eponas Song Cutscene", 0},
    { 0x04CE, "(Special) LLR Beat Ingo", 0},
    { 0x0558, "(Special) LLR Lose to Ingo", 0},
    { 0x055C, "(Special) LLR Near Ingo", 0},
    { 0x02E2, "(Special) LLR Near South Gate", 0},
    { 0x02E6, "(Special) LLR Near West Gate", 0},
    { 0x04CA, "(Special) LLR Pay Ingo to Ride", 0},
    { 0x04C6, "(Special) LW by Target in Woods", 0},
    { 0x01AD, "(Special) LW Mushroom Timeout", 0},
    { 0x0390, "(Special) Market Bombchu Shop", 0},
    { 0x0574, "(Special) Royal Familys Tomb after Sun Song Cutscene", 0},
    { 0x02B2, "(Special) Shadow Temple by Boss Door", 0},
    { 0x00A0, "(Special) Sky Temple", 0},
    { 0x02F5, "(Special) Spirit Temple by Boss Door", 0},
    { 0x05EC, "(Special) Spirit Temple Twinrova Room", 0},
    { 0x058C, "(Special) ToT after LACS", 0},
    { 0x0320, "(Special) ToT Facing Away From Pedestal", 0},
    { 0x0590, "(Special) ToT in front of Warp Pad", 0},
    { 0x0324, "(Special) ToT Pull Sword First Time", 0},
    { 0x0423, "(Special) Water Temple by Boss Door", 0},
    { 0x0153, "(Special) ZD Eyeball Frog Timeout", 0},
    { 0x05F0, "(Special) Zeldas Courtyard after Triforce Cutscene", 0},
    { 0x008D, "Spirit Temple Boss Door -> Twinrova Boss Room", 0},
    { 0x01E9, "Spirit Temple Left Iron Knuckle -> Desert Colossus Mirror Shield Chest", 0},
    { 0x01E1, "Spirit Temple Lobby -> Desert Colossus From Spirit Lobby", 0},
    { 0x01E5, "Spirit Temple Right Iron Knuckle -> Desert Colossus Silver Gauntlets Chest", 0},
    { 0x0472, "Temple of Time -> ToT Entrance", 0},
    { 0x0235, "Thieves Hideout 1 Torch Jail Back -> Gerudo Fortress Behind Crates Left", 0},
    { 0x0231, "Thieves Hideout 1 Torch Jail Front -> Gerudo Fortress Bottom Left", 0},
    { 0x02C2, "Thieves Hideout 2 Torch Jail Bottom -> Gerudo Fortress Above GTG Center", 0},
    { 0x02C6, "Thieves Hideout 2 Torch Jail Top -> Gerudo Fortress Above Vines Back", 0},
    { 0x03A4, "Thieves Hideout 3 Torch Jail -> Gerudo Fortress Near Top", 0},
    { 0x02BE, "Thieves Hideout 4 Torch Jail Back -> Gerudo Fortress Mid Across from Vines", 0},
    { 0x02BA, "Thieves Hideout 4 Torch Jail Front -> Gerudo Fortress Left of GTG", 0},
    { 0x02DA, "Thieves Hideout Break Room Bottom -> Gerudo Fortress Left Alcove", 0},
    { 0x02DE, "Thieves Hideout Break Room Top -> Gerudo Fortress Balcony", 0},
    { 0x02D6, "Thieves Hideout Kitchen Back -> Gerudo Fortress Top", 0},
    { 0x02D2, "Thieves Hideout Kitchen Front -> Gerudo Fortress Above Vines Left", 0},
    { 0x0239, "Thieves Hideout Kitchen Hallway Bottom -> Gerudo Fortress Behind Crates Center", 0},
    { 0x02AA, "Thieves Hideout Kitchen Hallway Top -> Gerudo Fortress Above GTG Left", 0},
    { 0x025E, "ToT Entrance -> Market", 0},
    { 0x0053, "ToT Entrance -> Temple of Time", 0},
    { 0x02CA, "ToT Pull or Place Sword -> Temple of Time", 0},
    { 0x03E0, "Tower Collapse Exterior 0 Gaps Bottom -> Tower Collapse Interior Stalfos Fight Front", 0},
    { 0x0256, "Tower Collapse Exterior 0 Gaps Top -> Tower Collapse Interior Burning Rocks Back", 0},
    { 0x03E4, "Tower Collapse Exterior 1 Gap Bottom -> Tower Collapse Interior Staircase Top", 0},
    { 0x03DC, "Tower Collapse Exterior 1 Gap Top -> Tower Collapse Interior Stalfos Fight Back", 0},
    { 0x01B5, "Tower Collapse Exterior 2 Gaps Bottom -> Tower Collapse Interior Burning Rocks Front", 0},
    { 0x0134, "Tower Collapse Exterior 2 Gaps Top -> Tower Collapse Interior Boss Room Floor Back", 0},
    { 0x0179, "Tower Collapse Exterior Top -> Tower Collapse Interior Boss Room Floor Front", 0},
    { 0x032C, "Tower Collapse Interior Boss Room Floor Back -> Tower Collapse Exterior 2 Gaps Top", 0},
    { 0x04BA, "Tower Collapse Interior Boss Room Floor Front -> Tower Collapse Exterior Top", 0},
    { 0x04B6, "Tower Collapse Interior Bridge Front -> Tower Collapse Interior Staircase Bottom", 0},
    { 0x0334, "Tower Collapse Interior Burning Rocks Back -> Tower Collapse Exterior 0 Gaps Top", 0},
    { 0x0330, "Tower Collapse Interior Burning Rocks Front -> Tower Collapse Exterior 2 Gaps Bottom", 0},
    { 0x056C, "Tower Collapse Interior Staircase Bottom -> Tower Collapse Interior Bridge Front", 0},
    { 0x0524, "Tower Collapse Interior Staircase Top -> Tower Collapse Exterior 1 Gap Bottom", 0},
    { 0x051C, "Tower Collapse Interior Stalfos Fight Back -> Tower Collapse Exterior 1 Gap Top", 0},
    { 0x01C9, "Tower Collapse Interior Stalfos Room Front -> Tower Collapse Exterior 0 Gaps Bottom", 0},
    { 0x0610, "Twinrova Boss Room Blue Warp -> Desert Colossus From Spirit Lobby", 0},
    { 0x0564, "Volvagia Boss Room Blue Warp -> DMC Fire Temple Entrance", 0},
    { 0x0123, "Wasteland Near Colossus -> Desert Colossus", 0},
    { 0x03AC, "Wasteland Near Fortress -> GF Outside Gate", 0},
    { 0x0417, "Water Temple Boss Door -> Morpha Boss Room", 0},
    { 0x021D, "Water Temple Lobby -> Lake Hylia", 0},
    { 0x0503, "Windmill after Song of Storms", 0},
    { 0x0225, "ZD Behind King Zora -> Zoras Fountain", 0},
    { 0x03C4, "ZD Shop -> Zoras Domain", 0},
    { 0x00CD, "Zelda Escape Cutscene -> Hyrule Field", 0},
    { 0x0296, "Zeldas Courtyard -> Castle Courtyard", 0},
    { 0x0394, "ZF GFF -> Zoras Fountain", 0},
    { 0x0088, "ZF Ice Ledge -> Ice Cavern Beginning", 0},
    { 0x04DA, "Zora River -> LW Underwater Entrance", 0},
    { 0x0560, "Zoras Domain -> Lake Hylia", 0},
    { 0x0380, "Zoras Domain -> ZD Shop", 0},
    { 0x019D, "Zoras Domain -> ZR Behind Waterfall", 0},
    { 0x0028, "Zoras Fountain -> Jabu Jabus Belly Beginning", 0},
    { 0x01A1, "Zoras Fountain -> ZD Behind King Zora", 0},
    { 0x0371, "Zoras Fountain -> ZF GFF", 0},
    { 0x0108, "ZR Behind Waterfall -> Zoras Domain", 0},
    { 0x0181, "ZR Front Land -> Hyrule Field", 0},
    { 0x0311, "ZR Front Water -> Hyrule Field", 0},
};

static int du_was_pressed = 0;
static int dd_was_pressed = 0;
static int dl_was_pressed = 0;
static int dr_was_pressed = 0;
static int s_scroll_idx = 0;
static int s_num_visited = 0;

static uint8_t s_entrances_visited[sizeof(VAR_Entrances) / sizeof(VAR_Entrances[0])] = {};

static int s_entrance_info_initialized = 0;
static int s_first_entrance_skipped = 0;

static DisplayMode s_display_mode = DISPLAYMODE_NONE;

void check_dpad() {
    if (dd_was_pressed && !z64_game.common.input[0].pad_pressed.dd) {
        int max = (sizeof(VAR_Entrances) / sizeof(VAR_Entrances[0]));
        if (s_display_mode == DISPLAYMODE_BY_VISITED || s_display_mode == DISPLAYMODE_BY_VISITED_DEST) {
            max = s_num_visited;
            if (max < NUM_LINES) max = NUM_LINES;
        }

        s_scroll_idx = s_scroll_idx + NUM_LINES;
        if (s_scroll_idx >= max - NUM_LINES) {
            s_scroll_idx = max - NUM_LINES;
        }
        dd_was_pressed = 0;
    }

    if (z64_game.common.input[0].pad_pressed.dd) {
        dd_was_pressed = 1;
    }

    if (du_was_pressed && !z64_game.common.input[0].pad_pressed.du) {
        s_scroll_idx = s_scroll_idx - NUM_LINES;
        if (s_scroll_idx < 0) {
            s_scroll_idx = 0;
        }
        du_was_pressed = 0;
    }

    if (z64_game.common.input[0].pad_pressed.du) {
        du_was_pressed = 1;
    }

    if (dl_was_pressed && !z64_game.common.input[0].pad_pressed.dl) {
        if (s_display_mode == DISPLAYMODE_NONE) {
            s_display_mode = DISPLAYMODE_BY_VISITED;
        } else if (s_display_mode == DISPLAYMODE_BY_VISITED) {
            s_display_mode = DISPLAYMODE_BY_VISITED_DEST;
        } else {
            s_display_mode = DISPLAYMODE_NONE;
        }
        s_scroll_idx = 0;
        dl_was_pressed = 0;
    }

    if (z64_game.common.input[0].pad_pressed.dl) {
        dl_was_pressed = 1;
    }

    if (dr_was_pressed && !z64_game.common.input[0].pad_pressed.dr) {
        if (s_display_mode == DISPLAYMODE_NONE) {
            s_display_mode = DISPLAYMODE_ALL;
        } else if (s_display_mode == DISPLAYMODE_ALL) {
            s_display_mode = DISPLAYMODE_ALL_DEST;
        } else {
            s_display_mode = DISPLAYMODE_NONE;
        }
        s_scroll_idx = 0;
        dr_was_pressed = 0;
    }

    if (z64_game.common.input[0].pad_pressed.dr) {
        dr_was_pressed = 1;
    }
}

static int s_first_entrance = 0;

void draw_entrance_info(z64_disp_buf_t *db)
{
    if (z64_file.game_mode != GAMEMODE_NORMAL) {
        s_entrance_info_initialized = 0;
        s_first_entrance_skipped = 0;
        s_display_mode = DISPLAYMODE_NONE;
    }

    // Only show the spoiler log if the filename starts with SL
    char *filename = z64_file.file_name;
    if (((unsigned char)filename[0]) != 0xbd ||
        ((unsigned char)filename[1]) != 0xb6) {
        return;
    }

    if (!s_entrance_info_initialized) {
        if (z64_file.game_mode == GAMEMODE_NORMAL) {
            s_entrance_info_initialized = 1;
            for (int i = 0; i < sizeof(VAR_Entrances) / sizeof(VAR_Entrances[0]); i++) {
                s_entrances_visited[i] = 0;
            }
            s_first_entrance = z64_game.entrance_index;
            s_first_entrance_skipped = 0;
            s_num_visited = 0;
        }
    }

    if (s_entrance_info_initialized && z64_ctxt.state_continue) {
        for (int i = 0; i < sizeof(VAR_Entrances) / sizeof(VAR_Entrances[0]); i++) {
            if (ptr_VAR_SPACE[i] == z64_game.entrance_index) {
                if (z64_game.entrance_index != s_first_entrance || s_first_entrance_skipped) {
                    s_entrances_visited[i] = 1;
                    s_num_visited++;
                    s_first_entrance_skipped = 1;
                }
            }
        }
    }

    int draw = z64_game.pause_ctxt.state == 6 &&
               z64_game.pause_ctxt.screen_idx == PAUSE_QUEST &&
               !z64_game.pause_ctxt.changing &&
               z64_ctxt.input[0].raw.pad.a &&
               s_entrance_info_initialized;
    if (!draw) {
        s_scroll_idx = 0;
        s_display_mode = DISPLAYMODE_NONE;
        return;
    }

    check_dpad();

    db->p = db->buf;

    // Call setup display list

    gSPDisplayList(db->p++, &setup_db);

    // Set up dimensions

    int icon_size = 16;
    int padding = 1;
    int bg_width = Z64_SCREEN_WIDTH; // all we can get;
    int bg_height = padding + (NUM_LINES*2 * (icon_size + padding));
    int bg_left = (Z64_SCREEN_WIDTH - bg_width) / 2;
    int bg_top = (Z64_SCREEN_HEIGHT - bg_height) / 2;

    int left = bg_left + padding;
    int start_top = bg_top + padding;

    // Draw background

    gDPSetCombineMode(db->p++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
    gDPSetPrimColor(db->p++, 0, 0, 0x00, 0x00, 0x00, 0xD0);
    gSPTextureRectangle(db->p++,
                        bg_left << 2, bg_top << 2,
                        (bg_left + bg_width) << 2, (bg_top + bg_height) << 2,
                        0,
                        0, 0,
                        1 << 10, 1 << 10);

    // Set the primary color to white for drawing sprites

    gDPPipeSync(db->p++);
    gDPSetCombineMode(db->p++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);

    gDPSetPrimColor(db->p++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);


    if (s_display_mode == DISPLAYMODE_NONE) {
        int top = start_top + 1;

        text_print("Use DPAD to spoil Beta Quest", left, top);

        top += (icon_size + padding)*2;
        text_print("DPAD_LEFT: spoil since savewarp", left, top);
        top += (icon_size + padding);
        text_print("DPAD_LEFT(2x): same sorted by dest", left, top);

        top += (icon_size + padding);
        text_print("DPAD_RIGHT: spoil everything", left, top);
        top += (icon_size + padding);
        text_print("DPAD_RIGHT(2x): same sorted by dest", left, top);
    } else {
        // Draw entrance names based on s_display_mode
        for (int i = 0, k = 0, s = 0; (i < sizeof(VAR_Entrances) / sizeof(VAR_Entrances[0])) && (k < NUM_LINES); i++)
        {
            int top = start_top + ((icon_size + padding) * (k*2)) + 1;

            char *toEntrance = 0;
            char *fromEntrance = 0;
            int toIdx = -1;

            int idx = -1;
            short entrance = -1;

            if (s_display_mode == DISPLAYMODE_ALL_DEST || s_display_mode == DISPLAYMODE_BY_VISITED_DEST) {
                for (idx = 0; idx < sizeof(VAR_Entrances) / sizeof(VAR_Entrances[0]); idx++) {
                    if (VAR_Entrances[idx] == entrance_table_dest[i].entrance) {
                        for (toIdx = 0; toIdx < sizeof(VAR_Entrances) / sizeof(VAR_Entrances[0]); toIdx++) {
                            if (VAR_Entrances[idx] == ptr_VAR_SPACE[toIdx]) {
                                for (int t = 0; t < sizeof(VAR_Entrances) / sizeof(VAR_Entrances[0]); t++) {
                                    if (VAR_Entrances[toIdx] == entrance_table[t].entrance) {
                                        entrance = entrance_table[t].entrance;
                                    }
                                }
                                break;
                            }
                        }
                        break;
                    }
                }
            } else {
                entrance = entrance_table[i].entrance;
            }

            for (idx = 0; idx < sizeof(VAR_Entrances) / sizeof(VAR_Entrances[0]); idx++) {
                if (VAR_Entrances[idx] == entrance) {
                    break;
                }
            }

            for (int j = 0; j < sizeof(VAR_Entrances) / sizeof(VAR_Entrances[0]); j++) {
                if (ptr_VAR_SPACE[j] == entrance) {
                    toIdx = j;
                    break;
                }
            }

            for (int l = 0; l < sizeof(grotto_table) / sizeof(entrance_table_entry_t); l++) {
                if (grotto_table[l].entrance == entrance) {
                    fromEntrance = grotto_table[l].name;
                    break;
                }
            }

            if (!fromEntrance) {
                for (int l = 0; l < sizeof(entrance_table) / sizeof(entrance_table_entry_t); l++) {
                    if (entrance_table[l].entrance == entrance) {
                        if (entrance_table[l].fromName) fromEntrance = entrance_table[l].fromName;
                        else fromEntrance = entrance_table[l].name;
                        break;
                    }
                }
            }

            for (int l = 0; l < sizeof(entrance_table) / sizeof(entrance_table_entry_t); l++) {
                if (entrance_table[l].entrance == ptr_VAR_SPACE[idx]) {
                    toEntrance = entrance_table[l].name;
                    break;
                }
            }

            if (s >= s_scroll_idx && toIdx >= 0 && fromEntrance && toEntrance && (s_entrances_visited[toIdx] || (s_display_mode == DISPLAYMODE_ALL || s_display_mode == DISPLAYMODE_ALL_DEST))) {
                int count = 0;

                text_print(fromEntrance, left, top);

                top += ((icon_size + padding)) + 1;

                count = text_print("Leads to: ", left, top);
                char *ptr_toEntrance = toEntrance;
                while (*ptr_toEntrance != '-' || *(ptr_toEntrance+1) != '>' || *(ptr_toEntrance+2) != ' ') {
                    if (*ptr_toEntrance == 0) break;
                    ptr_toEntrance++;
                }

                if (*ptr_toEntrance == 0) {
                    ptr_toEntrance = toEntrance;
                } else {
                    ptr_toEntrance += 3;
                }

                text_print(ptr_toEntrance, left + font_sprite.tile_w * count + padding, top);
                k++;
            }

            if (toIdx >= 0 && fromEntrance && toEntrance && (s_entrances_visited[toIdx] || s_display_mode == DISPLAYMODE_ALL || s_display_mode == DISPLAYMODE_ALL_DEST)) s++;
        }
    }

    // Finish

    text_flush(db);

    gDPFullSync(db->p++);
    gSPEndDisplayList(db->p++);
}


