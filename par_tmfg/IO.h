#pragma once

#include <iostream>
#include <fstream>
#include <string>

#include "parlay/parallel.h"
#include "parlay/primitives.h"

#include "symmatrix.h"


namespace IO {
  using namespace std;

  template <class T, class Seq>
  SymM<T> parseSymMatrix(Seq W, std::size_t n) {
    SymM<T> matrix = SymM<T>(n);
    parlay::parallel_for(0, n, [&](size_t i){
        parlay::parallel_for(i+1, n,[&] (size_t j){
          matrix.update(i, j, (T)W[i*n + j]);
        });
    });
    return matrix;
  }

  // read a symmatric matrix from file
    template <class T>
    SymM<T> readSymMatrixFromFile(char const *fname, std::size_t n) {
    parlay::sequence<double> W = parlay::sequence<double>(n*n);
    std::ifstream myFile(fname, std::ios::in | std::ios::binary);
    if (!myFile) {
        std::cerr << "Error opening file: " << fname << std::endl;
        abort();
    }
    myFile.read((char*)W.data(), sizeof(double) * n*n);
    if (W.size() == 0) {
        std::cerr << "readPointsFromFile empty file" << std::endl;
        abort();
    }
    if (W.size() % n != 0) {
        std::cerr << "readPointsFromFile wrong file type or wrong dimension" << std::endl;
        abort();
    }
    std::cout << "Matrix loaded successfully from file: " << fname << std::endl;
    // Print a few values for debugging
    std::cout << "Matrix values:" << std::endl;
    for (std::size_t i = 0; i < std::min(n, (std::size_t)5); ++i) {
        for (std::size_t j = 0; j < std::min(n, (std::size_t)5); ++j) {
            std::cout << W[i * n + j] << " ";
        }
       std::cout << std::endl;
    }
    return parseSymMatrix<T>(W.cut(0, W.size()), n);
}
}