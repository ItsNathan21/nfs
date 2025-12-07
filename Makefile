name := -o server
dbgName := -o server_dbg
baseFlags := -march=native -flto -fstrict-aliasing 
flags := $(baseFlags) $(name) -O3
dbgFlags := $(baseFlags) $(dbgName) -O0
SRCDIRS = main push store common
files := $(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.c))

all:
	gcc $(flags) $(files)
	gcc $(dbgFlags) $(files)