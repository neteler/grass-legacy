
# lexical analyzer and default options
LEXFLAGS= -d -i -s -t

# parser generator and default options
YACCFLAGS = -d -v -t

ifndef LOCAL_HEADERS
LOCAL_HEADERS = $(wildcard *.h)
endif

# default cc rules
$(OBJDIR)/%.o : %.c $(DEPENDENCIES) $(LOCAL_HEADERS) 
	@test -d $(OBJDIR) || mkdir $(OBJDIR)	
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(NLS_CFLAGS) $(EXTRA_INC) $(INC) \
		-o $(OBJDIR)/$*.o -c $*.c

# default parser generation rules, include prefix for files/vars
%.yy.c: %.l
	$(LEX) -P$* $(LEXFLAGS) $*.l | \
	$(SED) -e 's/unistd.h/limits.h/g' \
	> $@

%.tab.h %.tab.c: %.y
	$(YACC) -b$* -p$* $(YACCFLAGS) $<


# default clean rules
clean:
	-find . -name 'OBJ*' -exec rm -rf {} \; 2>/dev/null
	-rm -rf $(EXTRA_CLEAN_DIRS)
	-rm -f $(EXTRA_CLEAN_FILES)

#below is a dirty hack (feel free to rewrite):
# html rules for cmd commands
htmlcmd:
	-GRASS_FAKE_START=1 GISBASE=$(GISBASE) LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$(GISBASE)/lib $(ETC)/bin/cmd/$(PGM) --html-description | grep -v '</body>' > $(PGM).html
	@test ! -f description.html || ( cat description.html >> $(PGM).html )
	echo "<HR>" >> $(PGM).html
	echo "<P><a href=index.html>Help Index</a>" >> $(PGM).html
	echo "</body></html>" >> $(PGM).html
	mkdir -p $(GISBASE)/docs/html
	mv $(PGM).html $(GISBASE)/docs/html
	-cp *.png *.jpg $(GISBASE)/docs/html 2> /dev/null

# html rules for scripts
htmlscript:
	-GRASS_FAKE_START=1 GISBASE=$(GISBASE) LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$(GISBASE)/lib $(GISBASE)/scripts/$(PGM) --html-description | grep -v '</body>' > $(PGM).html
	@test ! -f description.html || ( cat description.html >> $(PGM).html )
	echo "<HR>" >> $(PGM).html
	echo "<P><a href=index.html>Help Index</a>" >> $(PGM).html
	echo "</body></html>" >> $(PGM).html
	mkdir -p $(GISBASE)/docs/html
	mv $(PGM).html $(GISBASE)/docs/html
	-cp *.png *.jpg $(GISBASE)/docs/html 2> /dev/null

# html rules for inter commands
# note that fakestart doesn't work here
htmlinter:
	@test ! -f description.html || ( cat description.html >> $(PGM).html )
	echo "<HR>" >> $(PGM).html
	echo "<P><a href=index.html>Help Index</a>" >> $(PGM).html
	echo "</body></html>" >> $(PGM).html
	mkdir -p $(GISBASE)/docs/html
	mv $(PGM).html $(GISBASE)/docs/html
	-cp *.png *.jpg $(GISBASE)/docs/html 2> /dev/null

# html rules for ETC commands
htmletc:
	-GRASS_FAKE_START=1 GISBASE=$(GISBASE) LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$(GISBASE)/lib $(ETC)/$(PGM) --html-description | grep -v '</body>' > $(PGM).html
	@test ! -f description.html || ( cat description.html >> $(PGM).html )
	echo "<HR>" >> $(PGM).html
	echo "<P><a href=index.html>Help Index</a>" >> $(PGM).html
	echo "</body></html>" >> $(PGM).html
	mkdir -p $(GISBASE)/docs/html
	mv $(PGM).html $(GISBASE)/docs/html
	-cp *.png *.jpg $(GISBASE)/docs/html 2> /dev/null
