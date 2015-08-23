#
# ICD AS (C) 2007 - MAKEFILE CREATED USING CDP MAKEFILE WIZARD
#
# Make sure that the environment variable CDPBase exists. This can
# be carried out by typing echo $CDPBase at the command promt, where
# the Makefile should be run by make. Note that the Makefile Wizard
# is created for the purpose of CDP Projects. It is, however, fully
# possible to use it for other project types as well.
#
# - To add a profiler in your executable, compile and link with, -pg.
# - To open/use shared libs at runtime, compile -rdynamic and link -ldl.
# - Make sure that a valid CDPLicense.key resides in $CDPBase/License.
# - Run CDP executables as root.


ifndef CFG
CFG=Release
endif

ifndef COMPILER
COMPILER=g++
endif

ifeq "$(CFG)" "Release"
CFLAGS=  -fPIC -D_UNICODE -O2 -W -fno-inline  -march=x86-64 -D_M_IX86=500 -fexceptions -D_THREAD_SAFE -D_LINUX  -DNDEBUG -D_CONSOLE
LIBS= -lboost_thread  -lboost_system -lboost_date_time -lboost_filesystem -lCommonServer -lpthread
ILFLAGS+= -I../Include 
TARGET=linuxserver
endif


SOURCE_FILES= 	\
	linuxserver.cpp

HEADER_FILES= 	\
	Common.h

OBJS=$(patsubst %.cpp, %.o,$(SOURCE_FILES))

.PHONY:all
all: $(TARGET)

$(TARGET): $(OBJS)
	$(COMPILER) -fPIC $(CFLAGS) $(ILFLAGS) $(GLOBALS) $(OBJS) -o $(TARGET) $(LIBS)

%.o: %.cpp
	$(COMPILER) -fPIC $(CFLAGS) $(ILFLAGS) $(GLOBALS) -o $@ -c $<

.PHONY:clean
clean:
	 -rm -f $(OBJS) $(TARGET)

