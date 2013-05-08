
all:
	$(CC) ss4200bright.c -o ss4200bright

static:
	$(CC) --static ss4200bright.c -o ss4200bright
	strip ss4200bright

clean:
	rm -f ss4200bright
