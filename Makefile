m: f.o g.o m.o
	cc -o m f.o g.o m.o
f.o : f.c
	cc -c f.c
g.o : g.c
	cc -c g.c
m.o : m.c
	cc -c m.c
