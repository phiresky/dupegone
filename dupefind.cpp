#include <cstdlib>
#include <cstdint>
#include <cmath>
#include <iostream>
#include <fstream>
#include <map>
#include <unordered_map>
#include <array>

#include <openssl/sha.h>
#include <boost/filesystem.hpp>
#include <boost/functional/hash/hash.hpp>

#ifdef DEBUG
#define DBG(x) x
#else
#define DBG(x)
#endif
namespace fs = boost::filesystem;
using namespace std;

const int hashlen = SHA_DIGEST_LENGTH;
auto hashfunc = SHA1;
using digest = array<unsigned char, hashlen>;
namespace std { template <> struct hash<digest> {
	// hash the hash
	size_t operator()(const digest& x) const {
		size_t seed = 0;
		for(char c:x) seed^=c;
		return seed;
	}
};}

uint64_t minsize = 1; // ignore files smaller than x bytes 
uint64_t buffersize = 1<<12; // size to do the initial checksumming

class file_info {
	
	digest _firsthash,_fullhash;
	bool got_size=false,got_firsthash=false,got_fullhash=false;
	public:
		string path;
		uint64_t size;
		file_info(): path(""), size(0) {}
		file_info(string path, uint64_t size) : path(path), size(size) {}
		/** hash of the full file */
		digest fullhash() {
			if(buffersize>=size) return firsthash();
			if(!got_fullhash++) {
	 			// best code ever (plz dont put " in your file names) (TODO:replace)
				string cmd = "sha1sum \""+path+"\"|cut -c 1-40|xxd -r -p";
				//cout << cmd << endl;
				FILE* pipe = popen(cmd.c_str(), "r");
				fread(&_fullhash.front(), 1, hashlen, pipe);
				pclose(pipe);
			}
			DBG(cout<<"fuHash of "<<path<<":"<< hash<digest>()(_fullhash) <<endl;)
			return _fullhash;
		}
		/** hash of the first segment of the file */
		digest firsthash() {
			if(!got_firsthash++) {
				uint64_t length = min(buffersize,size);
				ifstream file(path);
				char buf[length];
				file.read(buf,length);
				file.close();
				hashfunc((unsigned char*)buf,length,&_firsthash.front());
			}

			DBG(cout<<"fiHash of "<<path<<":"<< hash<digest>()(_firsthash) <<endl;)
			return _firsthash;
		}
};

// print progress bar
void progress_every(string prefix, int interval, int x, int of) {
	if(x%interval==0) cerr << prefix << x << "/" << of << "\r";	
}

int main(int argc, char *argv[]) {
	//fs::directory_entry a(fs::path("../unity/b3d/Assets/Necromancer GUI/Materials/lambert1.mat"));
	//fs::directory_entry b(fs::path("../unity/b3d/Assets/spider/Materials/lambert1.mat"));

	if(argc < 2) {
		cout << "Usage: dupefind <directory> <minfilesize>" << endl;
		return 0;
	}
	string dirname = argv[1];
	minsize = max(stoi(argv[2]),1);
	multimap<int,fs::directory_entry> map; 
	for(fs::recursive_directory_iterator end, file(dirname); file!=end; ++file) {
			if(file->status().type()!=fs::regular_file) continue;
			uint64_t size = fs::file_size(*file);
			if(size < minsize) continue;
			map.insert(make_pair(size,*file));
			progress_every("Scanning... ",10000,map.size(),0);
	}
	cerr<<"Found "<<map.size()<<" files, comparing"<<endl;

	file_info last;
	unordered_map<digest,file_info> samesizeset;
	int i = 0, dupes = 0;
	for(auto& entry:map) {
		file_info cur(entry.second.path().string(),entry.first);
		DBG(cout<<"Current size:"<<cur.size<<endl);
		progress_every("Comparing... ",100,i++,map.size());
		if(cur.size < minsize) continue;
		if(cur.size == last.size) {
			if(samesizeset.size() == 0) {
				samesizeset.insert(make_pair(last.firsthash(),last));
			}
			auto duplicate = samesizeset.insert(make_pair(cur.firsthash(),cur));
			if(!duplicate.second) {
				auto other = duplicate.first->second;
				// already exists in set, possibly same file
				if(other.fullhash()==cur.fullhash()) {
					cout << endl << other.path << endl << cur.path << endl;
					dupes++;
				}
			}
		} else samesizeset.clear();
		last = cur;
	}
	cerr << "Done, found " << dupes << " duplicates." << endl;
}
