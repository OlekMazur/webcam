PROGRAM	= nph-webcam.cgi
CFLAGS	+= -g -O3 -std=c99 -D_DEFAULT_SOURCE -pedantic -Wall -Wextra -Wno-variadic-macros -Wmissing-declarations -Wdeclaration-after-statement -Wformat=2 -Werror
LDFLAGS	+= -g

ifeq (,$(NO_JPEGLIB))
CFLAGS	+= -DUSE_JPEGLIB
LDLIBS	+= -ljpeg
ALL_C	= $(wildcard *.c)
else
ALL_C	= $(filter-out vff_yuv2jpeg.c,$(wildcard *.c))
endif
ALL_O	= $(patsubst %.c,%.o,$(ALL_C))
ALL_D	= $(patsubst %.c,%.d,$(ALL_C))

.PHONY:	clean all docs

all:	$(PROGRAM)

docs:	Doxyfile *.md *.c *.h
	mkdir -p doc
	doxygen $<

clean:
	rm -rf doc
	rm -f $(PROGRAM) $(ALL_O) $(ALL_D)

$(PROGRAM):	$(ALL_O)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o:	%.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.d:	%.c
	$(CC) $(CFLAGS) -MM $< -MF $@

-include $(ALL_D)
