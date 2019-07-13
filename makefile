default:
	gcc -Wall bluetooth/blue.c -lbluetooth -o out/a.out

torrent: packageObject cleanobj
	g++ -std=c++11 -Wall torrent/torrent.cpp torrent/torrent.h -Ilib/json/include -o out/a.out \
	out/pkobj.o -lz -larchive

packageObject: package package.o compress.o
	ar rvs out/pkobj.o package.o compress.o

package:
	gcc -Wall -c src/package.c src/compress.c -lz -larchive

hash:
	gcc -Wall src/hash.c -lcrypto -o out/a.out

cleanobj:
	rm compress.o package.o

clean:
	rm out/*
