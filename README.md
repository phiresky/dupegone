small and fast duplicate file finder
---

Walks a directory, adding all files into a sorted map. Then for every same-size-set, Hashes the first 64kb of the files, and if that matches hash the whole file.

on a sample of 24000 files, 16GB, mostly pdf:
* `fdupes`: 14:03 min
* this: 9:21 min
* `rmlint`: 7:38 min

I was planning to make the fastest duplicate file finder, but then I found [rmlint](https://github.com/sahib/rmlint), which seems to do pretty much everything possible.

Interestingly enough, simply sha1sum-ing *every complete file* in the order `find` finds them is even faster than using any of these tools. Well fuck me for trying to use a nice algorithm when in reality the only thing taking any time is the harddrive skipping around between:

    find "$1" -type f -exec sha1sum {} + | sort | uniq -w 32 --all-repeated=separate
(by /u/Rangi42) takes 4:41 min.
![pic](http://i.imgur.com/SdLftak.png)

Made mostly as an exercise of C++ and algorithms/data structures

#### Dependencies
* openssl for sha1
* boost for filesystem access
