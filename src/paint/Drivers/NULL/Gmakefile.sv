NAME=NULL
LIBS = $(SRC)/paint/Interface/driverlib/lib.a $(LOCKLIB)
PAINT = $(ETC)/paint
LIST = main.o alpha.o close.o data.o flush.o \
       open.o out.o pictsize.o raster.o rle.o text.o\
       init.o npixels.o finish.o

all: $(PAINT)/driver.uninst/$(NAME) $(PAINT)/driver/$(NAME)

$(PAINT)/driver/$(NAME): $(LIST) $(LIBS)
	$(CC) $(LDFLAGS) $(LIST) $(LIBS) -o $@
$(PAINT)/driver.uninst/$(NAME): DRIVER.sh
	cp $? $@
	chmod +x $@

$(LIST): P.h

$(LIBS): #
