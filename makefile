packageObjs = package.o compress.o 

default:
	gcc -Wall bluetooth/blue.c -lbluetooth -o out/a.out

torrent: packageObject
	g++ -std=c++11 -Wall torrent/torrent.cpp torrent/torrent.h -Ilib/json/include -o out/a.out \
	out/pkobj.o -lz -larchive

packageObject: package mvObjs $(foreach obj, $(packageObjs), out/$(obj))
	ar rvs out/pkobj.o $(foreach obj, $(packageObjs), out/$(obj))

package:
	gcc -Wall -c src/package.c src/compress.c -lz -larchive

hash:
	gcc -Wall src/hash.c -lcrypto -o out/a.out


#cleanup objfiles generated in root
mvObjs:
	mv $(packageObjs) -t out/

#cleanup all obj files and out dir
clean:
	rm out/*
