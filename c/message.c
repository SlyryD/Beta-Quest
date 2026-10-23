#include "count_tracker.h"
#include "counts.h"
#include "dungeon_info.h"
#include "stdbool.h"
#include "world_map_info.h"
#include "z64.h"

static void Message_AddCharacter(z64_message_ctxt_t* msgCtx, void* pFont, uint32_t* pDecodedBufPos,
                                 uint32_t* pCharTexIdx, uint8_t charToAdd) {
    uint32_t decodedBufPosVal = *pDecodedBufPos;
    uint32_t charTexIdx = *pCharTexIdx;
    msgCtx->msg_buf_decoded[decodedBufPosVal++] = charToAdd;
    if (charToAdd != ' ') {
        z64_FontLoadChar(pFont, charToAdd - ' ', charTexIdx);
        charTexIdx += 0x80;
    }

    *pDecodedBufPos = decodedBufPosVal;
    *pCharTexIdx = charTexIdx;
}

static void Message_AddInteger(z64_message_ctxt_t* msgCtx, void* pFont, uint32_t* pDecodedBufPos, uint32_t* pCharTexIdx,
                               uint32_t numToAdd) {
    uint8_t digits[10];
    uint8_t i = 0;

    do {
        digits[i] = numToAdd % 10;
        numToAdd = numToAdd / 10;
        i++;
    } while (numToAdd > 0);

    for (uint8_t c = i; c > 0; c--) {
        Message_AddCharacter(msgCtx, pFont, pDecodedBufPos, pCharTexIdx, '0' + digits[c - 1]);
    }
}

static bool decode_dungeon_count(z64_message_ctxt_t* msgCtx, z64_font_t* pFont, char* msgRaw, uint32_t* pDecodedBufPos,
                                 uint32_t* pCharTexIdx) {
    uint8_t dungeon_index = msgRaw[++(msgCtx->msg_buf_pos)];
    uint8_t count_index = msgRaw[++(msgCtx->msg_buf_pos)];

    if (dungeon_index == 0xFF) {
        dungeon_index = z64_file.minimap_index;
    }

    uint8_t count = get_dungeon_count(dungeon_index, count_index);
    if (count == 0xFF) {
        return false;
    }

    Message_AddInteger(msgCtx, pFont, pDecodedBufPos, pCharTexIdx, count);
    (*pDecodedBufPos)--;
    return true;
}

static bool decode_area_count(z64_message_ctxt_t* msgCtx, z64_font_t* pFont, char* msgRaw, uint32_t* pDecodedBufPos,
                              uint32_t* pCharTexIdx) {
    uint8_t area_index = msgRaw[++(msgCtx->msg_buf_pos)];
    uint8_t count_index = msgRaw[++(msgCtx->msg_buf_pos)];
    uint8_t count = get_area_count(area_index, count_index);

    if (count == 0xFF) {
        return false;
    }

    Message_AddInteger(msgCtx, pFont, pDecodedBufPos, pCharTexIdx, count);
    (*pDecodedBufPos)--;
    return true;
}

static bool decode_global_count(z64_message_ctxt_t* msgCtx, z64_font_t* pFont, char* msgRaw, uint32_t* pDecodedBufPos,
                                uint32_t* pCharTexIdx) {
    uint8_t count_index = msgRaw[++(msgCtx->msg_buf_pos)];

    count_tracker_update_chests();

    uint8_t count = get_global_count(count_index);
    if (count == 0xFF) {
        return false;
    }

    Message_AddInteger(msgCtx, pFont, pDecodedBufPos, pCharTexIdx, count);
    (*pDecodedBufPos)--;
    return true;
}

bool Message_Decode_Additional_Control_Codes(uint8_t currChar, uint32_t* pDecodedBufPos, uint32_t* pCharTexIdx) {
    z64_message_ctxt_t* msgCtx = &z64_game.msg_ctxt;
    z64_font_t* pFont = &msgCtx->font;
    char* msgRaw = (char*)&pFont->msg_buf;

    switch (currChar) {
        case 0xF1:
            return decode_dungeon_count(msgCtx, pFont, msgRaw, pDecodedBufPos, pCharTexIdx);
        case 0xF4:
            return decode_area_count(msgCtx, pFont, msgRaw, pDecodedBufPos, pCharTexIdx);
        case 0xF5:
            return decode_global_count(msgCtx, pFont, msgRaw, pDecodedBufPos, pCharTexIdx);
        default:
            return false;
    }
}
