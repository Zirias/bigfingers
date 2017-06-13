bigfingers_MODULES:= main
bigfingers_RESFILES:= led_lit.bmp led_unlit.bmp finger.bmp
bigfingers_posix_LIBS:= SDL2main SDL2
bigfingers_win32_STATICLIBS:= mingw32 SDL2main SDL2 m dinput8 dxguid dxerr8 user32 gdi32 winmm imm32 ole32 oleaut32 shell32 version uuid
bigfingers_win32_LDFLAGS := -static-libgcc -mwindows

$(call binrules,bigfingers)

