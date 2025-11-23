#include "main.h"
#include <strings/strings.h>
#include <iostream>
#include <array>

int main() {

	HEAP_INIT(16);

	FilePathContainer* s = (FilePathContainer*)GetOnHeap<FilePathContainer>();
	HEAP_HOOK(s);

	char* fname = (char*)GetOnHeap<char[32]>();
	HEAP_HOOK(fname);

	// structPath
	*&s->pathTypeInfo.separator = '/';
	strcpy_s((char*)&s->str1, 16, "C:/");
	strcpy_s((char*)&s->str2, 16, "some/path/");

	// fpath
	char* fpath = (char*)GetOnHeap<char[32]>();
	HEAP_HOOK(fpath);

	std::array<char[8], 10> pathValues = { "C:/a/b/", "C:/a/b", "C:/a/", "C:/a", "a", "a/", "a/b", "a/b/", "/a/b/", "/a/b"};
	for (char* _structPath : pathValues) {
		strcpy_s((char*)&s->path, 16, _structPath);
		for (char* _fpath : pathValues) {
			strcpy_s(fpath, 32, _fpath);
			JoinPath(s, fname, fpath, 32);
			std::cout << fname << ",";
		}
		std::cout << std::endl;
	}

	HEAP_FREE();

	return 0;

}

