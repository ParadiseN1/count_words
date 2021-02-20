#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <filesystem>
#include <boost/locale.hpp>
#include <boost/version.hpp>
#include <map>
#include <unordered_map>
#include <thread>
#include <numeric>
#include <algorithm>

#include "src/lib_archive.h"
#include "src/word_count.h"
#include "src/queue_t.h"
#include "src/time.h"

struct indexed_file{
    std::string str;
    size_t       idx;
};

namespace fs = std::filesystem;
const int MAX_ARCHIVE_SIZE = 10000000;
std::condition_variable cv;
std::mutex mu;
queue_t<indexed_file> file_content_queue;
size_t n_calc_threads = 5;
size_t n_merge_threads = 5;
bool readEnd = false;
bool calcEnd = false;

void calc_thread(std::vector<std::unordered_map<std::string, size_t>> *files_words, int thread_idx, queue_t<std::unordered_map<std::string, size_t>> *merge_queue)
{
    std::cout << "Thread {" << thread_idx << "} is alive." << std::endl;
    while(!readEnd || !file_content_queue.empty()) {
        std::cout << "Thread {" << thread_idx << "} start waiting." << "readEnd:" << readEnd << ", queue.empty():" << file_content_queue.empty() << std::endl;

        indexed_file queueObj;
        std::unordered_map<std::string, size_t> word_map;
        file_content_queue.pop(queueObj);

        std::cout << "{" << thread_idx << "}[" << queueObj.idx << "]Calc thread Begin...  " << "readEnd:" << readEnd << ", queue.empty():" << file_content_queue.empty() <<  std::endl;
        (*files_words)[queueObj.idx] = stringToMap(queueObj.str, &word_map);
        merge_queue->push(std::move((*files_words)[queueObj.idx]));
        std::cout << "{" << thread_idx << "}[" << queueObj.idx << "]Calc thread Finish..." << "readEnd:" << readEnd << ", queue.empty():" << file_content_queue.empty() << std::endl;
    }
    std::cout << "Thread {" << thread_idx << "} is dead." << std::endl;
}

void read_thread(std::vector<std::string> &&archive_filenames, std::vector<std::unordered_map<std::string, size_t>> *vec){
    size_t q_idx = 0;
    for (const auto& fn: archive_filenames) {
        LibArchiveArchive archive = LibArchiveArchive();
        std::cout << "[" << q_idx << "]Read thread..." << fn << std::endl;
        archive.init(fn);
//        auto file_name = archive.getFileName();
        while (archive.nextFile()) {
            size_t entry_size = archive.getFileSize();
            if (entry_size > MAX_ARCHIVE_SIZE) {
                std::cout << "archive entry is too large, skip it" << std::endl;
                continue;
            }
            if (archive.getFileName().compare(archive.getFileName().size()-3, 3, "txt") != 0)
                continue;
            indexed_file file;
            file.idx = q_idx;
            std::cout<< "/" << q_idx << "\\" << archive.getFileName() << std::endl;
            file.str = std::string(entry_size, char{});
            archive.readNextFile(file.str);
            file_content_queue.push(std::move(file));
//            cv.notify_one();
            if (q_idx >= vec->size())
                vec->resize(vec->size() * 2);
            ++q_idx;
        }
    }
    readEnd = true;
    std::cout << "FILES COUNT:" << q_idx << std::endl;
}

void merge_pair(std::unordered_map<std::string, size_t> &&first, std::unordered_map<std::string, size_t> &&second,
                queue_t<std::unordered_map<std::string, size_t>> *merge_queue){

    first = std::accumulate( second.begin(), second.end(), first,
                             []( auto &m, auto &p )
                             {
                                 return ( m[p.first] +=p.second, m );
                             });
    merge_queue->push(std::move(first));
}

void merge_thread(queue_t<std::unordered_map<std::string, size_t>> *merge_queue, int i, int *working_threads, std::mutex *mu){
    while(true){
        std::cout << "{" << i << "}" << " Starts!\n";
        std::cout << "Merge queue size:" << merge_queue->size() << std::endl;
        mu->lock();
        std::cout << "Working threads:" << *working_threads << std::endl;
        if (*working_threads == 1 && merge_queue->size() == 1) {
            mu->unlock();
            break;
        }
        mu->unlock();
        if (merge_queue->empty())
            break;
        std::unordered_map<std::string, size_t> first_to_merge;
        std::cout << "{" << i << "}" << " pop1;\n";
        merge_queue->pop(first_to_merge);
        if (merge_queue->empty()){
            merge_queue->push(std::move(first_to_merge));
            break;
        }
        std::unordered_map<std::string, size_t> second_to_merge;
        std::cout << "{" << i << "}" << " pop2;\n";
        merge_queue->pop(second_to_merge);
        std::cout << "{" << i << "}" << " merge;\n";
        merge_pair(std::move(first_to_merge), std::move(second_to_merge), merge_queue);

    }
    mu->lock();
    --(*working_threads);
    mu->unlock();
    std::cout << "Thread {" << i << "} is dead." << std::endl;
}

std::vector<std::string> file_names(std::string path){
    std::vector<std::string> archive_filenames;
    for (const auto& entry: fs::recursive_directory_iterator(path)) {
        if ((entry.path().extension() == ".ZIP") || (entry.path().extension() == ".zip")) {
            archive_filenames.push_back(entry.path().string());
        }
    }

    return archive_filenames;
}

void sort(std::unordered_map<std::string, size_t>& M)
{
    std::vector<std::pair<std::string, int> > A;

    for (auto& it : M) {
        A.emplace_back(it);
    }

    using pairtype=std::pair<std::string,size_t>;
    sort(A.begin(), A.end(),
         [](pairtype a, pairtype b){
                    return a.second < b.second;
            }
        );
}

template<typename KeyType, typename ValueType>
std::pair<KeyType,ValueType> get_max( const std::unordered_map<KeyType,ValueType>& x ) {
    using pairtype=std::pair<KeyType,ValueType>;
    return *std::max_element(x.begin(), x.end(),
                             [] (const pairtype & p1, const pairtype & p2) {
                                        return p1.second < p2.second;
                                    }
    );
}

int main() {
    std::string test_data_folder = "../test";
    std::string full_data_folder = "../data";
    auto before = get_current_time_fenced();

    //// 1. Get archive filenames, and their number. (not parallel)
    std::vector<std::string> archive_filenames;

    archive_filenames = file_names(test_data_folder);
//    archive_filenames = std::vector(archive_filenames.begin(), archive_filenames.begin() + 15);
    std::cout << "number of archives: " << archive_filenames.size() << std::endl;
    std::cout << "First archive: " << archive_filenames[0] << std::endl;
    std::cout << "Last archive: " << archive_filenames[archive_filenames.size()-1] << std::endl;

    //// 2. Create an vector of words dictionaries, which size if the number of files. (not parallel)
    std::vector<std::unordered_map<std::string, size_t>> files_words(archive_filenames.size());
    //// 3. Calculate words dictionaries for every file. (parallel)
    queue_t<std::unordered_map<std::string, size_t>> merge_queue;
    std::vector<std::thread> calc_threads;
    // std::thread calc_threads[n_calc_threads]; GCC only! Nonstandard!
    for(int i = 0; i < n_calc_threads; ++i) {
        calc_threads.emplace_back(calc_thread, &files_words, i, &merge_queue); // Fixed rather unstylish wording.
    }

    std::thread readThread{read_thread, std::move(archive_filenames), &files_words};
    std::unordered_map<std::string, size_t> word_map;
    ////queue to MAP:
    readThread.join();
    for(int i = 0; i < n_calc_threads; ++i) {
        calc_threads[i].join();
    }
    std::cout << "ALL THREADS FINISHED!!!\n";
    std::vector<std::thread> merge_threads; // [n_merge_threads]; !non-standard, gcc only

    std::cout << "Merge queue size:" << merge_queue.size() << std::endl;

    int threads_left = n_merge_threads;
    std::mutex mu;
    for (int i =0; i < n_merge_threads; ++i) {
        merge_threads.emplace_back(merge_thread, &merge_queue, i, &threads_left, &mu);
    }

    for (int i =0; i < n_merge_threads; ++i) {
        merge_threads[i].join();
    }

    std::unordered_map<std::string, size_t> final_dict;

    merge_queue.pop(final_dict);

    auto max = get_max(final_dict);

    std::cout << max.first << ":" << max.second << std::endl;
//    sort(final_dict);
//
//    for (auto const& [key, val] : final_dict) {
//        std::cout << key        // string (key)
//                  << ':'
//                  << val        // string's value
//                  << std::endl;
//    }
    auto time_to_calculate = get_current_time_fenced() - before;
    std::cout << "Time: " << to_s(time_to_calculate) << "s" << std::endl;
    std::cout << files_words.size() << std::endl;
//    std::cout << "queue is empty!" << std::endl;
//    for (auto & files_word : files_words){
//        sortMap(files_word, 1);
//    }
//    printMap(word_map);
//
//// TEST SECTION
//    std::cout << "--------------------------------" << std::endl;
//    std::string str = "Hello hello, Abbs abbs, abu abu, aba aba, ccc sad sad sad:aaa-aaa\'aaa\' zx cds the loop";
//    std::unordered_map<std::string, size_t> test_word_map;
//    stringToMap(str, test_word_map);
//    //printMap(test_word_map);
//    sortMap(test_word_map);

    return 0;
}

