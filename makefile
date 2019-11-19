cc = g++
cv = -std=c++1z
wrn = -Wall -Wextra -pedantic
compile = $(cc) $(cv) $(wrn)
libs = -lz -larchive -lcrypto -lbluetooth -pthread
incl = -Ilib/json/include


default: utils device torrent rrpacket
	$(compile) src/node.cpp src/workerThread.cpp \
	$(incl) \
	-o out/a.out \
	out/btdevice.o out/btchannel.o \
	out/rrfactory.o out/rrpacket.o out/torrentFileReq.o out/torrentListReq.o out/chunkReq.o \
	out/torrent.o out/package.o out/compress.o out/archiver.o out/hash.o \
	out/utils.o \
	$(libs)

rrpacket:
	$(compile) -c -Ilib/json/include \
	src/rrfactory.cpp src/rrpacket.cpp src/torrentFileReq.cpp src/torrentListReq.cpp src/chunkReq.cpp;
	mv rrfactory.o  rrpacket.o torrentFileReq.o torrentListReq.o chunkReq.o -t out/

device: channel
	$(compile) -c src/btdevice.cpp
	mv btdevice.o out/btdevice.o

channel:
	$(compile) -c src/btchannel.cpp
	mv btchannel.o out/btchannel.o

torrent: package hash
	$(compile) -c src/torrent.cpp $(incl)
	mv torrent.o out/torrent.o

package:
	$(compile) -c src/package.cpp src/archiver.cpp src/compress.cpp
	mv package.o archiver.o compress.o -t out/

hash:
	$(compile) -c src/hash.cpp
	mv hash.o out/hash.o

utils:
	$(compile) -c src/utils.cpp;
	mv utils.o out/utils.o



objs = package.o compress.o hash.o torrent.o
cobjs = package.o compress.o archiver.o hash.o

# moves obj files from root/ to out/
mvObjs:
	mv $(objs) -t out/

mvCobjs:
	mv $(cobjs) -t out/

packageObjs: mvObjs $(foreach obj, $(objs), out/$(obj))
	ar rvs out/arobj.a $(foreach obj, $(objs), out/$(obj))


# compile each module as its own executable
# make sure to set DEBUG = 1
blueTest:
	gcc $(wrn) src/blue.c -lbluetooth -o out/a.out

torrentTest: package hash mvCobjs
	$(compile) src/torrent.cpp $(incl) -o out/a.out \
	out/utils.o \
	$(foreach obj, $(cobjs), out/$(obj)) \
	$(libs)

hashTest:
	$(compile) src/hash.cpp src/utils.cpp -lcrypto -o out/a.out

packageTest:
	$(compile) src/package.cpp src/archiver.cpp src/compress.cpp -lz -larchive -o out/a.out

compressTest:
	$(compile) src/compress.cpp -lz -o out/a.out

archiverTest:
	$(compile) src/archiver.cpp -larchive -o out/a.out

channelTest:
	$(compile) src/btchannel.cpp -lbluetooth -o out/a.out

deviceTest:
	$(compile) src/btdevice.cpp src/btchannel.cpp -lbluetooth -o out/a.out



# cleanup out dir
clean:
	rm out/*

# cleanup out dir
cleanall:
	rm out/* torrents/* torrentData/*

