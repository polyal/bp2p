default:
	gcc -Wall bluetooth/blue.c -lbluetooth -o out/a.out

torrent:
	g++ -std=c++11 -Wall torrent/torrent.cpp torrent/torrent.h -o out/torrent

compression:
	gcc -Wall compression/compression.c -lz -o out/compression

archive:
	gcc -Wall archive/archive.c -lz -lbz2 -larchive -o out/a.out

clean:
	rm out/*
