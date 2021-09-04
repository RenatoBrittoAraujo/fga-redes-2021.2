CC = g++ -std=c++17
SRCDIR = src
BUILDDIR = obj
TARGET = program
 
SRCEXT = cpp

SOURCES = $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))

OBJECTS = $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))

LIB = -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf
INC = -I include -I include/engine

$(TARGET): $(OBJECTS)
	@echo " $(CC) $(INC) $^ -o $(TARGET) $(LIB)"; $(CC) $(INC) $^ -o $(TARGET) $(LIB)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	$(CC) $(INC) -c -o $@ $<

clean:
	@echo " find . -type f -name '*.o' -delete"; find . -type f -name '*.o' -delete
	@echo " rm ./$(TARGET)"; rm ./$(TARGET)

run:
	./$(TARGET)

runv:
	./$(TARGET) -v

runf:
	./$(TARGET) -f

runc:
	./$(TARGET) -c

makefiledebug:
	@echo "MAKEFILE VARIABLES: ";
	@echo "|CC| => $(CC)"
	@echo "|SRCDIR| => $(SRCDIR)"
	@echo "|BUILDDIR| => $(BUILDDIR)"
	@echo "|TARGET| => $(TARGET)"
	@echo "|SRCEXT| => $(SRCEXT)"
	@echo "|SOURCES| => $(SOURCES)"
	@echo "|OBJECTS| => $(OBJECTS)"
	@echo "|LIB| => $(LIB)"
	@echo "|INC| => $(INC)"

.PHONY: clean