bigfingers_MODULES:= main
bigfingers_RESFILES:= led_lit.bmp led_unlit.bmp finger.bmp
bigfingers_posix_LIBS:= SDL2main SDL2
bigfingers_win32_STATICLIBS:= mingw32 SDL2main SDL2 m dinput8 dxguid dxerr8 user32 gdi32 winmm imm32 ole32 oleaut32 shell32 version uuid
bigfingers_win32_LDFLAGS := -static-libgcc -mwindows

$(call binrules,bigfingers)

bigfingers_PACKEDDIR:= $(bigfingers_TGTDIR)$(PSEP)packed
bigfingers_PACKEDEXE:= $(bigfingers_PACKEDDIR)$(PSEP)$(bigfingers_TARGET)$(EXE)

$(bigfingers_PACKEDEXE): $(bigfingers_EXE) | $(bigfingers_PACKEDDIR) strip
	$(UPX) $(UPXFLAGS) -o$@ $<

pack:: $(bigfingers_PACKEDEXE)

ifeq ($(filter pack,$(MAKECMDGOALS)),pack)
OUTFILES:=$(bigfingers_PACKEDEXE)
$(eval $(DIRRULES))
endif
