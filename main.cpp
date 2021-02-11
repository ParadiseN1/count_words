#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <experimental/filesystem>
#include <boost/locale.hpp>
#include <boost/version.hpp>
#include <map>
#include <unordered_map>
#include <thread>

#include "src/lib_archive.h"
#include "src/word_count.h"
#include "src/queue_t.h"
#include "src/indexed_file.h"
#include "src/time.h"

void print(std::string strz){
    std::cout << strz << std::endl;
}

namespace fs = std::filesystem;
const int MAX_ARCHIVE_SIZE = 10000000;
std::condition_variable cv;
std::mutex mu;
queue_t<indexed_file> file_content_queue;
u_int n_calc_threads = 10;
u_int n_merge_threads = 4;
bool readEnd = false;

void calc_thread(std::vector<std::unordered_map<std::string, u_int>> *files_words, int thread_idx)
{
    std::cout << "Thread {" << thread_idx << "} is alive." << std::endl;
    while(!readEnd || !file_content_queue.empty()) {
        std::unique_lock<std::mutex> lk(mu);
        std::cout << "Thread {" << thread_idx << "} start waiting." << "readEnd:" << readEnd << ", queue.empty():" << file_content_queue.empty() << std::endl;
        cv.wait(lk, [] { return !file_content_queue.empty(); });

        indexed_file queueObj;
        std::unordered_map<std::string, u_int> word_map;
        file_content_queue.pop(queueObj);

        std::cout << "{" << thread_idx << "}[" << queueObj.idx << "]Calc thread Begin...  " << "readEnd:" << readEnd << ", queue.empty():" << file_content_queue.empty() <<  std::endl;
//      std::cout << "idx:" << queueObj.idx << "/" << q_idx << "|" << files_words.size() << std::endl;
        (*files_words)[queueObj.idx] = stringToMap(queueObj.str, &word_map);
        std::cout << "{" << thread_idx << "}[" << queueObj.idx << "]Calc thread Finish..." << "readEnd:" << readEnd << ", queue.empty():" << file_content_queue.empty() << std::endl;
        if (!file_content_queue.empty())
            cv.notify_all();
        else
            break;
    }
    std::cout << "Thread {" << thread_idx << "} is dead." << std::endl;
}

void read_thread(std::vector<std::string> &&archive_filenames){
    u_int q_idx = 0;
    for (const auto& fn: archive_filenames) {
        LibArchiveArchive archive = LibArchiveArchive();
        std::cout << "[" << q_idx << "]Read thread..." << fn << std::endl;
        archive.init(fn);
        while (archive.nextFile()) {
            size_t entry_size = archive.getFileSize();
            if (entry_size > MAX_ARCHIVE_SIZE) {
                std::cout << "archive entry is too large, skip it" << std::endl;
                continue;
            }
            indexed_file file;
            file.idx = q_idx;
            file.str = std::string(entry_size, char{});
            archive.readNextFile(file.str);
            file_content_queue.push(std::move(file));
            cv.notify_all();
        }
        ++q_idx;
        if (q_idx == archive_filenames.size()){
            readEnd = true;
        }
    }
}

void merge(std::vector<std::unordered_map<std::string, u_int>> *files_words){

}

std::vector<std::string> file_names(std::string path){
    std::vector<std::string> archive_filenames;
    for (const auto& entry: fs::recursive_directory_iterator(path)) {
        if ((entry.path().extension() == ".ZIP") || (entry.path().extension() == ".zip")) {
            archive_filenames.push_back(entry.path());
        }
    }

    return archive_filenames;
}

int main() {
    std::string test_data_folder = "../test";
    std::string full_data_folder = "../data";
    auto before = get_current_time_fenced();

    //// 1. Get archive filenames, and their number. (not parallel)
    std::vector<std::string> archive_filenames;

    archive_filenames = file_names(test_data_folder);
    std::cout << "number of archives: " << archive_filenames.size() << std::endl;
    std::cout << "First archive: " << archive_filenames[0] << std::endl;
    std::cout << "Last archive: " << archive_filenames[archive_filenames.size()-1] << std::endl;

    //// 2. Create an vector of words dictionaries, which size if the number of files. (not parallel)
    std::vector<std::unordered_map<std::string, u_int>> files_words(archive_filenames.size());
    //// 3. Calculate words dictionaries for every file. (parallel)
    // Read archive content
    std::thread calc_threads[n_calc_threads];
    for(int i = 0; i < n_calc_threads; ++i) {
        calc_threads[i] = std::thread(calc_thread, &files_words, i);
    }

    std::thread readThread{read_thread, std::move(archive_filenames)};
    std::unordered_map<std::string, u_int> word_map;
    ////queue to MAP:
    readThread.join();
    for(int i = 0; i < n_calc_threads; ++i) {
        calc_threads[i].join();
    }

//    std::thread merge_threads[n_merge_threads];
//    for (int i =0; i < n_merge_threads; ++i) {
//
//    }

    auto time_to_calculate = get_current_time_fenced() - before;
    std::cout << "Time: " << to_s(time_to_calculate) << "s" << std::endl;

//    std::cout << "queue is empty!" << std::endl;
//    for (auto & files_word : files_words){
//        sortMap(files_word, 1);
//    }
//    printMap(word_map);
//
//// TEST SECTION
//    std::cout << "--------------------------------" << std::endl;
//    std::string str = "Hello hello, Abbs abbs, abu abu, aba aba, ccc sad sad sad:aaa-aaa\'aaa\' zx cds the loop";
//    std::unordered_map<std::string, u_int> test_word_map;
//    stringToMap(str, test_word_map);
//    //printMap(test_word_map);
//    sortMap(test_word_map);

    return 0;
}

