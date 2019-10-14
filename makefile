objs = package.o compress.o hash.o torrent.o
cobjs = package.o compress.o archiver.o hash.o
libs = -lz -larchive -lcrypto
inclJson = -Ilib/json/include

blueLibs = -lbluetooth


default: utils bluetooth torrent rrpacket
	g++ -std=c++1y -Wall src/node.cpp \
	-Ilib/json/include -o out/a.out \
	out/blue.o out/utils.o out/torrent.o \
	out/rrfactory.o out/rrpacket.o out/torrentFileReq.o out/torrentListReq.o out/chunkReq.o \
	out/package.o out/compress.o out/archiver.o out/hash.o \
	$(blueLibs) \
	$(libs)

rrpacket:
	g++ -std=c++1y -Wall -c -Ilib/json/include \
	src/rrfactory.cpp src/rrpacket.cpp src/torrentFileReq.cpp src/torrentListReq.cpp src/chunkReq.cpp;
	mv rrfactory.o  rrpacket.o torrentFileReq.o torrentListReq.o chunkReq.o -t out/

bluetooth:
	gcc -Wall -c src/blue.c;
	mv blue.o out/blue.o

torrent: package hash
	g++ -std=c++1y -Wall -Wextra -pedantic -c src/torrent.cpp -Ilib/json/include
	mv torrent.o out/torrent.o

packageObjs: mvObjs $(foreach obj, $(objs), out/$(obj))
	ar rvs out/arobj.a $(foreach obj, $(objs), out/$(obj))

package:
	g++ -std=c++1y -Wall -Wextra -pedantic -c src/package.cpp src/archiver.cpp src/compress.cpp
	mv package.o archiver.o compress.o -t out/

hash:
	g++ -std=c++1y -Wall -Wextra -pedantic -c src/hash.cpp
	mv hash.o out/hash.o

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
	g++ -std=c++1y -Wall -Wextra -pedantic src/hash.cpp src/utils.cpp -lcrypto -o out/a.out

packageTest:
	g++ -std=c++1y -Wall -Wextra -pedantic src/package.cpp src/archiver.cpp src/compress.cpp -lz -larchive -o out/a.out

compressTest:
	g++ -std=c++1y -Wall -Wextra -pedantic src/compress.cpp -lz -o out/a.out

archiverTest:
	g++ -std=c++1y -Wall -Wextra -pedantic src/archiver.cpp -larchive -o out/a.out

channelTest:
	g++ -std=c++1y -Wall -Wextra -pedantic src/btchannel.cpp -lbluetooth -o out/a.out

deviceTest:
	g++ -std=c++1y -Wall -Wextra -pedantic src/btdevice.cpp src/btchannel.cpp -lbluetooth -o out/a.out



# cleanup out dir
clean:
	rm out/*

# cleanup out dir
cleanall:
	rm out/* torrents/* torrentData/*

