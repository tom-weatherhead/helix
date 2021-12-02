# See 'The GNU Make Manual' at
# https://www.gnu.org/software/make/manual/make.html

# Note: To link the BigNum class as a dylib (DLL) :
# $ ld -dylib -lc -lc++ -o outputfilename BigNum.o Version.o

# Run via one of the following:
# $ make
# $ make clean

MAIN := helix
all:: $(MAIN)

CC := gcc
CFLAGS := -g -IInclude -O2 -Wall
# CPPFLAGS := ?
# -lc links in the standard C library, I believe.
# -lc++ links in the standard C++ library, I believe.
LIBS := -lc++
LINK := gcc
RM := rm -f

CPPFILES := $(wildcard *.cpp)
HEADERFILES := Include/$(wildcard *.h)
OBJECTFILES := $(patsubst %.cpp,%.o,$(CPPFILES))

%.o: %.cpp $(HEADERFILES)
	$(CC) $(CFLAGS) -c $<

$(MAIN): $(OBJECTFILES)
	$(LINK) $(LIBS) -o $@ $(OBJECTFILES)

clean:
	@$(RM) $(MAIN) $(OBJECTFILES)
