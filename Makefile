# Generic C and C++ Makefile project template
# Contains targets for `release', `debug', and `clean'.

PROJECT=cow
AUTHOR=Avril (Flanchan) <flanchan@cumallover.me>

SRC_C   = $(wildcard src/*.c)
SRC_CXX = $(wildcard src/*.cpp)

INCLUDE=include

COMMON_FLAGS= -W -Wall -pedantic -fno-strict-aliasing $(addprefix -I,$(INCLUDE))

MARCH?=native
OPT_FLAGS?= $(addprefix -march=,$(MARCH)) -fgraphite -fopenmp -floop-parallelize-all -ftree-parallelize-loops=4 \
	    -floop-interchange -ftree-loop-distribution -floop-strip-mine -floop-block \
	    -fno-stack-check

CXX_OPT_FLAGS?= $(OPT_FLAGS) -felide-constructors

CFLAGS   += $(COMMON_FLAGS) --std=gnu11
CXXFLAGS += $(COMMON_FLAGS) --std=gnu++20 #-fno-exceptions
LDFLAGS  +=  

STRIP=strip

RELEASE_CFLAGS?=   -O3 -flto $(OPT_FLAGS)
RELEASE_CXXFLAGS?= -O3 -flto $(CXX_OPT_FLAGS)
RELEASE_LDFLAGS?=  -O3 -flto

DEBUG_CFLAGS?= -O0 -g -DDEBUG
DEBUG_CXXFLAGS?= $(DEBUG_CFLAGS)
DEBUG_LDFLAGS?=

# Objects

OBJ_C   = $(addprefix obj/c/,$(SRC_C:.c=.o))
OBJ_CXX = $(addprefix obj/cxx/,$(SRC_CXX:.cpp=.o))
OBJ = $(OBJ_C) $(OBJ_CXX)

# Phonies

.PHONY: release
release: | dirs lib$(PROJECT)-release.a
	-$(MAKE) clean-rebuild
	-$(MAKE) dirs
	$(MAKE) lib$(PROJECT)-release.so

.PHONY: debug
debug: | dirs lib$(PROJECT)-debug.a
	-$(MAKE) clean-rebuild
	-$(MAKE) dirs
	$(MAKE) lib$(PROJECT)-debug.so

# Rebuild both release and debug targets from scratch
.PHONY: all
all: | clean
	-$(MAKE) release
	-$(MAKE) clean-rebuild
	-$(MAKE) debug

# Targets

dirs:
	@mkdir -p obj/c{,xx}/src

obj/c/%.o: %.c
	$(CC) -c $< $(CFLAGS) -o $@ $(LDFLAGS)

obj/cxx/%.o: %.cpp
	$(CXX) -c $< $(CXXFLAGS) -o $@ $(LDFLAGS)

lib$(PROJECT)-release.a: CFLAGS+= $(RELEASE_CFLAGS)
lib$(PROJECT)-release.a: CXXFLAGS += $(RELEASE_CXXFLAGS)
lib$(PROJECT)-release.a: LDFLAGS += $(RELEASE_LDFLAGS)
lib$(PROJECT)-release.a: $(OBJ)
	ar rcs $@ $^
	ln -sf $@ lib$(PROJECT).a

lib$(PROJECT)-debug.a: CFLAGS+= $(DEBUG_CFLAGS)
lib$(PROJECT)-debug.a: CXXFLAGS += $(DEBUG_CXXFLAGS)
lib$(PROJECT)-debug.a: LDFLAGS += $(DEBUG_LDFLAGS)
lib$(PROJECT)-debug.a: $(OBJ)
	ar rcs $@ $^

lib$(PROJECT)-release.so: CFLAGS+= $(RELEASE_CFLAGS) -fPIC
lib$(PROJECT)-release.so: CXXFLAGS += $(RELEASE_CXXFLAGS) -fPIC
lib$(PROJECT)-release.so: LDFLAGS += $(RELEASE_LDFLAGS)
lib$(PROJECT)-release.so: $(OBJ)
	$(CXX) -shared $^ -o $@
	$(STRIP) $@
	ln -sf $@ lib$(PROJECT).so

lib$(PROJECT)-debug.so: CFLAGS+= $(DEBUG_CFLAGS) -fPIC
lib$(PROJECT)-debug.so: CXXFLAGS += $(DEBUG_CXXFLAGS) -fPIC
lib$(PROJECT)-debug.so: LDFLAGS += $(DEBUG_LDFLAGS)
lib$(PROJECT)-debug.so: $(OBJ)
	$(CXX) -shared $^ -o $@

clean-rebuild:
	rm -rf obj

clean: clean-rebuild
	rm -f lib$(PROJECT){,-{release,debug,pgo}}.{a,so}

