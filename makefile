packageObjs = package.o compress.o 

default:
	gcc -Wall bluetooth/blue.c -lbluetooth -o out/a.out

torrent: packageObject cleanObjs
	g++ -std=c++11 -Wall torrent/torrent.cpp torrent/torrent.h -Ilib/json/include -o out/a.out \
	out/pkobj.o -lz -larchive

packageObject: package $(packageObjs)
	ar rvs out/pkobj.o $(packageObjs)

package:
	gcc -Wall -c src/package.c src/compress.c -lz -larchive

hash:
	gcc -Wall src/hash.c -lcrypto -o out/a.out

cleanObjs:
	rm $(packageObjs)

clean:
	rm out/*
