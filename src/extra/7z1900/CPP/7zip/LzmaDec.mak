!IF "$(PLATFORM)" == "x64"
CFLAGS_C_SPEC = -D_LZMA_DEC_OPT
ASM_OBJS = $(ASM_OBJS) \
  $O\LzmaDecOpt.obj
!ENDIF
