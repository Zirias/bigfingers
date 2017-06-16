SINGLECONFVARS:=UPX
LISTCONFVARS:=UPXFLAGS

DEFAULT_UPX := upx
DEFAULT_UPXFLAGS := --lzma

include zimk/zimk.mk
$(call zinc,src/src.mk)

