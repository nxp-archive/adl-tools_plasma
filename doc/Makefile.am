webdir = $(WEBHOME)

web_DATA = \
	user-guide.html \
	schedule.html

clean:
	rm -f *.html

%.html : %.rst ;
	$(TRIP) $^ > $@ 
