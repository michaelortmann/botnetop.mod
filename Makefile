# Makefile for src/mod/botnetop.mod/
# $Id: Makefile,v 1.4 2002/07/14 09:28:44 sup Exp $

srcdir = .


doofus:
	@echo ""
	@echo "Let's try this from the right directory..."
	@echo ""
	@cd ../../../ && make

static: ../botnetop.o

modules: ../../../botnetop.$(MOD_EXT)

../botnetop.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -DMAKING_MODS -c $(srcdir)/botnetop.c
	@rm -f ../botnetop.o
	mv botnetop.o ../

../../../botnetop.$(MOD_EXT): ../botnetop.o
	$(LD) -o ../../../botnetop.$(MOD_EXT) ../botnetop.o $(XLIBS)
	$(STRIP) ../../../botnetop.$(MOD_EXT)

depend:
	$(CC) $(CFLAGS) $(CPPFLAGS) -MM $(srcdir)/botnetop.c > .depend

clean:
	@rm -f .depend *.o *.$(MOD_EXT) *~
distclean: clean

#safety hash
../botnetop.o: .././botnetop.mod/botnetop.c ../../../src/mod/module.h \
 ../../../src/main.h ../../../config.h \
 ../../../src/eggdrop.h ../../../src/flags.h ../../../src/proto.h \
 ../../../lush.h ../../../src/misc_file.h ../../../src/cmdt.h \
 ../../../src/tclegg.h ../../../src/tclhash.h ../../../src/chan.h \
 ../../../src/users.h ../../../src/compat/compat.h \
 ../../../src/compat/inet_aton.h ../../../src/compat/snprintf.h \
 ../../../src/compat/memset.h ../../../src/compat/memcpy.h \
 ../../../src/compat/strcasecmp.h ../../../src/compat/strftime.h \
 ../../../src/mod/modvals.h ../../../src/tandem.h \
 ../botnetop.mod/botnetop.h
