#ifndef FILETREE_H
#define FILETREE_H
#include <string>
#include <memory>


class FileTree
{
public:
    FileTree(const std::string &root);
    static bool isDir(std::string &root);
    void printTree();


    class FileTreeNode;
    class FileInfo;
private:
    void printTreeRecursive(std::shared_ptr<FileTreeNode> root, int level);
    void buildTree();
    void populateEntries(std::shared_ptr<FileTreeNode> &node);

    std::string root_path;
    std::shared_ptr<FileTreeNode> ft_root{nullptr};
};

#endif // FILETREE_H
