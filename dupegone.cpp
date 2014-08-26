#include <cstdint>
#include <cmath>
#include <iostream>
#include <fstream>
#include <map>
#include <unordered_map>
#include <array>

#include <openssl/sha.h>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;
using namespace std;

const int hashlen = SHA_DIGEST_LENGTH;
using digest = array<unsigned char, hashlen>;
namespace std { template <> struct hash<digest> {
	size_t operator()(const digest& x) const { // hash sha to int
		size_t seed = 0;
		for(char c:x) seed^=c;
		return seed;
	}
};}

digest sha1(string path, uint64_t maxlen) {
	char buf[8192];
	SHA_CTX sc;
	ifstream f(path);
	SHA1_Init(&sc);
	uint64_t read = 0;
	while(read < maxlen) {
		f.read(buf, sizeof buf);
		size_t len = f.gcount(); read+=len;
		if (len == 0) break;
		SHA1_Update(&sc, buf, len);
	}
	f.close();
	digest dig;
	SHA1_Final(&dig.front(), &sc);
	return dig;
}

uint64_t minsize = 1; // ignore files smaller than x bytes 
uint64_t firsthash_size = 1<<16; // size to do the initial checksumming

class file_info {
	digest _firsthash, _fullhash;
	bool got_size=false, got_firsthash=false, got_fullhash=false;
	public:
		string path;
		uint64_t size;
		file_info(): path(""), size(0) {}
		file_info(string path, uint64_t size) : path(path), size(size) {}
		digest fullhash() {
			if(firsthash_size>=size) return firsthash();
			if(!got_fullhash++) _fullhash = sha1(path, size);
			return _fullhash;
		}
		/** hash of the first segment of the file */
		digest firsthash() {
			if(!got_firsthash++) _firsthash = sha1(path, min(firsthash_size,size));
			return _firsthash;
		}
};

int main(int argc, char *argv[]) {
	if(argc < 3) {
		cout << "Usage: dupefind <directory> <minfilesize>" << endl;
		return 0;
	}
	string dir = argv[1];
	minsize = max(stoi(argv[2]),1);
	multimap<int,file_info> map; 
	for(fs::recursive_directory_iterator end, file(dir); file!=end; ++file) {
		if(file->status().type()!=fs::regular_file) continue;
		uint64_t size = fs::file_size(*file);
		if(size < minsize) continue;
		map.insert(make_pair(size,file_info(file->path().string(),size)));
	}
	cerr<<"Found "<<map.size()<<" files, comparing"<<endl;

	for(auto it = map.begin(); it != map.end();) {
		if(map.count(it->first) < 2) map.erase(it++);
		else ++it;
	}
	cerr<<map.size()<<" files after size-filtering"<<endl;

	unordered_multimap<digest,file_info> samesizeset;
	uint64_t i = 0, dupes = 0, lastsize=0;
	for(auto& entry:map) {
		if(i++%100==0) cerr << "\rComparing... " << i << "/" << map.size();
		if(entry.second.size < minsize) continue;
		if(entry.second.size == lastsize) {
			auto equals = samesizeset.equal_range(entry.second.firsthash());
			for(auto it=equals.first; it!=equals.second; ++it) {
				auto other = it->second;
				if(other.fullhash() == entry.second.fullhash()) {
					cout << endl << other.path << endl
						<< entry.second.path << endl;
					dupes++;
				}
			}
		} else {
			samesizeset.clear();
			lastsize = entry.second.size;
		}
		samesizeset.insert(make_pair(entry.second.firsthash(),entry.second));
	}
	cerr << endl << "Done, found " << dupes << " duplicates." << endl;
}
