.align 4
MQ_PATCH_FILE_COUNT:
    .word 232

MQ_PATCH_FILES:
    .incbin("../resources/mq_patch_files.bin")

MQ_PATCH_GLOBAL_COUNT:
    .word 2

MQ_PATCH_GLOBALS:
    .incbin("../resources/mq_patch_globals.bin")

MQ_PATCH_RUNS:
    .incbin("../resources/mq_patch_runs.bin")

MQ_PATCH_DATA:
    .incbin("../resources/mq_patch_data.bin")

.align 4
