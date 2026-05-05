.SUFFIXES: .c

CFLAGS=-Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer -I.

LXDIALOG := lxdialog/checklist.o lxdialog/inputbox.o lxdialog/menubox.o lxdialog/textbox.o lxdialog/util.o lxdialog/yesno.o

common-objs     := confdata.o expr.o lexer.lex.o menu.o parser.tab.o \
                   preprocess.o symbol.o util.o

conf: conf.o $(common-objs)

mconf_CFLAGS = $(shell cat mconf-cflags.vars)
mconf_LDFLAGS = $(shell cat mconf-libs.vars)
mconf: CFLAGS += $(mconf_CFLAGS)

mconf-cflags.vars mconf-libs.vars:
	HOSTPKG_CONFIG=pkg-config ./mconf-cfg.sh mconf-cflags.vars mconf-libs.vars

mconf: mconf-cflags.vars mconf-libs.vars
mconf: mconf.o mnconf-common.o $(common-objs) $(LXDIALOG)
	$(CC) -o mconf $(filter-out %.vars,$^) $(mconf_LDFLAGS)

.PHONY: clean
clean:
	@rm -f mconf conf *.o lxdialog/*.o

lexer.lex.c: parser.tab.h

%.tab.h %.tab.c: %.y
	bison -o $(basename $@).c --defines=$(basename $@).h -t -l $<

%.lex.c: %.l
	flex -o$@ -L $<
