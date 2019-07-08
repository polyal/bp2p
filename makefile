default:
	gcc -Wall bluetooth/blue.c -lbluetooth -o out/a.out

torrent:
	g++ -std=c++11 -Wall torrent/torrent.cpp torrent/torrent.h -o out/torrent

package:
	gcc -Wall src/package.c src/compress.c -lz -larchive -o out/a.out

clean:
	rm out/*
