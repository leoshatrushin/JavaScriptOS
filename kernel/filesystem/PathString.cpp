/*#include <iterator>*/
/*#include "PathString.h"*/
/*#include "../libk/string.h"*/
/**/
/*class PathString {*/
/*public:*/
/*    PathString(const char* path) : path(path) {}*/
/**/
/*    class Iterator {*/
/*    public:*/
/*        Iterator(const std::string& path, size_t pos) : path(path), current_pos(pos) {*/
/*            if (pos != strlen(path)) {*/
/*                extractNext();*/
/*            }*/
/*        }*/
/**/
/*        const char* operator*() const {*/
/*            return part;*/
/*        }*/
/**/
/*        Iterator& operator++() {*/
/*            if (pos != partsz) {*/
/*                extractNext();*/
/*            }*/
/*            return *this;*/
/*        }*/
/**/
/*    private:*/
/*        size_t pos;*/
/*        char part[128];*/
/**/
/*        // Helper function to extract the next part*/
/*        void extractNext() {*/
/*            size_t start = pos;*/
/*            size_t end = strchr(path + start, '/');*/
/**/
/*            if (end == std::string::npos) {*/
/*                current_part_ = path_.substr(start);*/
/*                current_pos_ = std::string::npos;*/
/*            } else {*/
/*                if (end == start) {*/
/*                    // Skip empty parts (consecutive '/')*/
/*                    current_pos_ = end + 1;*/
/*                    extractNext();*/
/*                    return;*/
/*                }*/
/*                current_part_ = path_.substr(start, end - start);*/
/*                current_pos_ = end + 1;*/
/*            }*/
/*        }*/
/*    };*/
/**/
/*    Iterator begin() const {*/
/*        size_t startpos = 0;*/
/*        while (path[startpos] == '/') startpos++;*/
/*        if (!path[startpos]) return end();*/
/*        return Iterator(path, startpos);*/
/*    }*/
/**/
/*    Iterator end() const {*/
/*        return Iterator(path, strlen(path));*/
/*    }*/
/**/
/*private:*/
/*    const char* path;*/
/*};*/
/**/
/*// Example usage*/
/*int main() {*/
/*    // Example Unix path*/
/*    PathString unixPath("/home/user/documents/report.txt");*/
/**/
/*    std::cout << "Unix Path Components:" << std::endl;*/
/*    for (const auto& part : unixPath) {*/
/*        std::cout << part << std::endl;*/
/*    }*/
/**/
/*    return 0;*/
/*}*/
