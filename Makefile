.POSIX:
.SUFFIXES:

include config.mk

all: build/queequeg

# local includes
CXXFLAGS += -I./lib/
CXXFLAGS += -I.

SRCS =\
  browser/main\
  html_parser/tokenizer\
  html_parser/treebuilder\
  html_parser/parser\

OBJS = $(patsubst %,build/%.o,$(SRCS))
DEPS = $(OBJS:.o=.d)

-include $(DEPS)

$(OBJS): config.mk Makefile

build/queequeg: $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS) $(LIBS)

build/%.o: %.cc
	@mkdir -p $(@D)
	$(CXX) -c -o $@ -MMD $(CXXFLAGS) $<

clean:
	rm -rf build

.PHONY: clean

