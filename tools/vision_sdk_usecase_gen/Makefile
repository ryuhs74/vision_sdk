#SHELL=C:/Windows/System32/cmd.exe
OBJDIR = objs
OBJS = parse.tab.o link.o processor.o usecase.o main.o options.o scan.o support.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(OBJS))
HEADERS_CORE = csdk-ctx.hh

#BUILD_OS=Linux

CC = g++
BISON = bison
FLEX = flex
ifeq ($(BUILD_OS),Linux)
  TARGET = ./bin/vsdk_linux.out
else
  TARGET = ./bin/vsdk_win32.exe
endif

$(TARGET):  depend $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET)
	cp $(TARGET) ../../build/scripts/

depend:
	-mkdir objs
	-mkdir bin

$(OBJDIR)/%.o: %.cc
	$(CC) -c  $< -o $@

$(OBJDIR)/%.o: %.cpp
	$(CC) -c  $< -o $@

scan.cpp: scan.ll
	$(FLEX) -oscan.cpp scan.ll

parse.tab.cc parse.tab.hh: parse.yy
	$(BISON) -vd parse.yy

init:
	@rm -rf objs
	@mkdir objs

all: init $(TARGET)

clean:
	-rm -rf objs
	-rm scan.cpp parse.tab.cc parse.tab.hh parse.output location.hh position.hh stack.hh

