#include <iostream>
#include "FileTree.h"
#include <stdexcept>

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

    cout << "Succesfully Opened dir: " << path << endl;

    //tree->printTree();

    cout << "Scanned " << tree->insert_count << " files." << endl;

    return 0;
}
