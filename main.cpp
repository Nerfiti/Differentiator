#include <cstdio>

#include "Differentiator.hpp"
#include "logs.hpp"
#include "MyGeneralFunctions.hpp"
#include "Syntax_analyzer.hpp"

int main(const int argc, const char *argv[])
{
    initLog();

    const char *filename = (argc == 2) ? argv[1] : "./funcfile";
    FILE *input_file = fopen(filename, "r");

    FILE *texfile = fopen(OUT_TEX_FILE, "w");

    AnalyseFunction(input_file);

    closeLog();
}