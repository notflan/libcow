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
CXXFLAGS += $(COMMON_FLAGS) --std=gnu++20 -fno-exceptions
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
release: | dirs $(PROJECT)-release.a
	-$(MAKE) clean-rebuild
	-$(MAKE) dirs
	$(MAKE) $(PROJECT)-release.so

.PHONY: debug
debug: | dirs $(PROJECT)-debug.a
	-$(MAKE) clean-rebuild
	-$(MAKE) dirs
	$(MAKE) $(PROJECT)-debug.so

# Targets

dirs:
	@mkdir -p obj/c{,xx}/src

obj/c/%.o: %.c
	$(CC) -c $< $(CFLAGS) -o $@ $(LDFLAGS)

obj/cxx/%.o: %.cpp
	$(CXX) -c $< $(CXXFLAGS) -o $@ $(LDFLAGS)

$(PROJECT)-release.a: CFLAGS+= $(RELEASE_CFLAGS)
$(PROJECT)-release.a: CXXFLAGS += $(RELEASE_CXXFLAGS)
$(PROJECT)-release.a: LDFLAGS += $(RELEASE_LDFLAGS)
$(PROJECT)-release.a: $(OBJ)
	ar rcs $@ $^
	ln -sf $@ $(PROJECT).a

$(PROJECT)-debug.a: CFLAGS+= $(DEBUG_CFLAGS)
$(PROJECT)-debug.a: CXXFLAGS += $(DEBUG_CXXFLAGS)
$(PROJECT)-debug.a: LDFLAGS += $(DEBUG_LDFLAGS)
$(PROJECT)-debug.a: $(OBJ)
	ar rcs $@ $^

$(PROJECT)-release.so: CFLAGS+= $(RELEASE_CFLAGS) -fPIC
$(PROJECT)-release.so: CXXFLAGS += $(RELEASE_CXXFLAGS) -fPIC
$(PROJECT)-release.so: LDFLAGS += $(RELEASE_LDFLAGS)
$(PROJECT)-release.so: $(OBJ)
	$(CXX) -shared $^ -o $@
	$(STRIP) $@
	ln -sf $@ $(PROJECT).so

$(PROJECT)-debug.so: CFLAGS+= $(DEBUG_CFLAGS) -fPIC
$(PROJECT)-debug.so: CXXFLAGS += $(DEBUG_CXXFLAGS) -fPIC
$(PROJECT)-debug.so: LDFLAGS += $(DEBUG_LDFLAGS)
$(PROJECT)-debug.so: $(OBJ)
	$(CXX) -shared $^ -o $@

clean-rebuild:
	rm -rf obj

clean: clean-rebuild
	rm -f $(PROJECT){,-{release,debug,pgo}}.{a,so}

