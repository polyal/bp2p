objs = package.o compress.o hash.o           

default:
	gcc -Wall bluetooth/blue.c -lbluetooth -o out/a.out

torrent: packageObjs
	g++ -std=c++11 -Wall torrent/torrent.cpp torrent/torrent.h -Ilib/json/include -o out/a.out \
	out/arobj.o \
	-lz -larchive \
	-lcrypto

packageObjs: package hash mvObjs $(foreach obj, $(objs), out/$(obj))
	ar rvs out/arobj.o $(foreach obj, $(objs), out/$(obj))

package:
	gcc -Wall -c src/package.c src/compress.c -lz -larchive

hash:
	gcc -Wall -c src/hash.c -lcrypto

# moves obj files from root/ to out/
mvObjs:
	mv $(objs) -t out/



# compile each module as its own executable
# make sure to set DEBUG = 1
hashTest:
	gcc -Wall src/hash.c -lcrypto -o out/a.out

packageTest:
	gcc -Wall -o out/a.out src/package.c src/compress.c -lz -larchive;



# cleanup out dir
clean:
	rm out/*
