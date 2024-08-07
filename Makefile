.POSIX:
.SUFFIXES:

include config.mk

all: build/queequeg

# local includes
CXXFLAGS += -I./lib/
CXXFLAGS += -I.

SRCS =\
	browser/main\
	\
	html/dom\
	html/elements\
	\
	html_parser/insertion_modes\
	html_parser/parser\
	html_parser/tokenizer\
	html_parser/tokenizer_states\
	html_parser/treebuilder\
	\
	dom/core/document\
	dom/core/element\
	dom/core/node\
	dom/core/tree_node_dfs_iterator\
	\
	qglib/unicode\

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

