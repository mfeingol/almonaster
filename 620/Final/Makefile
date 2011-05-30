SRCS =
SUBDIRS               = Osal AlajarDll Alajar Admin Database Almonaster DBConv AlmonasterHook
OUTPUT                = 
DEFINES               = -D__LINUX__
INCLUDE_PATH          = 
LIBRARIES             = 
LIBDIR                = .
OBJS     = $(SRCS:.cpp=.o)
CXX       = g++
CXXFLAGS  = -g $(DEFINES) $(INCLUDE_PATH)
RM        = rm -f



all: $(OUTPUT) $(SUBDIRS) 

# recursively convert all files from dos CRLF to unix LF
# some compilers can't correctly handle CRLF line breaks
convert:
	@echo Converting all source files to use Unix newlines
	@find . \( -name "*.cpp" -o -name "*.h" \) -exec sh -c "mv {} {}.old; cat {}.old | tr -d '\015\032' > {}" \;

# Implicit rules

.SUFFIXES: .cpp

.cpp.o:
	$(CXX) -c $(CXXFLAGS) -o $@ $<

.PHONY: all clean $(SUBDIRS)

clean::  $(SUBDIRS:%=%/__clean__)
	$(RM) $(OBJS) $(OUTPUT)

$(SUBDIRS:%=%/__clean__):
	@cd `dirname $@` && $(MAKE) clean

depend::  $(SUBDIRS:%=%/__depend__)

$(SUBDIRS:%=%/__depend__):
	@cd `dirname $@` && $(MAKE) depend

$(SUBDIRS):
	@cd $@ && $(MAKE)
