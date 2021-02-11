//
// Created by Admin on 27.01.2021.
//

#ifndef UNTITLED4_LIB_ARCHIVE_H
#define UNTITLED4_LIB_ARCHIVE_H
#include <iostream>

// http://www.libarchive.org
// https://github.com/libarchive/libarchive/wiki/Examples
// https://github.com/libarchive/libarchive/wiki/ManPageArchiveReadHeader3
class LibArchiveArchive{
private:
    struct archive *arc = nullptr;
    struct archive_entry *entry = nullptr;
    int res = 0;

public:
    void init(const std::string& data);
    uint64_t getFileSize();
    std::string getFileName();
    bool nextFile();
    void readNextFile(std::string &file_content);

    LibArchiveArchive();
    ~LibArchiveArchive();

};
#endif //UNTITLED4_LIB_ARCHIVE_H
