////////////////////////////////////////////////////////////////////////
///
/// Copyright (c) 2019, Wasserman lab
///
/// FILE        ftsearch.cpp
///
/// DESCRIPTION This file contains the definition of ftsearch. The class can
///             be summarized as the init point for the application.
///             The init function calls other parts of the application
///
/// Initial version @ Godfrain Jacques Kounkou
///
////////////////////////////////////////////////////////////////////////

#include "ftsearch.h"

namespace ft {

//======================================================================
FTSearch::FTSearch()
    : _utils(&_ownedUtils)
    , _stats(&_ownedStats)
    , _writerBridge(&_ownedWriterBridge)
    , _kmerGenerator(&_ownedKmerGenerator)
    , _finder(&_ownedFinder)
    , _resultProcessor(&_ownedResultProcessor)
    , _queryExtractor(&_ownedQueryExtractor)
{
}

//======================================================================
void FTSearch::init(const fs::path& pathToQueryFile, uint kmerSize, uint readLength,
                    const fs::path& indexFileLocation, const fs::path& outputFolder,
                    bool refOnly, SearchType searchType, bool multithread, const fs::path& inputFastQ,
                    uint overlap, bool returnMatchesOnly, bool kmerCounts,
                    uint stride, uint maxOccurences, uint threadNumber, bool ignoreNonUniqueKmers, bool crossover,
                    bool printSearchTime)
{
    fs::path readFile;

    if (inputFastQ.empty()) {
        readFile.stem() = _utils->trimmedReadFileName(indexFileLocation); // want to remove the "Indexes_" at the beginning
    } else {
        readFile = inputFastQ;
    }

    fs::path queryOutputFile;

    queryOutputFile  = outputFolder;
    queryOutputFile /= pathToQueryFile.stem() += std::string("_") += readFile.stem() += "_Results.tsv";
    std::set<fs::path> setOfIndexes = _utils->getSetOfIndexes();
    std::set<Query> inputQueries    = _queryExtractor->getInputQueries(refOnly, crossover, pathToQueryFile);

    KmerMap kmerMap;
    _kmerGenerator->genKmerMap(inputQueries, kmerSize, refOnly, searchType, kmerMap, overlap, stride, crossover, ignoreNonUniqueKmers, kmerCounts);
    std::cout << "kmerMap size                  : " << kmerMap.size() << std::endl;

    std::cout << "\nsearching..." << std::endl;
    MapOfCounts indexCounts;

    auto indexFile = setOfIndexes.begin()->c_str();

    ResultsMap indexPosResults;

    // selecting the correct strategy depending on the size of the index size set
    if (setOfIndexes.size() == 1) {
        std::cout << "searching with " << setOfIndexes.size() << " indexes" << std::endl;
        _finder->searchMonoIndex(indexPosResults, kmerMap, indexFile, indexFileLocation, maxOccurences,
                                 multithread, threadNumber, printSearchTime);

    } else if (setOfIndexes.size() > 1) {
        std::cout << "searching with " << setOfIndexes.size() << " indexes" << std::endl;
        _finder->searchMultipleIndexes(indexPosResults, kmerMap, setOfIndexes, indexFileLocation,
                                       maxOccurences, multithread, threadNumber, printSearchTime);
    }

    fs::path indexMapFile = indexFile;
    indexMapFile += ".map";

    indexCounts = _resultProcessor->processResults(indexPosResults, readLength);
    _writerBridge->saveQueryOutput(indexCounts, returnMatchesOnly, crossover, pathToQueryFile, queryOutputFile);
}

//======================================================================
void FTSearch::overrideUtils(std::shared_ptr<IUtils> utils)
{
    _utils = utils.get();
}

//======================================================================
void FTSearch::overrideStats(std::shared_ptr<IStats> stats)
{
    _stats = stats.get();
}

//======================================================================
void FTSearch::overrideWriterBridge(std::shared_ptr<IWriterBridge> writerBridge)
{
    _writerBridge = writerBridge.get();
}

//======================================================================
void FTSearch::overrideKmerGenerator(std::shared_ptr<IKmerGenerator> kmerGenerator)
{
    _kmerGenerator = kmerGenerator.get();
}

//======================================================================
void FTSearch::overrideFinder(std::shared_ptr<IFinder> finder)
{
    _finder = finder.get();
}

//======================================================================
void FTSearch::overrideResultProcessor(std::shared_ptr<IResultProcessor> resultProcessor)
{
    _resultProcessor = resultProcessor.get();
}

//======================================================================
void FTSearch::overrideQueryExtractor(std::shared_ptr<IQueryExtractor> queryExtractor)
{
    _queryExtractor = queryExtractor.get();
}

//======================================================================
FTSearch::~FTSearch()
{
}
}