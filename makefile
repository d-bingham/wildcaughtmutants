#CPPC = /s/gcc-4.8.3/bin/g++
CPPC = g++
CC = gcc
CFLAGS = -Wall -Wextra -Werror -O3 -c -fPIC -Wno-unused-function -Wno-unused-value -Wno-unused-result -Wno-reorder --std=c++11
LFLAGS = -Wall -Wextra -Werror -O3 -fPIC 

SRCDIR = src
OBJDIR = obj
INTDIR = int
OUTDIR = bin

TOOLS = 

MAINSRC = $(wildcard $(SRCDIR)/*_main.cpp)
EXECUTABLES = $(patsubst $(SRCDIR)/%_main.cpp,$(OUTDIR)/%,$(MAINSRC))

SRCFILES = $(filter-out $(MAINSRC),$(wildcard $(SRCDIR)/*.cpp))
OBJ = $(SRCFILES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

TOOLFILES = $(TOOLS:%=%_parser.c) $(TOOLS:%=%_parser.h) $(TOOLS:%=%_lexer.c)
TOOLINTS = $(TOOLFILES:%=$(INTDIR)/%)
TOOLSRC = $(filter %.c,$(TOOLINTS))
TOOLOBJ = $(TOOLSRC:$(INTDIR)/%.c=$(OBJDIR)/%.o)

HEADER = $(wildcard $(SRCDIR)/*.h)


.SECONDARY:

all: $(EXECUTABLES)


showvar:
	@ echo OBJ: $(OBJ)
	@ echo TOOLOBJ: $(TOOLOBJ)
	@ echo EXECUTABLES: $(EXECUTABLES)
	@ echo SRCFILES: $(SRCFILES)
	@ echo HEADER: $(HEADER)



$(OUTDIR)/%: $(TOOLOBJ) $(OBJ) $(OBJDIR)/%_main.o
	@ echo Linking $@...
	@ mkdir -p $(OUTDIR)
	@ $(CPPC) $(LFLAGS) -o $@ $^

$(OBJDIR)/%_lexer.o: $(INTDIR)/%_parser.h $(INTDIR)/%_lexer.cpp $(SRCDIR)/interop.h | $(OBJDIR)
	@ echo Compiling $* lexical analyzer
	@ $(CPPC) $(CFLAGS) -c $(INTDIR)/$*_lexer.cpp -o $(OBJDIR)/$*_lexer.o

$(INTDIR)/%_lexer.cpp: $(SRCDIR)/%.ll | $(INTDIR)
	@ echo Generating $* lexical analyzer
	@ flex -o $(INTDIR)/$*_lexer.cpp $(SRCDIR)/$*.ll

$(INTDIR)/%_lexer.c $(INTDIR)/%.tab.h: $(SRCDIR)/%.l | $(INTDIR)
	@ echo Generating $* lexical analyzer
	@ flex -P yy_$*_ --header-file=$(INTDIR)/$*.tab.h -o $(INTDIR)/$*_lexer.c $<

$(INTDIR)/%_parser.cpp $(INTDIR)/%_parser.h: $(SRCDIR)/%.yy | $(INTDIR)
	@ echo Generating $* parser
	@ bison -d -o $(INTDIR)/script_parser.cpp $(SRCDIR)/$*.yy

$(INTDIR)/%_parser.c $(INTDIR)/%_parser.h: $(SRCDIR)/%.y | $(INTDIR)
	@ echo Generating $* parser
	@ bison -p yy_$*_  --defines=$(INTDIR)/$*_parser.h -o $(INTDIR)/$*_parser.cpp -y $(SRCDIR)/$*.y 

$(OBJDIR)/%_parser.o: $(INTDIR)/%_parser.cpp $(SRCDIR)/interop.h | $(OBJDIR)
	@ echo Compiling $* parser
	@ $(CPPC) $(CFLAGS) -c $(INTDIR)/$*_parser.cpp -o $(OBJDIR)/$*_parser.o

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(HEADER) | $(OBJDIR)
	@ echo Compiling - $*
	@ $(CPPC) $(CFLAGS) $< -o $@


$(OBJDIR):
	@ mkdir -p $(OBJDIR)

$(INTDIR):
	@ mkdir -p $(INTDIR)

tools: $(TOOLINTS)

clean:
	@ rm -f $(EXE)
	@ rm -rf $(OBJDIR) 
	@ rm -rf $(INTDIR)
	@ rm -rf $(OUTDIR)

rebuild: clean all


# DO NOT DELETE
