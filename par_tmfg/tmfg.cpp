// #include "dbht.h"
#include "partmfg_double.h"
#include "profiler.h"
#include "IO.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>
#include <fstream>
#include <direct.h> // For _mkdir on Windows

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

void runDBHT(SymM<double> *W, SymM<double> *D, size_t n, size_t THRESHOLD, string method, bool use_heap, string dsname = ""){
    cout << "====" << endl;
    cout << "threshold: " << THRESHOLD << endl;
    cout << "method: " << method << endl;
    cout << "use_heap: " << use_heap << endl;

    // Ensure directories exist
    createDirectoryIfNotExists("./outputs");
    createDirectoryIfNotExists("./outputs/Zs");
    createDirectoryIfNotExists("./outputs/Ps");

    #ifdef PROFILE
        auto pf = Profiler();
    #else
        auto pf = DummyProfiler();
    #endif

    timer t2;t2.start();
    ParTMFGD computer = ParTMFGD(W, n, &pf, use_heap); 
    timer t;t.start();
    computer.init();           
    computer.initGainArray();                 pf.setInitTime(t2.next());
    auto clusterer = new ParDBHTTMFGD(computer.cliques.data(), computer.triangles.data(), n, computer.W, computer.P.data(), D, &pf);
    int round=0;

    if(method == "prefix"){ //get best gain by scanning vertex list
        while(computer.hasUninsertedV()){
                size_t round_THRESHOLD = min(THRESHOLD, computer.getTrianglesNum());
                auto insert_list = computer.getBestVertices(round_THRESHOLD);   pf.incVTime(t2.next());
                computer.inertMultiple(insert_list, clusterer);                 pf.incInsertTime(t2.next());
                computer.updateGainArray(insert_list);                          pf.incUpdTime(t2.next());
    #ifdef DEBUG
            computer.checkTriangles();
    #endif
                round++;
        } //while end
    }else if(method == "exact"){ //use exact
        while(computer.hasUninsertedV()){
            computer.insertOne(clusterer);
            round++;
        } //while end
    }else if(method == "naive"){ //naive method
        while(computer.hasUninsertedV()){
            auto insert_list = computer.getAllBestVertices(computer.getTrianglesNum()); pf.incVTime(t2.next());
            computer.inertMultiple(insert_list, clusterer);                             pf.incInsertTime(t2.next());
            computer.initGainArray();                                                   pf.incUpdTime(t2.next());
            round++;
        } //while end
    }
    // compute the total gain here
    cout << "tmfg total: "<< t.next() << endl;
    cout << "round: " << round << endl;
    computer.computeCost();
    pf.report();

    // Comment out clustering-related methods
	//t.next();
    //clusterer->APSP();
    //cout << "APSP total: "<< t.next() << endl;
    //clusterer->computeDirection();
    //cout << "direction total: "<< t.next() << endl;
    //clusterer->nonDiscreteClustering();
    //cout << "non-discrete total: "<< t.next() << endl;
    //clusterer->assignToConvergingBubble(); // need to test
    //cout << "discrete total: "<< t.next() << endl;
    //cout << "num cluster: "<< clusterer->nc << endl;
    //clusterer->assignToBubble(); // need to test
    //cout << "bubble total: "<< t.next() << endl;
    //clusterer->buildHierarchy();
    //cout << "hierarchy total: "<< t.next() << endl;

    if (method == "exact" || method == "naive") {
        std::cout << "Outputting files for method: " << method << std::endl;
        computer.outputP("./outputs/Ps/" + dsname + "-" + method + "-P-1");
        clusterer->outputDendro("./outputs/Zs/" + dsname + "-" + method + "-Z-1");
    } else {
        computer.outputP("./outputs/Ps/" + dsname + "-" + method + "-P-" + std::to_string(THRESHOLD));
        clusterer->outputDendro("./outputs/Zs/" + dsname + "-" + method + "-Z-" + std::to_string(THRESHOLD));
    }
}

void runTMFG(SymM<double> *W, SymM<double> *D, size_t n, size_t THRESHOLD, string method, bool use_heap, string dsname = ""){
    cout << "====" << endl;
    cout << "threshold: " << THRESHOLD << endl;
    cout << "method: " << method << endl;
    cout << "use_heap: " << use_heap << endl;

    // Ensure directories exist
    createDirectoryIfNotExists("./outputs");
    createDirectoryIfNotExists("./outputs/Zs");
    createDirectoryIfNotExists("./outputs/Ps");

    #ifdef PROFILE
        auto pf = Profiler();
    #else
        auto pf = DummyProfiler();
    #endif

    timer t2;t2.start();
    ParTMFGD computer = ParTMFGD(W, n, &pf, use_heap); 
    timer t;t.start();
    computer.init();           
    computer.initGainArray();                 pf.setInitTime(t2.next());
    auto clusterer = new ParDBHTTMFGD(computer.cliques.data(), computer.triangles.data(), n, computer.W, computer.P.data(), D, &pf);
    int round=0;

    if(method == "prefix"){ //get best gain by scanning vertex list
        while(computer.hasUninsertedV()){
                size_t round_THRESHOLD = min(THRESHOLD, computer.getTrianglesNum());
                auto insert_list = computer.getBestVertices(round_THRESHOLD);   pf.incVTime(t2.next());
                computer.inertMultiple(insert_list, clusterer);                 pf.incInsertTime(t2.next());
                computer.updateGainArray(insert_list);                          pf.incUpdTime(t2.next());
    #ifdef DEBUG
            computer.checkTriangles();
    #endif
                round++;
        } //while end
    }else if(method == "exact"){ //use exact
        while(computer.hasUninsertedV()){
            computer.insertOne(clusterer);
            round++;
        } //while end
    }else if(method == "naive"){ //naive method
        while(computer.hasUninsertedV()){
            auto insert_list = computer.getAllBestVertices(computer.getTrianglesNum()); pf.incVTime(t2.next());
            computer.inertMultiple(insert_list, clusterer);                             pf.incInsertTime(t2.next());
            computer.initGainArray();                                                   pf.incUpdTime(t2.next());
            round++;
        } //while end
    }
    // compute the total gain here
    cout << "tmfg total: "<< t.next() << endl;
    cout << "round: " << round << endl;
    computer.computeCost();
    pf.report();

    // Comment out clustering-related methods
	//t.next();
    //clusterer->APSP();
    //cout << "APSP total: "<< t.next() << endl;
    //clusterer->computeDirection();
    //cout << "direction total: "<< t.next() << endl;
    //clusterer->nonDiscreteClustering();
    //cout << "non-discrete total: "<< t.next() << endl;
    //clusterer->assignToConvergingBubble(); // need to test
    //cout << "discrete total: "<< t.next() << endl;
    //cout << "num cluster: "<< clusterer->nc << endl;
    //clusterer->assignToBubble(); // need to test
    //cout << "bubble total: "<< t.next() << endl;
    //clusterer->buildHierarchy();
    //cout << "hierarchy total: "<< t.next() << endl;

    if (method == "exact" || method == "naive") {
        std::cout << "Outputting files for method: " << method << std::endl;
        computer.outputP("./outputs/Ps/" + dsname + "-" + method + "-P-1");
        clusterer->outputDendro("./outputs/Zs/" + dsname + "-" + method + "-Z-1");
    } else {
        computer.outputP("./outputs/Ps/" + dsname + "-" + method + "-P-" + std::to_string(THRESHOLD));
        clusterer->outputDendro("./outputs/Zs/" + dsname + "-" + method + "-Z-" + std::to_string(THRESHOLD));
    }
}


int main(int argc, char *argv[]) {
char* filename = argv[1];
bool use_heap = 1;
string dsname = argv[2];
size_t n = atoi(argv[3]);
char* distance_filename = argv[4]; //if 0, use sqrt(2*(1-w)) by default
string method = argv[5];
size_t THRESHOLD = atoi(argv[6]); // the prefix to insert, only used when method='prefix
int round = atoi(argv[7]);

cout << "workers: " << parlay::num_workers() << endl;
timer t2;t2.start();
SymM<double> W = IO::readSymMatrixFromFile<double>(filename, n); 
W.setDiag(1);
for(int r=0;r<round;++r){
    if(distance_filename[0]!='0'){
        SymM<double> D = IO::readSymMatrixFromFile<double>(distance_filename, n);
        cout << "read: " << t2.next() << endl;
        runTMFG(&W, &D, n, THRESHOLD, method, use_heap, dsname);
    }else{
        cout << "read: " << t2.next() << endl;
        runTMFG(&W, nullptr, n, THRESHOLD, method, use_heap, dsname);
    }
}
}
