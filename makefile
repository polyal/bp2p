objs = package.o compress.o hash.o torrent.o
cobjs = package.o compress.o hash.o
libs = -lz -larchive -lcrypto
inclJson = -Ilib/json/include

blueLibs = -lbluetooth


default: utils bluetooth torrent rrpacket
	g++ -std=c++1y -Wall src/node.cpp \
	-Ilib/json/include -o out/a.out \
	out/blue.o out/utils.o out/torrent.o \
	out/rrfactory.o out/rrpacket.o out/torrentFileReq.o out/torrentListReq.o out/chunkReq.o \
	$(foreach obj, $(cobjs), out/$(obj)) \
	$(blueLibs) \
	$(libs)

rrpacket:
	g++ -std=c++1y -Wall -c -Ilib/json/include \
	src/rrfactory.cpp src/rrpacket.cpp src/torrentFileReq.cpp src/torrentListReq.cpp src/chunkReq.cpp;
	mv rrfactory.o out/rrfactory.o;
	mv rrpacket.o out/rrpacket.o;
	mv torrentFileReq.o out/torrentFileReq.o;
	mv torrentListReq.o out/torrentListReq.o;
	mv chunkReq.o out/chunkReq.o;

bluetooth:
	gcc -Wall -c src/blue.c;
	mv blue.o out/blue.o

torrent: package hash mvCobjs
	g++ -std=c++1y -Wall -Wextra -pedantic -c src/torrent.cpp -Ilib/json/include
	mv torrent.o out/torrent.o

packageObjs: mvObjs $(foreach obj, $(objs), out/$(obj))
	ar rvs out/arobj.a $(foreach obj, $(objs), out/$(obj))

package:
	gcc -Wall -c src/package.c src/compress.c

hash:
	gcc -Wall -c src/hash.c

utils:
	g++ -std=c++1y -Wall -c src/utils.cpp;
	mv utils.o out/utils.o

# moves obj files from root/ to out/
mvObjs:
	mv $(objs) -t out/

mvCobjs:
	mv $(cobjs) -t out/



# compile each module as its own executable
# make sure to set DEBUG = 1

bluetoohTest:
	gcc -Wall src/blue.c -lbluetooth -o out/a.out

torrentTest: package hash mvCobjs
	g++ -std=c++1y -Wall src/torrent.cpp -Ilib/json/include -o out/a.out \
	out/utils.o \
	$(foreach obj, $(cobjs), out/$(obj)) \
	$(libs)

hashTest:
	gcc -Wall src/hash.c -lcrypto -o out/a.out

packageTest:
	gcc -Wall src/package.c src/compress.c -lz -larchive -o out/a.out



# cleanup out dir
clean:
	rm out/*

# cleanup out dir
cleanall:
	rm out/* torrents/* torrentData/*

