# Simple Makefile

PRJ:=xpr

SRCDIR:=./src
OBJDIR:=./obj
VENDORDIR:=./vendor

CC := gcc
CFLAGS := -std=c2x -I $(SRCDIR) -I $(VENDORDIR)/tgc -Wall -Wextra -g -O0 -DDEBUG -MMD -MP
LDLIBS := -lm
LDFLAGS := -L $(OBJDIR)

sources := $(wildcard $(SRCDIR)/*.c) $(shell find $(VENDORDIR) -name "*.c" -not -path "*/examples/*")
objects := $(addprefix $(OBJDIR)/, $(notdir $(sources:.c=.o)))

all: $(PRJ)

vars:
	@echo "PRJ=$(PRJ)"
	@echo "SRCDIR=$(SRCDIR)"
	@echo "OBJDIR=$(OBJDIR)"
	@echo "VENDORDIR=$(VENDORDIR)"
	@echo "sources=$(sources)"
	@echo "objects=$(objects)"
	@echo "CFLAGS=$(CFLAGS)"

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/tgc.o: tgc.c | $(OBJDIR)
	$(CC) $(filter-out -Wall -Wextra, $(CFLAGS)) -c $< -o $@

$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

VPATH := $(SRCDIR) $(shell find $(VENDORDIR) -type d -not -path "*/examples/*")

$(PRJ): $(objects)
	$(CC) $(LDFLAGS) $(objects) $(LDLIBS) -o $(PRJ)

run:
	./$(PRJ)

test: $(PRJ)
	./tests/run_tests.sh

.PHONY: all clean test vars run

clean:
	@rm -rf $(OBJDIR) $(PRJ)
