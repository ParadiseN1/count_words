// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <iostream>
#include <fstream>
#include <sstream>
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
size_t n_calc_threads = 20;
size_t n_merge_threads = 20;
bool readEnd = false;
bool calcEnd = false;

void calc_thread(int thread_idx, queue_t<std::unordered_map<std::string, size_t>> *merge_queue)
{
    std::cout << "Thread {" << thread_idx << "} is alive." << std::endl;
    while(!readEnd || !file_content_queue.empty()) {
        indexed_file queueObj;
        file_content_queue.pop(queueObj);

        std::cout << "stringToMap()\n";
        merge_queue->push(text_to_vocabulary(std::move(queueObj.str)));
    } // Terminating with uncaught exception of type std::bad_cast: std::bad_cast
    std::cout << "Thread {" << thread_idx << "} is dead." << std::endl;
}

void read_thread(std::vector<std::string> &&archive_filenames){
    size_t q_idx = 0;
    for (const auto& fn: archive_filenames) {
        LibArchiveArchive archive{};
//        std::cout << "[" << q_idx << "]Read thread..." << fn << std::endl;
        archive.init(fn);
        while (archive.nextFile()) {
            size_t entry_size = archive.getFileSize();
            if (entry_size > MAX_ARCHIVE_SIZE) {
                std::cout << "archive entry is too large, skip it" << std::endl;
                continue;
            }
            if (archive.getFileName().compare(archive.getFileName().size() - 3, 3, "txt") != 0)
                continue;
            indexed_file file;
            file.idx = q_idx;
//            std::cout<< "/" << q_idx << "\\" << archive.getFileName() << std::endl;
            file.str = std::string(entry_size, char{});
            archive.readNextFile(&file.str[0], entry_size);
            file_content_queue.push(std::move(file));
            ++q_idx;
        }
    }
    readEnd = true;
    std::cout << "FILES COUNT:" << q_idx << std::endl;
}

void merge_pair1(std::unordered_map<std::string, size_t> &first,
                 const std::unordered_map<std::string, size_t> &second,
                 queue_t<std::unordered_map<std::string, size_t>> *merge_queue){
    for (const auto& [word, count] : second) {
            first[word] += count;
    }
    merge_queue->push(std::move(first));
}

void merge_thread(queue_t<std::unordered_map<std::string, size_t>> *merge_queue, int i, size_t *working_threads, std::mutex *mu){
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
        merge_pair1(first_to_merge, second_to_merge, merge_queue);
    }
    mu->lock();
    --(*working_threads);
    mu->unlock();
    std::cout << "Thread {" << i << "} is dead." << std::endl;
}

std::vector<std::string> file_names(const std::string& path){
    using namespace std::literals::string_literals; //! This is OK!
    std::vector<std::string> archive_filenames;
    for (const auto& entry: fs::recursive_directory_iterator(path)) {
        auto ext = entry.path().extension().string();
        if (( ext == ".ZIP"s ) || ( ext == ".zip"s )) {
            archive_filenames.push_back(entry.path().string());
        }
    }

    return archive_filenames;
}

void sort(std::unordered_map<std::string, size_t>& M)
{
    std::vector<std::pair<std::string, int> > A{M.begin(), M.end()}; //! Навіщо в циклі? :-)

    using pairtype=std::pair<std::string,size_t>;
    sort(A.begin(), A.end(),
         [](const auto& a, const auto& b){ //! Ага, ще тут всі скопіюємо... Поставив const ref.
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
    //! This code is not reentrant! Do not use concurrently.
    boost::locale::generator gen;
    std::locale::global(gen("en_US.UTF-8"));

    std::string test_data_folder = "../test";
    //std::string full_data_folder = "../data";
    auto before = get_current_time_fenced();

    //// 1. Get archive filenames, and their number. (not parallel)
    std::vector<std::string> archive_filenames;

    archive_filenames = file_names(test_data_folder);
//    archive_filenames = std::vector(archive_filenames.begin(), archive_filenames.begin() + 15);
    std::cout << "number of archives: " << archive_filenames.size() << std::endl;
    std::cout << "First archive: " << archive_filenames[0] << std::endl;
    std::cout << "Last archive: " << archive_filenames[archive_filenames.size()-1] << std::endl;

    //// 2. Create an vector of words dictionaries, which size if the number of files. (not parallel)
    // std::vector<std::unordered_map<std::string, size_t>> files_words(archive_filenames.size());
    //// 3. Calculate words dictionaries for every file. (parallel)
    queue_t<std::unordered_map<std::string, size_t>> merge_queue;
    std::vector<std::thread> calc_threads;
    // std::thread calc_threads[n_calc_threads]; GCC only! Nonstandard!
    for(size_t i = 0; i < n_calc_threads; ++i) {
        calc_threads.emplace_back(calc_thread, i, &merge_queue); // Fixed rather unstylish wording.
    }

    std::thread readThread{read_thread, std::move(archive_filenames)}; //! Це варто раніше починати.
    //std::unordered_map<std::string, size_t> word_map;
    ////queue to MAP:
    readThread.join();
    for(size_t i = 0; i < n_calc_threads; ++i) {
        calc_threads[i].join();
    }
    std::cout << "ALL THREADS FINISHED!!!\n";
    std::vector<std::thread> merge_threads; // [n_merge_threads]; !non-standard, gcc only

    std::cout << "Merge queue size:" << merge_queue.size() << std::endl;

    size_t threads_left = n_merge_threads;
    std::mutex mu;
    for (size_t i = 0; i < n_merge_threads; ++i) {
        merge_threads.emplace_back(merge_thread, &merge_queue, i, &threads_left, &mu);
    }

    for (size_t i = 0; i < n_merge_threads; ++i) {
        merge_threads[i].join();
    }

    std::unordered_map<std::string, size_t> final_dict;

    merge_queue.pop(final_dict);

    auto max = get_max(final_dict);

    std::cout << max.first << ":" << max.second << std::endl;

    auto time_to_calculate = get_current_time_fenced() - before;
    std::cout << "Time: " << static_cast<int>(to_s(time_to_calculate) / 60)  << ":" << ((to_s(time_to_calculate) / 60) - static_cast<int>(to_s(time_to_calculate) / 60)) * 10 << "s" << std::endl;
    std::cout << final_dict.size() << std::endl;

    auto num = sort_by_num(final_dict);
    auto name = sort_by_name(final_dict);

    std::ofstream out("../by_name.txt");

    if (!out.is_open()) {
        throw std::runtime_error("couldn't open a file ");
    }
    for (auto& t : name) {
        out << t.first << " " << t.second << std::endl;
    }


    std::ofstream out1("../by_num.txt");

    if (!out1.is_open()) {
        throw std::runtime_error("couldn't open a file ");
    }
    for (auto& t : num) {
        out1 << t.first << " " << t.second << std::endl;
    }
    return 0;
}

