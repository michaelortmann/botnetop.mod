# Makefile for src/mod/botnetop.mod/

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
botnetop.o: .././botnetop.mod/botnetop.c .././botnetop.mod/../module.h \
 ../../../src/main.h ../../../config.h ../../../eggint.h ../../../lush.h \
 ../../../src/lang.h ../../../src/eggdrop.h ../../../src/compat/in6.h \
 ../../../src/flags.h ../../../src/cmdt.h ../../../src/tclegg.h \
 ../../../src/tclhash.h ../../../src/chan.h ../../../src/users.h \
 ../../../src/compat/compat.h ../../../src/compat/snprintf.h \
 ../../../src/compat/strlcpy.h .././botnetop.mod/../modvals.h \
 ../../../src/tandem.h .././botnetop.mod/../irc.mod/irc.h \
 .././botnetop.mod/../server.mod/server.h \
 .././botnetop.mod/../channels.mod/channels.h \
 .././botnetop.mod/botnetop.h .././botnetop.mod/botbinds.c \
 .././botnetop.mod/botcmds.c .././botnetop.mod/delay.c \
 .././botnetop.mod/flood.c .././botnetop.mod/ircbinds.c \
 .././botnetop.mod/misc.c .././botnetop.mod/request.c \
 .././botnetop.mod/tclcmds.c .././botnetop.mod/who.c
