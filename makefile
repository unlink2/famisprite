CC=gcc
DBG=gdb
BIN=libfamisprite

INCLUDEDIR=./src/include
SRCDIR=./src
ODIR=./obj
BINDIR=./bin

LIBS=
CFLAGS=-Wall -g
CFLAGS_RELEASE=-Wall -O1
MAIN = main
TEST_MAIN = test
INSTALLDIR = /usr/local/bin

MODULES = famisprite

DEPS=$(patsubst %,$(INCLUDEDIR)/%.h,$(MODULES))
OBJ=$(patsubst %,$(ODIR)/%.o,$(MODULES))
TEST_OBJ=$(patsubst %,$(ODIR)/%.o,$(MODULES))
TEST_OBJ+=$(patsubst %,$(ODIR)/%.o,$(TEST_MAIN))

# main

$(ODIR)/%.o: $(SRCDIR)/%.c $(DEPS) | init
	$(CC) -c -o $@ $< $(CFLAGS)

famisprite.a: $(OBJ)
	ar rcs $(BINDIR)/$@ $^

# test

$(ODIR)/%.o: $(SRCDIR)/%.c $(DEPS) | init
	$(CC) -c -o $@ $< $(CFLAGS)

build_test: $(TEST_OBJ)
	$(CC) -o $(BINDIR)/${TEST_MAIN} $^ $(LIBS) -l cmocka

test: build_test
	$(BINDIR)/$(TEST_MAIN)

leaktest: build_test
	valgrind -s $(BINDIR)/$(TEST_MAIN)

# other useful things

.PHONY: clean
clean:
	@echo Cleaning stuff. This make file officially is doing better than you irl.
	rm -f $(ODIR)/*.o
	rm -f $(BINDIR)/*

.PHONY: setup
init:
	mkdir -p $(ODIR)
	mkdir -p $(BINDIR)

.PHONY: install
install:
	cp ${BINDIR}/${BIN} ${INSTALLDIR}/${BIN}
