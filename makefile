default:
	gcc -Wall blue.c -lbluetooth -o out/a.out

torrent:
	g++ -std=c++11 -Wall torrent/torrent.cpp torrent/torrent.h -o out/torrent

compression:
	gcc -Wall compression/compression.c -lz -o out/compression

clean:
	rm out/*
