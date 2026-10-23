#ifndef MQ_DATA_H
#define MQ_DATA_H

#include "z64.h"

typedef struct {
    uint16_t first_run;
    uint16_t data_offset;
    uint16_t run_count;
    uint8_t scene_id;
    int8_t room_id;
} mq_file_patch_t;

typedef struct {
    uint32_t offset_and_length;
} mq_patch_run_t;

typedef struct {
    uint32_t vrom_start;
    uint16_t first_run;
    uint16_t data_offset;
    uint16_t run_count;
    uint16_t reserved;
} mq_global_patch_t;

extern const uint8_t MQ_PATCH_DATA[];
extern const mq_file_patch_t MQ_PATCH_FILES[];
extern const mq_global_patch_t MQ_PATCH_GLOBALS[];
extern const mq_patch_run_t MQ_PATCH_RUNS[];
extern const uint32_t MQ_PATCH_FILE_COUNT;
extern const uint32_t MQ_PATCH_GLOBAL_COUNT;

#endif
