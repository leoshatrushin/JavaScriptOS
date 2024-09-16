#pragma once
#include "../libk/string.h"

class PathString {
public:
    PathString(const char* path) : str(path) {}

    class Iterator {
    public:
        Iterator(const char* path) : pos(path), pathend(strchr(path, '\0')) {
            if (pos != pathend) {
                extractNext();
            }
        }

        const char* operator*() const {
            return part;
        }

        Iterator& operator++() {
            if (pos != pathend) {
                extractNext();
            }
            return *this;
        }

        bool operator==(const Iterator& other) const {
            return (pos == other.pos) && (pathend == other.pathend);
        }

        bool operator!=(const Iterator& other) const {
            return !(*this == other);
        }

        bool is_last() const {
            return pos == pathend;
        }

    private:
        const char* pos;
        const char* pathend;
        char part[128];

        void extractNext() {
            const char* start = pos;
            while (*start == '/') start++;
            if (!start) {
                pos = pathend;
                part[0] = '\0';
                return;
            }

            char* end = strchr(start, '/');

            size_t len = end - start;
            if (len >= sizeof(part)) {
                len = sizeof(part) - 1;
            }

            strncpy(part, start, len);
            part[len] = '\0';

            pos = end;
            while (*pos == '/') pos++;
        }
    };

    Iterator begin() const {
        return Iterator(str);
    }

    Iterator end() const {
        return Iterator(strchr(str, '\0'));
    }

private:
    const char* str;
};
