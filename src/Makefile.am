# Makefile.am for B-em

#generated files
VPATH = $(srcdir) $(builddir)

bin_PROGRAMS = b-em hdfmt jstest gtest cpu_test1
noinst_SCRIPTS = ../b-em$(EXEEXT)

noinst_LIBRARIES = lib_m6502.a

CLEANFILES = $(noinst_SCRIPTS) $(m6502_GEN) 

../b-em$(EXEEXT): b-em$(EXEEXT)
	cp b-em$(EXEEXT) ..

amrefresh:

m6502_GEN = 	cpu/m6502/m6502.gen.h cpu/m6502/m6502_top.gen.h cpu/m6502/m6502.gen.cpp 
m65c02_GEN =	cpu/m6502/m65c02.gen.h cpu/m6502/m65c02_top.gen.h cpu/m6502/m65c02.gen.cpp
r65c02_GEN =	cpu/m6502/r65c02.gen.h cpu/m6502/r65c02_top.gen.h cpu/m6502/r65c02.gen.cpp
m65ce02_GEN =	cpu/m6502/m65ce02.gen.h cpu/m6502/m65ce02_top.gen.h cpu/m6502/m65ce02.gen.cpp

BUILT_SOURCES = $(m6502_GEN) $(m65c02_GEN) $(r65c02_GEN) $(m65ce02_GEN)

lib_m6502_a_SOURCES = cpu/m6502/m65x_device.cpp \
		cpu/m6502/m6502.cpp \
		cpu/m6502/m65ce02.cpp \
		cpu/m6502/m65sc02.cpp \
		cpu/m6502/r65c02.cpp \
		cpu/m6502/m6502.gen.cpp \
		cpu/m6502/m65c02.gen.cpp \
		cpu/m6502/r65c02.gen.cpp \
		cpu/m6502/m65ce02.gen.cpp 

lib_m6502_a_CFLAGS = -I$(srcdir) -I$(srcdir)/cpu/m6502 -I$(builddir)/cpu/m6502
lib_m6502_a_CPPFLAGS = $(lib_m6502_a_CFLAGS)


$(m6502_GEN): $(srcdir)/cpu/m6502/om6502.lst $(srcdir)/cpu/m6502/dm6502.lst
	$(PERL) $(srcdir)/cpu/m6502/makehxx.pl m6502 $(srcdir)/cpu/m6502/om6502.lst $(srcdir)/cpu/m6502/dm6502.lst $(m6502_GEN)

$(m65c02_GEN): $(srcdir)/cpu/m6502/om65c02.lst $(srcdir)/cpu/m6502/dm65c02.lst
	$(PERL) $(srcdir)/cpu/m6502/makehxx.pl m65c02 $(srcdir)/cpu/m6502/om65c02.lst $(srcdir)/cpu/m6502/dm65c02.lst $(m65c02_GEN)

$(r65c02_GEN): $(srcdir)/cpu/m6502/dr65c02.lst
	$(PERL) $(srcdir)/cpu/m6502/makehxx.pl r65c02 - $(srcdir)/cpu/m6502/dm65c02.lst $(r65c02_GEN)

$(m65ce02_GEN): $(srcdir)/cpu/m6502/om65ce02.lst $(srcdir)/cpu/m6502/dm65ce02.lst
	$(PERL) $(srcdir)/cpu/m6502/makehxx.pl m65ce02 $(srcdir)/cpu/m6502/om65ce02.lst $(srcdir)/cpu/m6502/dm65ce02.lst $(m65ce02_GEN)


# The following flags are used by PiTubeDirect Co Pros:
#                BEM - fixes for compilation in B-Em envorinment
#   INCLUDE_DEBUGGER - include the cpu_debug implementation
# USE_MEMORY_POINTER - do not assume Co Pro memory starts at address 0

b_em_CFLAGS = $(allegro_CFLAGS) -DBEM -DINCLUDE_DEBUGGER -DUSE_MEMORY_POINTER -I$(srcdir) -I$(srcdir)/cpu/m6502 -I$(builddir)/cpu/m6502
b_em_CPPFLAGS = $(b_em_CFLAGS)

cpu_test1_CFLAGS = $(allegro_CFLAGS) -DBEM -DINCLUDE_DEBUGGER -DUSE_MEMORY_POINTER -I$(srcdir) -I$(srcdir)/cpu/m6502 -I$(builddir)/cpu/m6502
cpu_test1_CPPFLAGS = $(cpu_test1_CFLAGS)

cpu_test1_SOURCES = cpu/test1/test1.cpp
cpu_test1_LDADD = lib_m6502.a

# workaround for Win32 Allegro, which has `allegro-config' missing
if OS_WIN
b_em_LDADD = lib_m6502.a -lallegro_audio -lallegro_acodec -lallegro_primitives -lallegro_dialog -lallegro_image -lallegro_font -lallegro -lz -lm
else
b_em_LDADD = lib_m6502.a -lallegro_audio -lallegro_acodec -lallegro_primitives -lallegro_dialog -lallegro_image -lallegro_font -lallegro_main -lallegro -lz -lm -lpthread
endif


#	6502.c 

b_em_SOURCES = \
	$(nodist_m6502_SOURCES) \
	$(m6502_SOURCES) \
	6502debug.c \
	6502tube.c \
	65816.c \
    6809tube.c \
	NS32016/32016.c \
	NS32016/32016_debug.c \
	NS32016/Decode.c \
	NS32016/NSDis.c \
	NS32016/Profile.c \
	NS32016/Trap.c \
	NS32016/mem32016.c \
	z80.c \
	z80dis.c \
	acia.c \
	adc.c \
	arm.c \
	darm/darm.c \
	darm/darm-tbl.c \
	darm/armv7.c \
	darm/armv7-tbl.c \
	darm/thumb.c \
	darm/thumb-tbl.c \
	darm/thumb2.c \
	darm/thumb2-decoder.c \
	darm/thumb2-tbl.c \
	cmos.c \
	compact_joystick.c \
	compactcmos.c \
	compat_wrappers.c \
	config.c \
	csw.c \
	ddnoise.c \
	debugger.c \
	disc.c fdi.c \
	fdi2raw.c \
	gui-allegro.c\
	i8271.c \
	ide.c \
	joystick.c \
	keyboard.c \
	keydef-allegro.c \
	linux.c \
	logging.c \
        mc6809nc/mc6809nc.c \
        mc6809nc/mc6809_debug.c \
        mc6809nc/mc6809_dis.c \
	main.c \
	mem.c \
	model.c \
	mouse.c \
	midi-linux.c \
	music2000.c \
	music4000.c \
	music5000.c \
	paula.c \
	pal.c\
	resid.cc \
	savestate.c \
	scsi.c \
	sdf-acc.c \
	sdf-geo.c \
	serial.c \
	sn76489.c \
	sound.c \
	sysacia.c \
	sysvia.c \
	sys.cpp \
	tape.c \
	tapecat-allegro.c \
	tapenoise.c \
	tube.c \
	uef.c \
	uservia.c \
	vdfs.c \
	via.c \
	vidalleg.c \
	video.c \
	wd1770.c \
	win.c \
	x86.c \
	x86dasm.c \
	resid-fp/convolve-sse.cc \
	resid-fp/convolve.cc \
	resid-fp/envelope.cc \
	resid-fp/extfilt.cc \
	resid-fp/filter.cc \
	resid-fp/pot.cc \
	resid-fp/sid.cc \
	resid-fp/voice.cc \
	resid-fp/wave.cc \
	resid-fp/wave6581_PST.cc \
	resid-fp/wave6581_PS_.cc \
	resid-fp/wave6581_P_T.cc \
	resid-fp/wave6581__ST.cc \
	resid-fp/wave8580_PST.cc \
	resid-fp/wave8580_PS_.cc \
	resid-fp/wave8580_P_T.cc \
	resid-fp/wave8580__ST.cc

if NO_TSEARCH
b_em_SOURCES += tsearch.c
endif

hdfmt_SOURCES = hdfmt.c

jstest_SOURCES = jstest.c

jstest_LDADD = -lallegro -lallegro_main

gtest_SOURCES = sdf-gtest.c sdf-geo.c
