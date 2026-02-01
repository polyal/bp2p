# bp2p
An application that defines a node in a decentralized peer-to-peer network where nodes communicate via bluetooth.  When a user wants to share a file or several files, they can create a torrent out of these files.  The files will then be archived, compressed, and split into chunks.  All of this data about this torrent will be stored in a torrent file.  Nearby users will have the option to request which torrents other users are sharing, then they will have the option to download them.
<br><br>
There project is currently on development where improvements are always being added and bugs being fixed.  If you're interested in the inner-workings of the project, I suggest starting from **_main()_** in **_src/node.cpp_** and tracing through.  I am also providing a simplified version of a uml diagram to get a better idea of the structure and behaviour of classes.

### Usage
#### Commands
###### -lnt : List Nearby Torrents
###### -tc [torrent name] [file1 | file2 ...]  : Create torrent
###### -tr [torrent name] [addr] : request torrent file from a specific device
###### -td [torrent name] : download the file(s) descrived by the torrent

### Requirements

##### Packages
* libbluetooth-dev
* libarchive-dev
* libz-dev
* libssl-dev
* libmysqlcppconn-dev
* mysql-server -- must have MySQL server installed and initialzed

##### To clone submodule nlohmann/json
1. git submodule init
2. git submodule update

### Build
$> make<br>
$> ./out/a.out

