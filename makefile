objs = package.o compress.o hash.o torrent.o
cobjs = package.o compress.o hash.o

default:
	gcc -Wall src/blue.c -lbluetooth -o out/a.out

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


# for testing torrent specific functinality
torrentTest: package hash mvCobjs
	g++ -std=c++1y -Wall src/torrent.cpp -Ilib/json/include -o out/a.out \
	$(foreach obj, $(cobjs), out/$(obj)) \
	-lz -larchive \
	-lcrypto

# compile each module as its own executable
# make sure to set DEBUG = 1
hashTest:
	gcc -Wall src/hash.c -lcrypto -o out/a.out

packageTest:
	gcc -Wall src/package.c src/compress.c -lz -larchive -o out/a.out



# cleanup out dir
clean:
	rm out/* torrents/*
