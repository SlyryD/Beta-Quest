CC = mips64-gcc
LD = mips64-ld
OBJDUMP = mips64-objdump
HOST_CC ?= gcc

CFLAGS = -O1 -fno-reorder-blocks -march=vr4300 -mtune=vr4300 -mabi=32 -mno-gpopt -mdivide-breaks \
	-mexplicit-relocs
CPPFLAGS = -DF3DEX_GBI_2

OUTDIR := build
OBJDIR := build/bin
SRCDIR := c
vpath %.c c
vpath %.h c

OBJECTS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(sort $(wildcard $(SRCDIR)/*.c)))

.PHONY: all clean bundle symbols makesrc test

all: clean bundle symbols makesrc

$(OBJDIR)/%.o: %.c 
	$(CC) -o $@ -c $< $(CFLAGS) $(CPPFLAGS)
ifdef RUN_OBJDUMP
		$(OBJDUMP) -d $@ | tr -d '\015' > $@_d.txt
		$(OBJDUMP) -r $@ | tr -d '\015' > $@_r.txt
endif

$(OBJDIR): 
	mkdir -p $@

$(OBJECTS): | $(OBJDIR)

bundle: $(OBJECTS)
	$(LD) -o $(OUTDIR)/bundle.o -i -L. $(patsubst %.o,-l:%.o,$(OBJECTS))

symbols: bundle
	$(OBJDUMP) -t $(OUTDIR)/bundle.o | tr -d '\015' > $(OUTDIR)/c_symbols.txt
ifdef RUN_OBJDUMP
		$(OBJDUMP) -d $(OUTDIR)/bundle.o | tr -d '\015' > $(OUTDIR)/bundle_d.txt
		$(OBJDUMP) -r $(OUTDIR)/bundle.o | tr -d '\015' > $(OUTDIR)/bundle_r.txt
endif

makesrc:
	$(MAKE) -C src

clean:
	rm -f $(OBJDIR)/*.o

test:
	$(HOST_CC) -std=c11 -Wall -Wextra -Werror tests/count_save_test.c -o $(OUTDIR)/count_save_test
	$(HOST_CC) -std=c11 -Wall -Wextra -Werror tests/count_tracker_test.c -o $(OUTDIR)/count_tracker_test
	$(HOST_CC) -std=c11 -Wall -Wextra -Werror tests/counts_test.c -o $(OUTDIR)/counts_test
	$(OUTDIR)/count_save_test
	$(OUTDIR)/count_tracker_test
	$(OUTDIR)/counts_test
