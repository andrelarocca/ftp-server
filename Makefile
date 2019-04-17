all:
	gcc -pthread servidorFTP.c -o servidor
	gcc -pthread clienteFTP.c -o cliente

run:
	gcc -pthread servidorFTP.c -o servidor
	gcc -pthread clienteFTP.c -o cliente
	./servidor