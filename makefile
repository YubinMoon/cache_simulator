csim : csim.o cache.o
	gcc -o csim csim.o cache.o

cache.o : cache.c cache.h
	gcc -c -o cache.o cache.c
  
csim.o : csim.c
	gcc -c -o csim.o csim.c
  
clean : 
	rm *.o csim