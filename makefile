objs = package.o compress.o hash.o torrent.o
cobjs = package.o compress.o hash.o
libs = -lz -larchive -lcrypto

blueLibs = -lbluetooth

default:
	gcc -Wall src/blue.c -lbluetooth -o out/a.out

comm: bluetooth
	g++ -std=c++1y -Wall src/comm.cpp -o out/a.out \
	out/blue.o \
	$(blueLibs)

bluetooth:
	gcc -Wall -c src/blue.c;
	mv blue.o out/blue.o

torrent: package hash
	g++ -std=c++1y -Wall -c src/torrent.cpp -Ilib/json/include

packageObjs: mvObjs $(foreach obj, $(objs), out/$(obj))
	ar rvs out/arobj.a $(foreach obj, $(objs), out/$(obj))

package:
	gcc -Wall -c src/package.c src/compress.c

hash:
	gcc -Wall -c src/hash.c

# moves obj files from root/ to out/
mvObjs:
	mv $(objs) -t out/

mvCobjs:
	mv $(cobjs) -t out/



# compile each module as its own executable
# make sure to set DEBUG = 1
torrentTest: package hash mvCobjs
	g++ -std=c++1y -Wall src/torrent.cpp -Ilib/json/include -o out/a.out \
	$(foreach obj, $(cobjs), out/$(obj)) \
	$(libs)

hashTest:
	gcc -Wall src/hash.c -lcrypto -o out/a.out

packageTest:
	gcc -Wall src/package.c src/compress.c -lz -larchive -o out/a.out



# cleanup out dir
clean:
	rm out/* torrents/*
