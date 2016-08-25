#include <iostream>
#include "FileTree.h"
#include <stdexcept>
#include <chrono>

using namespace std;

int main(int argc, char *argv[])
{
    std::unique_ptr<FileTree> tree;
    string path{"/lib"};
    cout << "Path: " << path << endl;
    try {
        tree = std::make_unique<FileTree>(path);

    }catch (exception &e)
    {
        cout << "Provided path is not a valid direcotry... exiting" << endl;
        return 0;
    }

    tree->crossFileSystems(false);

    auto start = chrono::system_clock::now();
    tree->performScan();
    auto end = chrono::system_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end-start); 

    cout << "Scan took: " << duration.count() << endl;
    cout << "Scanned Total: " << tree->getTotalEntries()
         << ", (F/D): " << tree->getFileCount() << "/" << tree->getDirectoryCount()
         << " with " << tree->getStatErrorCount() << " stat errors." << endl;

    return 0;
}
