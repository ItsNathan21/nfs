name := -o nfs_sys
dbgName := -o nfs_dbg
baseFlags := -march=native -fstrict-aliasing -g
flags := $(baseFlags) $(name) -O3 -flto
dbgFlags := $(baseFlags) $(dbgName) -O0
SRCDIRS = main server common logging
files := $(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.c))

all:
	gcc $(flags) $(files)
	gcc $(dbgFlags) $(files)