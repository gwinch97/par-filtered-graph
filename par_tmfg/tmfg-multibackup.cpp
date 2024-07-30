#include "partmfg_double.h"
#include "profiler.h"
#include "IO.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>
#include <fstream>
#include <direct.h> // For _mkdir on Windows
#include <filesystem>
#include <iomanip>
#include <string>
#include <sstream>
#include <regex>

namespace fs = std::filesystem;

// Function to create directory if it doesn't exist
void createDirectoryIfNotExists(const std::string& path) {
    struct stat info;

    if (stat(path.c_str(), &info) != 0) {
        // Directory does not exist, create it
        if (_mkdir(path.c_str()) != 0) {
            std::cerr << "Error creating directory: " << path << std::endl;
            exit(1);
        }
    } else if (!(info.st_mode & S_IFDIR)) {
        // Path exists but is not a directory
        std::cerr << path << " is not a directory!" << std::endl;
        exit(1);
    }
}

// Custom logging function
void log(const std::string& message, bool verbose) {
    if (verbose) {
        std::cout << message << std::endl;
    }
}

// Function to extract file number from filename
int extractFileNumber(const std::string& filename) {
    std::regex regex(R"(dsb-(\d+)\.bin)");
    std::smatch match;
    if (std::regex_match(filename, match, regex) && match.size() > 1) {
        return std::stoi(match.str(1));
    }
    return -1;
}

void runParTMFGD(SymM<double> *W, size_t n, size_t THRESHOLD, const std::string& method, bool use_heap, const std::string& dsname, int fileIndex, bool verbose){
    std::ostringstream oss;
    oss << "====" << std::endl;
    oss << "threshold: " << THRESHOLD << std::endl;
    oss << "method: " << method << std::endl;
    oss << "use_heap: " << use_heap << std::endl;
    log(oss.str(), verbose);

    // Ensure directories exist
    createDirectoryIfNotExists("./outputs");
    createDirectoryIfNotExists("./outputs/Ps");

    #ifdef PROFILE
        auto pf = Profiler();
    #else
        auto pf = DummyProfiler();
    #endif

    timer t2; t2.start();
    ParTMFGD computer = ParTMFGD(W, n, &pf, use_heap); 
    timer t; t.start();
    computer.init();           
    computer.initGainArray();                 
    int round = 0;

    if(method == "prefix"){ // get best gain by scanning vertex list
        while(computer.hasUninsertedV()){
            size_t round_THRESHOLD = std::min(THRESHOLD, computer.getTrianglesNum());
            auto insert_list = computer.getBestVertices(round_THRESHOLD);   
            computer.inertMultiple(insert_list);                 
            computer.updateGainArray(insert_list);                          
            round++;
        }
    } else if(method == "exact"){ // use exact
        while(computer.hasUninsertedV()){
            computer.insertOne();
            round++;
        }
    } else if(method == "naive"){ // naive method
        while(computer.hasUninsertedV()){
            auto insert_list = computer.getAllBestVertices(computer.getTrianglesNum()); 
            computer.inertMultiple(insert_list);                             
            computer.initGainArray();                                                   
            round++;
        }
    }
    // compute the total gain here
    oss.str(""); oss.clear(); // Clear the string stream
    oss << "tmfg total: " << t.next() << std::endl;
    oss << "round: " << round << std::endl;
    computer.computeCost();
    pf.report();
    log(oss.str(), verbose);

    std::string fileIndexStr = std::to_string(fileIndex);
    if (method == "exact" || method == "naive") {
        log("Outputting files for method: " + method, verbose);
        computer.outputP("./outputs/Ps/" + dsname + "-" + fileIndexStr + "-" + method + "-P-1");
    } else {
        computer.outputP("./outputs/Ps/" + dsname + "-" + fileIndexStr + "-" + method + "-P-" + std::to_string(THRESHOLD));
    }
}

// Function to display progress bar
void displayProgressBar(int current, int total) {
    int barWidth = 70;
    float progress = (float)current / total;
    std::cout << "[";
    int pos = barWidth * progress;
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0) << " %\r";
    std::cout.flush();
}

int main(int argc, char *argv[]) {
    if (argc < 7) {
        std::cerr << "Usage: " << argv[0] << " <directory> <dsname> <n> <method> <THRESHOLD> <round>" << std::endl;
        return 1;
    }

    std::string directory = argv[1];
    std::string dsname = argv[2];
    size_t n = std::atoi(argv[3]);
    std::string method = argv[4];
    size_t THRESHOLD = std::atoi(argv[5]);
    int round = std::atoi(argv[6]);
    bool verbose = false;

    std::cout << "workers: " << parlay::num_workers() << std::endl;

    std::vector<fs::path> files;
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (entry.is_regular_file()) {
            files.push_back(entry.path());
        }
    }

    int totalFiles = files.size();
    for (int i = 0; i < totalFiles; ++i) {
        std::string filepath = files[i].string();
        std::string filename = files[i].filename().string();

        int fileNumber = extractFileNumber(filename);
        if (fileNumber == -1) {
            std::cerr << "Error extracting file number from filename: " << filename << std::endl;
            continue;
        }

        timer t2; t2.start();
        SymM<double> W = IO::readSymMatrixFromFile<double>(filepath.c_str(), n); 
        W.setDiag(1);

        for (int r = 0; r < round; ++r) {
            runParTMFGD(&W, n, THRESHOLD, method, true, dsname, fileNumber, verbose);
        }

        displayProgressBar(i + 1, totalFiles);
    }

    std::cout << std::endl << "Processing completed." << std::endl;
    return 0;
}
