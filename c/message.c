#include "message.h"
#include "z64.h"
#include "counts.h"
#include "dungeon_info.h"
#include "world_map_info.h"
#include "save.h"
#include "stdbool.h"

// no support for kana since they're not part of the message charset
char FILENAME_ENCODING[256] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '?', '?', '?', '?', '?', '?',
    '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?',
    '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?',
    '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?',
    '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?',
    '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?',
    '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?',
    '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?',
    '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?',
    '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?',
    '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', 'A', 'B', 'C', 'D', 'E',
    'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U',
    'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k',
    'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', ' ',
    '?', '?', '!', ':', '-', '(', ')', '?', '?', ',', '.', '/', '?', '?', '?', '?',
    '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?',
};

extern uint8_t PLAYER_NAMES[256][8];
extern uint8_t PLAYER_NAME_ID;

// Helper function for adding characters to the decoded message buffer
void Message_AddCharacter(MessageContext* msgCtx, void* pFont, uint32_t* pDecodedBufPos, uint32_t* pCharTexIdx, uint8_t charToAdd) {
    uint32_t decodedBufPosVal = *pDecodedBufPos;
    uint32_t charTexIdx = *pCharTexIdx;
    msgCtx->msgBufDecoded[decodedBufPosVal++] = charToAdd; // Add the character to the output buffer, increment the output position
    if (charToAdd != ' ') { // Only load the character texture if it's not a space.
        Font_LoadChar(pFont, charToAdd - ' ', charTexIdx); // Load the character texture
        charTexIdx += 0x80; // Increment the texture pointer
    }

    // Copy our locals back to their pointers
    *pDecodedBufPos = decodedBufPosVal;
    *pCharTexIdx = charTexIdx;
}

// Helper function for adding integer numbers to the decoded message buffer
void Message_AddInteger(MessageContext* msgCtx, void* pFont, uint32_t* pDecodedBufPos, uint32_t* pCharTexIdx, uint32_t numToAdd) {
    uint8_t digits[10];
    uint8_t i = 0;
    // Extract each digit. They are added, in reverse order, to digits[]
    do {
        digits[i] = numToAdd % 10;
        numToAdd = numToAdd / 10;
        i++;
    }
    // Loop through each digit in digits[] and add the character to the decoded buffer.
    while (numToAdd > 0);

    for (uint8_t c = i; c > 0; c--) {
        Message_AddCharacter(msgCtx, pFont, pDecodedBufPos, pCharTexIdx, '0' + digits[c - 1]);
    }
}

// Helper function for adding simple strings to the decoded message buffer. Does not support additional control codes.
void Message_AddString(MessageContext* msgCtx, void* pFont, uint32_t* pDecodedBufPos, uint32_t* pCharTexIdx, char* stringToAdd) {
    while (*stringToAdd != 0) {
        Message_AddCharacter(msgCtx, pFont, pDecodedBufPos, pCharTexIdx, *stringToAdd);
        stringToAdd++;
    }
}

// Helper function for adding a filename to the decoded message buffer. Filenames use a different character set from other text.
void Message_AddFileName(MessageContext* msgCtx, void* pFont, uint32_t* pDecodedBufPos, uint32_t* pCharTexIdx, uint8_t* filenameToAdd) {
    int end = 8;
    while (filenameToAdd[end - 1] == 0xDF) {
        // trim trailing space
        end--;
    }
    for (int i = 0; i < end; i++) {
        Message_AddCharacter(msgCtx, pFont, pDecodedBufPos, pCharTexIdx, FILENAME_ENCODING[filenameToAdd[i]]);
    }
}

// Function that handles dungeon item counts
bool decode_dungeon_item_count(MessageContext* msgCtx, Font* pFont, char* msgRaw, uint32_t* pDecodedBufPos, uint32_t* pCharTexIdx) {
    // Get the next character that tells us which dungeon
    uint8_t dungeon_index = msgRaw[++(msgCtx->msgBufPos)];

    // Get the next character that tells us which item
    uint8_t item_index = msgRaw[++(msgCtx->msgBufPos)];

    // Get the count from the save context
    uint8_t count = get_dungeon_item_count(dungeon_index, item_index);
    if (count == 0xFF) {
        return false;
    }

    Message_AddInteger(msgCtx, pFont, pDecodedBufPos, pCharTexIdx, count);
    (*pDecodedBufPos)--;
    return true;
}

// Function that handles area item counts
bool decode_area_item_count(MessageContext* msgCtx, Font* pFont, char* msgRaw, uint32_t* pDecodedBufPos, uint32_t* pCharTexIdx) {
    // Get the next character that tells us which area
    uint8_t area_index = msgRaw[++(msgCtx->msgBufPos)];

    // Get the next character that tells us which item
    uint8_t item_index = msgRaw[++(msgCtx->msgBufPos)];

    // Get the count from the save context
    uint8_t count = get_area_item_count(area_index, item_index);
    if (count == 0xFF) {
        return false;
    }

    Message_AddInteger(msgCtx, pFont, pDecodedBufPos, pCharTexIdx, count);
    (*pDecodedBufPos)--;
    return true;
}

bool decode_item_count(MessageContext* msgCtx, Font* pFont, char* msgRaw, uint32_t* pDecodedBufPos, uint32_t* pCharTexIdx) {
    // Get the next character that tells us which item
    uint8_t item_index = msgRaw[++(msgCtx->msgBufPos)];

    // Get the count from the save context
    uint8_t count = get_item_count(item_index);
    if (count == 0xFF) {
        return false;
    }

    Message_AddInteger(msgCtx, pFont, pDecodedBufPos, pCharTexIdx, count);
    (*pDecodedBufPos)--;
    return true;
}

// Hack to add additional text control codes.
// If additional codes need to be read after the primary code, increment msgCtx->msgBufPos and index msgRaw
// To add a new control code:
//      Compare currChar to the control code.
//          Add text to the output buffer by performing the following:
//          Call one of the above functions to add the text.
//          Subtract 1 from* pDecodedBufPos
//          Return true
bool Message_Decode_Additional_Control_Codes(uint8_t currChar, uint32_t* pDecodedBufPos, uint32_t* pCharTexIdx) {
    MessageContext* msgCtx = &(z64_game.msgContext);
    Font* pFont = &(msgCtx->font); // Get a reference to the font.
    char* msgRaw = (char*) &(pFont->msgBuf); // Get a reference to the start of the raw message. Index using msgCtx->msgBufPos.

    switch (currChar) {
        case 0xF1: {
            return decode_dungeon_item_count(msgCtx, pFont, msgRaw, pDecodedBufPos, pCharTexIdx);
        }
        case 0xF4: {
            return decode_area_item_count(msgCtx, pFont, msgRaw, pDecodedBufPos, pCharTexIdx);
        }
        case 0xF5: {
            return decode_item_count(msgCtx, pFont, msgRaw, pDecodedBufPos, pCharTexIdx);
        }
        default: {
            return false;
        }
    }
}

uint8_t shooting_gallery_show_message = 0;
// Displays a warning message if player did adult shooting gallery without bow.
void shooting_gallery_message() {
    // Child/Adult shooting galleries actor is the same, so the asm hook will work for both.
    // We only want the message for Adult.
    if (!LINK_IS_ADULT) {
        return;
    }
    // Check if we have a bow.
    if (z64_file.items[ITEM_BOW] != ITEM_NONE) {
        return;
    }
    // Check if the message was already displayed once.
    if (shooting_gallery_show_message != 0) {
        return;
    }
    shooting_gallery_show_message = 1;
}

uint8_t treasure_chest_game_show_message = 0;
// Displays a warning message if the player attempted the Treasure Chest Game without
// Lens of Truth when settings require it.
void treasure_chest_game_message() {
    if (z64_file.items[Z64_SLOT_LENS] != Z64_ITEM_LENS || !z64_file.magic_acquired) {
        treasure_chest_game_show_message = 1;
    }
}

// Function to display custom textboxes ingame.
void display_misc_messages() {
    if (z64_MessageGetState(((uint8_t*) (&z64_game)) + 0x20D8) == 0) {
        // Each minigame warning message can only be triggered in their respective
        // scenes. Order doesn't matter.
        if (shooting_gallery_show_message == 1) {
            z64_DisplayTextbox(&z64_game, 0x045C, 0);
            // To avoid displaying the message several times if the player just wants to farm the 50 rupees.
            shooting_gallery_show_message = -1;
        } else if (treasure_chest_game_show_message) {
            z64_DisplayTextbox(&z64_game, 0x045D, 0);
            // No reason not to repeat the message on a reattempt in case the player forgot.
            treasure_chest_game_show_message = 0;
        }
    }
}
