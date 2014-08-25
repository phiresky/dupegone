small and fast duplicate file finder
---

Walks a directory, adding all files into a sorted map. Then for every same-size-set, Hashes the first 64kb of the files, and if that matches hash the whole file.

on a sample of 24000 files, 16GB, mostly pdf:
* `fdupes`: 14:03 min
* this: 9:21 min
* `rmlint`: 7:38 min

I was planning to make the fastest duplicate file finder, but then I found [rmlint](https://github.com/sahib/rmlint), which seems to do pretty much everything possible.

Made mostly as an exercise of C++ and algorithms/data structures

#### Dependencies
* openssl for sha1
* boost for filesystem access
