#include "FileTree.h"
#include <stdexcept>
#include <vector>
#include <queue>
#include <iostream>


extern "C" {
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
}

std::string concatenatePath(const std::string &path, const std::string &entry);
std::string concatenatePath(const std::string &path, const char* entry);

enum class ErrorStatus : int { OK, CannotStat, CannotOpen };
class FileTree::FileInfo
{
public:
    FileInfo();
    FileInfo(const std::string &file_name, const std::string &path, const struct stat &entry_stat);


    bool isFile() const;
    bool isDirectory() const;
    std::string absolutePath() const;
    std::string getName() const;


    void setErrorStatus(const ErrorStatus &errorStatus);

private:
    ino_t     inode;
    mode_t    mode;
    short     user_id;
    short     group_id;
    off_t     total_size;
    time_t    modified_time;

    std::string name;
    std::string path;

    ErrorStatus error{ErrorStatus::OK};
};


class FileTree::FileTreeNode
{
public:
    FileTreeNode();
    FileTreeNode(const std::weak_ptr<FileTreeNode> &parent_node,
                 const FileInfo &fi);

    const FileInfo& entryInfo() const;
    FileInfo& entryInfo();

    void addEntry(std::shared_ptr<FileTreeNode> &child);


    const std::vector<std::shared_ptr<FileTreeNode>> children() const;
    std::vector<std::shared_ptr<FileTreeNode>>       children();

private:
    std::weak_ptr<FileTreeNode>                parent;
    std::vector<std::shared_ptr<FileTreeNode>> entries;

    FileTree::FileInfo fileInfo;
};





FileTree::FileTree(const std::string &root):
    root_path(root)
{
    if(!isDir(root_path))
    {
        throw std::runtime_error(std::string("Path is not a directory: '") + root + "'");
    }

    buildTree();
}

bool FileTree::isDir(std::string &root)
{
    struct stat stat_buf;
    int ret_val = stat(root.c_str(), &stat_buf);

    if(ret_val != 0)
        return false;

    if(S_ISDIR(stat_buf.st_mode))
    {
        return true;
    }
    return false;
}

void FileTree::printTree()
{
    std::cout << ft_root->entryInfo().absolutePath() << ":" << std::endl;
    printTreeRecursive(ft_root, 1);
}

void FileTree::printTreeRecursive(std::shared_ptr<FileTree::FileTreeNode> root, int level)
{
    std::string indentation((size_t)level*2, (char)' ');

    for(auto it : root->children())
    {
        std::cout << indentation << it->entryInfo().getName() << std::endl;
        if(it->entryInfo().isDirectory())
            printTreeRecursive(it, level+1);
    }

}

void FileTree::buildTree()
{
    struct stat stat_buf;

    if(ft_root)
    {
        /* TODO: Delete tree */
    }

    int ret_val = stat(root_path.c_str(), &stat_buf);
    if(ret_val)
    {
        throw std::runtime_error(std::string("Could not stat root path: ") + root_path);
    }

    ft_root = std::make_shared<FileTreeNode>(
                FileTreeNode(std::weak_ptr<FileTreeNode>(),
                             FileInfo("", root_path, stat_buf)));

    std::cout << "ft_root: " << ft_root << std::endl;

    /* If its not a directory, there is nothing to do */
    if(!ft_root->entryInfo().isDirectory())
    {
        throw std::runtime_error(std::string("Path is not a directory ('" + root_path + "')"));
    }

    processDir_rec(ft_root);
    
}

void FileTree::processDir_rec(std::shared_ptr<FileTreeNode> &node)
{
    struct stat stat_buf;
    std::string fullPath {node->entryInfo().absolutePath()};
    DIR *dir = opendir(fullPath.c_str());
    if(!dir)
    {
        std::cout << "Cannot open: '" << fullPath << "' with error: " << errno << std::endl;
        node->entryInfo().setErrorStatus(ErrorStatus::CannotOpen);
        return;
    }

    struct dirent *dirEntry = nullptr;
    while( (dirEntry = readdir(dir)) != nullptr )
    {
        if(dirEntry->d_name[0] == '.' && (dirEntry->d_name[1] == '\0' || dirEntry->d_name[1] == '.'))
            continue;
        int ret_val = stat(concatenatePath(fullPath, dirEntry->d_name).c_str(), &stat_buf);

        std::shared_ptr<FileTreeNode> new_node =
                std::make_shared<FileTreeNode>(node, FileInfo(std::string(dirEntry->d_name), fullPath, stat_buf));
        node->addEntry(new_node);
        insert_count++;

        if(ret_val)
        {
            std::cout << "Could not stat '" << new_node->entryInfo().getName() << "'" << std::endl;
            new_node->entryInfo().setErrorStatus(ErrorStatus::CannotStat);
        }
    }

    closedir(dir);

    for(auto node_itr : node->children())
    {
        if(node_itr->entryInfo().isDirectory())
        {
            processDir_rec(node_itr);
        }
    }
}







std::string concatenatePath(const std::string &path, const std::string &entry)
{
#ifdef _WIN32
    if(path.back() != '\\')
        return path + "\\" + entry;
    return path + entry;
#endif
    if(path.back() != '/')
        return path + "/" + entry;
    return path + entry;
}

std::string concatenatePath(const std::string &path, const char* entry)
{
#ifdef _WIN32
    return path + "\\" + entry;
#endif
    return path + "/" + entry;
}







FileTree::FileInfo::FileInfo()
{
}

FileTree::FileInfo::FileInfo(const std::string &file_name,
                             const std::string &path,
                             const struct stat &entry_stat):
    name{file_name}, path{path}
{
    inode         = entry_stat.st_ino;
    mode          = entry_stat.st_mode;
    user_id       = entry_stat.st_uid;
    group_id      = entry_stat.st_gid;
    total_size    = entry_stat.st_size;
    modified_time = entry_stat.st_mtime;
}

bool FileTree::FileInfo::isFile() const
{
    return S_ISREG(mode);
}

bool FileTree::FileInfo::isDirectory() const
{
    return S_ISDIR(mode);
}

std::string FileTree::FileInfo::absolutePath() const
{
    return concatenatePath(path, name);
}

std::string FileTree::FileInfo::getName() const
{
    return name;
}

void FileTree::FileInfo::setErrorStatus(const ErrorStatus &errorStatus)
{
    error = errorStatus;
}







FileTree::FileTreeNode::FileTreeNode()
{
    parent = std::weak_ptr<FileTreeNode>();
}

FileTree::FileTreeNode::FileTreeNode(
        const std::weak_ptr<FileTree::FileTreeNode> &parent_node,
        const FileInfo &fi):
    parent{parent_node}, fileInfo{fi}
{

}

const FileTree::FileInfo &FileTree::FileTreeNode::entryInfo() const
{
    return fileInfo;
}

FileTree::FileInfo &FileTree::FileTreeNode::entryInfo()
{
    return fileInfo;
}

void FileTree::FileTreeNode::addEntry(std::shared_ptr<FileTree::FileTreeNode> &child)
{
    entries.push_back(child);
}

const std::vector<std::shared_ptr<FileTree::FileTreeNode> > FileTree::FileTreeNode::children() const
{
    return entries;
}

std::vector<std::shared_ptr<FileTree::FileTreeNode> > FileTree::FileTreeNode::children()
{
    return entries;
}
