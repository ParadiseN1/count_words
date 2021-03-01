// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "lib_archive.h"

#include "../libArchive/archive.h"
#include "../libArchive/archive_entry.h"

void LibArchiveArchive::init(const std::string& data) {
    if(archive_read_open_filename(arc, data.c_str(), 10240) != ARCHIVE_OK  ){
        std::string lib_err_msg{archive_error_string(arc)};
        throw ("Error initializing error from memory: " + lib_err_msg);
    }
//    std::cout << archive_read_next_header(arc, &entry) << "\n";
}

uint64_t LibArchiveArchive::getFileSize() {
    if(archive_entry_size_is_set(entry) == 0){
        std::cout << "File size is not set." << '\n';
//        throw;
    }
    return archive_entry_size(entry);
}

std::string LibArchiveArchive::getFileName() {
    return archive_entry_pathname(entry);
}

bool LibArchiveArchive::nextFile() {
    return (archive_read_next_header(arc, &entry) == ARCHIVE_OK);
}

void LibArchiveArchive::readNextFile(void *file_content_pt, size_t size) {
//    archive_read_data(arc, &file_content[0], file_content.size());
    archive_read_data(arc, file_content_pt, size);
}


LibArchiveArchive::LibArchiveArchive(){
    arc = archive_read_new();
    archive_read_support_filter_all(arc);
    archive_read_support_format_all(arc);
}

LibArchiveArchive::~LibArchiveArchive() {
    archive_read_free(arc);

}

