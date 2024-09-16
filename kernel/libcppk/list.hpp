#pragma once
#include "cstddef.hpp"
#include "allocator_traits.hpp"

namespace std {

template <typename T, typename A>
class list {
private:
    struct Node {
        T data;
        Node* prev;
        Node* next;

        Node(const T& value) : data(value), prev(nullptr), next(nullptr) {}
    };

    Node* head;
    Node* tail;
    size_t list_size;
    A allocator;
    typedef allocator_traits<A> traits_type;

public:
    list(const A& alloc = A()) : head(nullptr), tail(nullptr), list_size(0), allocator(alloc) {}

    ~list() {
        clear();
    }

    void push_front(const T& value) {
        Node* new_node = traits_type::allocate(allocator, 1);
        try {
            new (new_node) Node(value);
        } catch(...) {
            traits_type::deallocate(allocator, new_node, 1);
            throw;
        }

        new_node->next = head;
        if (head)
            head->prev = new_node;
        head = new_node;
        if (!tail)
            tail = new_node;
        ++list_size;
    }

    void push_back(const T& value) {
        Node* new_node = traits_type::allocate(allocator, 1);
        try {
            new (new_node) Node(value);
        } catch(...) {
            traits_type::deallocate(allocator, new_node, 1);
            throw;
        }

        new_node->prev = tail;
        if (tail)
            tail->next = new_node;
        tail = new_node;
        if (!head)
            head = new_node;
        ++list_size;
    }

    void pop_front() {
        if (!head) return;
        Node* temp = head;
        head = head->next;
        if (head)
            head->prev = nullptr;
        else
            tail = nullptr;
        temp->~Node();
        traits_type::deallocate(allocator, temp, 1);
        --list_size;
    }

    void pop_back() {
        if (!tail) return;
        Node* temp = tail;
        tail = tail->prev;
        if (tail)
            tail->next = nullptr;
        else
            head = nullptr;
        temp->~Node();
        traits_type::deallocate(allocator, temp, 1);
        --list_size;
    }

    size_t size() const {
        return list_size;
    }

    void clear() {
        while (head) {
            pop_front();
        }
    }

    class iterator {
    private:
        Node* current;
    public:
        iterator(Node* node) : current(node) {}

        T& operator*() const { return current->data; }

        iterator& operator++() { 
            if (current) current = current->next; 
            return *this; 
        }

        bool operator!=(const iterator& other) const { 
            return current != other.current; 
        }
    };

    iterator begin() { return iterator(head); }
    iterator end() { return iterator(nullptr); }
};

}
