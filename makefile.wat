#
# OpenWatcom v2 build
#

.SILENT

sys=pmodew
stack_size=128k
CP=wpp386
CC=wcc386
debug=all
wcflags20=-bc -ecf
cflags=-oneatx -w3 -bc -zq -zp4 -zc -3r -zt256 $(wcflags20)
lflags=
name=ud.exe
cpu=3x

dcflags=-d0 -DNDEBUG
dlflags=

odir=out.$(CPU)

OBJS=$(odir)/main.obj $(odir)/play.obj $(odir)/screen.obj &
	$(odir)/rnd.obj $(odir)/dlist.obj $(odir)/version.obj $(odir)/message.obj &
	$(odir)/ini.obj $(odir)/scores.obj $(odir)/time.obj $(odir)/strings.obj &
	$(odir)/news.obj $(odir)/name.obj $(odir)/dgen.obj $(odir)/item.obj &
	$(odir)/keys.obj $(odir)/death.obj $(odir)/inv.obj $(odir)/monster.obj &
	$(odir)/levup.obj $(odir)/save.obj $(odir)/save_ext.obj $(odir)/load.obj &
	$(odir)/msdos.obj

################################################################################

all: $(name)

$(odir)/vers.exe: $(odir)/vers.obj
	wcl386 -lpmodew -d0 -oneatx $< -fo=$(odir)/vers.obj -fe=$@

$(odir)/bin2c.exe: $(odir)/bin2c.obj
	wcl386 -lpmodew -d0 -oneatx $< -fo=$(odir)/bin2c.obj -fe=$@

verx: $(odir)/vers.exe .symbolic
	$(odir)\\vers.exe

pre: .symbolic
	@if not exist $(odir)\con @mkdir $(odir)

inc_ini.h: $(ODIR)/bin2c.exe udark.inx
	$(ODIR)\\bin2c udark.inx inc_ini>inc_ini.h
	if exist $(ODIR)\\main.obj del $(ODIR)\\main.obj
inc_nws.h: $(ODIR)/bin2c.exe news.txt
	$(ODIR)\\bin2c news.txt inc_nws>inc_nws.h
	if exist $(ODIR)\\news.obj del $(ODIR)\\news.obj
inc_wlc.h: $(ODIR)/bin2c.exe welcome.txt
	$(ODIR)\\bin2c welcome.txt inc_wlc>inc_wlc.h
	if exist $(ODIR)\\news.obj del $(ODIR)\\news.obj

$(name): pre verx inc_ini.h inc_wlc.h inc_nws.h $(OBJS)
	%create ud.lnk
	%append ud.lnk name $(name)
	%append ud.lnk option quiet
	%append ud.lnk option stack=$(stack_size)
	%append ud.lnk $(lflags) $(dlflags)
	%append ud.lnk system $(sys)
	@for %i in ($(OBJS)) do @%append ud.lnk file %i
	@echo Linking... $(name)
	@wlink @ud.lnk
	%erase ud.lnk
	@wstrip -q -n $(name)
	@upx -9 $(name)
	@pmwsetup /L0xFFFF /M1 /B0 $(NAME) /V4 /A4

.after
	@if exist ud.lnk @del ud.lnk

.error
	@if exist ud.lnk @del ud.lnk

dist: .symbolic $(NAME)
	@if not exist dist.dos\nul mkdir dist.dos
	@del /y dist.dos\*.*>nul
	@wstrip -q -n $(name)
	@sauce -i $(name) -o dist.dos\$(name) -preset ud
	@copy readme.txt dist.dos
	@copy news.txt dist.dos
	@copy welcome.txt dist.dos
	@copy udark.inx dist.dos\udark.ini

clean: .symbolic
	@if exist $(ODIR)\*.obj del $(ODIR)\*.obj>nul
	@if exist $(name) del $(name)>nul
	@if exist *.lnk del *.lnk>nul
	@if exist *.bin del *.bin>nul
	@if exist *.sav del *.sav>nul

.cpp.obj : .AUTODEPEND
	@echo Compiling $^&
	$(cp) $(cflags) $(dcflags) $< -fo=$@

.cc.obj : .AUTODEPEND
	@echo Compiling $^&
	$(cp) $(cflags) $(dcflags) $< -fo=$@

.c.obj : .AUTODEPEND
	@echo Compiling $^&
	$(cc) $(cflags) $(dcflags) -fo=$@ $<

