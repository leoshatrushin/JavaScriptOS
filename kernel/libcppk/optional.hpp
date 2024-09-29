#pragma once
#include "move.hpp"

namespace mystd {
    template <typename T>
class optional {
public:
    // Constructor: no value by default
    optional() : initialized(false) {}

    // Constructor with value
    optional(const T& value) {
        construct(value);
    }

    // Destructor: explicitly call the destructor of the contained value if initialized
    ~optional() {
        reset();
    }

    // Copy constructor
    optional(const optional& other) {
        if (other.initialized) {
            construct(other.value());
        }
    }

    // Move constructor
    optional(optional&& other) noexcept {
        if (other.initialized) {
            construct(mystd::move(other.value()));
            other.reset();
        }
    }

    // Assignment operator
    optional& operator=(const T& new_value) {
        if (initialized) {
            value_ref() = new_value;  // Assign to existing value
        } else {
            construct(new_value);  // Construct new value
        }
        return *this;
    }

    // Check if the value is initialized
    bool has_value() const {
        return initialized;
    }

    // Get the value (unsafe version, assumes the value is present)
    T& value() {
        return value_ref();
    }

    const T& value() const {
        return value_ref();
    }

    // Reset the optional (destroy the contained value if any)
    void reset() {
        if (initialized) {
            value_ref().~T();  // Call the destructor explicitly
            initialized = false;
        }
    }

private:
    // Raw storage for the value (no constructor is called here)
    alignas(T) char storage[sizeof(T)];
    bool initialized;

    // Helper function to access the value
    T& value_ref() {
        return *reinterpret_cast<T*>(&storage);
    }

    const T& value_ref() const {
        return *reinterpret_cast<const T*>(&storage);
    }

    // Helper function to construct a new value in the storage
    void construct(const T& new_value) {
        new (&storage) T(new_value);  // Placement new: construct the value in place
        initialized = true;
    }

    void construct(T&& new_value) {
        new (&storage) T(mystd::move(new_value));  // Placement new for move semantics
        initialized = true;
    }
};
}
