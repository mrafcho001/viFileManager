#ifndef FILETREE_H
#define FILETREE_H
#include <string>
#include <memory>


class FileTree
{
public:
    class FileTreeNode;
    class FileInfo;

    FileTree(const std::string &root);
    void performScan();


    // Scan across all filesystems or limit to current filesystem only
    void crossFileSystems(bool flag);


    //--------- Static Functions ---------
    static bool isDir(const std::string &root);


    //--------- Debugging functions ---------
    void printTree();

    //--------- Statistics -----------
    unsigned int getTotalEntries() const;
    unsigned int getFileCount() const;
    unsigned int getDirectoryCount() const;
    unsigned int getStatErrorCount() const;


private:
    //--------- Helper functions ---------
    void processDirRecursive(std::shared_ptr<FileTreeNode> &node);
    void resetStatistics();

    
    //--------- Helper Debugging Functions ---------
    void printTreeRecursive(std::shared_ptr<FileTreeNode> root, int level);


    //--------- Internal variables ---------
    std::string root_path;
    std::shared_ptr<FileTreeNode> ft_root{nullptr};


    //--------- Options ---------
    // Can we scan across all filesystems (setby crossFileSystems)
    bool crossFS{true};
    unsigned long int rootDevID{0};


    //--------- Useful information/statistics ---------
    int statisticsFileCount{0};
    int statisticsDirCount{0};
    int statisticsStatErrors{0};
};

#endif // FILETREE_H
