#include <generator>
#include <memory>

typedef char u8;

constexpr std::size_t PoolSize = 512;
alignas(std::max_align_t) static char memoryPool[PoolSize];

// Memory block structure for tracking allocations
struct Block {
    Block* next;
    std::size_t size;
    char* address;
};

typedef long syscall_t;

#define SYS_write 1

// Define our own entry point
void print(const char* msg) {
    int n = 0;
    const char* p = msg;
    while (*p++) n++;
    syscall_t ret;

    // Perform the write syscall to output the message
    asm volatile (
        "movq %[sys_write], %%rax\n"  // syscall number for write
        "movq %[stdout], %%rdi\n"     // file descriptor (stdout = 1)
        "movq %[message], %%rsi\n"    // pointer to message
        "movq %[len], %%rdx\n"        // length of message
        "syscall\n"                    // invoke syscall
        "movq %%rax, %[ret]\n"         // store return value
        : [ret] "=r" (ret)
        : [sys_write] "i" (SYS_write),
          [stdout] "r" ((syscall_t)1),
          [message] "r" (msg),
          [len] "r" ((syscall_t)(n))
        : "rax", "rdi", "rsi", "rdx"
    );
}

static Block* freeList = nullptr;  // Free list to track available blocks
static std::size_t poolOffset = 0; // Offset to track current position in the pool

// Custom Static Allocator with deallocation support
template <typename T>
struct StaticAllocator {
    using value_type = T;

    StaticAllocator() noexcept = default;

    template <typename U>
    constexpr StaticAllocator(const StaticAllocator<U>&) noexcept {}

    // Allocate memory from the static memory pool
    T* allocate(std::size_t n) {
	const char msg[]{'A', 'L', ' ', '0' + n / 10, '0' + n % 10, '\n', 0};
	print(msg);
        std::size_t requiredSize = n * sizeof(T);

        // Search the free list for an available block
        Block** prev = &freeList;
        for (Block* current = freeList; current; current = current->next) {
            if (current->size >= requiredSize) {
                // We found a block that fits the requested size
                *prev = current->next;  // Remove this block from the free list
                return reinterpret_cast<T*>(current->address);
            }
            prev = &current->next;
        }

        // If no free block is found, allocate from the static pool
        if (poolOffset + requiredSize > PoolSize) {
	    const char str[]{'B', 'A', 'D', '\n', 0};
	    print(str);
            throw std::bad_alloc();  // Out of memory in the static pool
        }

        T* result = reinterpret_cast<T*>(memoryPool + poolOffset);
        poolOffset += requiredSize;
        return result;
    }

    // Deallocate memory and add it to the free list
    void deallocate(T* p, std::size_t n) noexcept {
	const char msg[]{'D', 'E', ' ', '0' + n / 10, '0' + n % 10, '\n', 0};
	print(msg);
        std::size_t blockSize = n * sizeof(T);
        char* blockAddress = reinterpret_cast<char*>(p);

        // Add the block to the free list
        Block* newBlock = reinterpret_cast<Block*>(blockAddress);
        newBlock->address = blockAddress;
        newBlock->size = blockSize;
        newBlock->next = freeList;
        freeList = newBlock;
    }

    // Rebind allocator to another type
    template <typename U>
    struct rebind {
        using other = StaticAllocator<U>;
    };
};

// Allocators of different types are considered equal
template <typename T, typename U>
bool operator==(const StaticAllocator<T>&, const StaticAllocator<U>&) { return true; }

template <typename T, typename U>
bool operator!=(const StaticAllocator<T>& a, const StaticAllocator<U>& b) { return !(a == b); }

struct DeviceBase {
	u8 bus;
	u8 deviceno;
	u8 function;
};

#define co_yield_from(expr) co_yield std::ranges::elements_of(expr, StaticAllocator<DeviceBase>{})

using gen = std::generator<DeviceBase, void, StaticAllocator<DeviceBase>>;

gen enumerate_pci_bus(u8 bus);

gen enumerate_function(u8 bus, u8 deviceno, u8 function) {
    auto device = DeviceBase{bus, deviceno, function};
    co_yield device;

    // recurse into PCI to PCI bridges
    if (bus == 0 && deviceno == 0 && function == 0) {
        co_yield_from(enumerate_pci_bus(8));
    }
}

gen enumerate_device(u8 bus, u8 deviceno) {
    u8 function = 0;
    co_yield_from(enumerate_function(bus, deviceno, function));
    // must check - some devices report details for function 0 for all functions
    if (1) {
        for (u8 function = 1; function < 8; ++function) {
            co_yield_from(enumerate_function(bus, deviceno, function));
        }
    }
}

gen enumerate_pci_bus(u8 bus) {
    //static std::bitset<256> enumerated_buses;
    //if (enumerated_buses[bus]) co_return;
    //enumerated_buses[bus] = 1;
    for (u8 device = 0; device < 32; ++device) {
        co_yield_from(enumerate_device(bus, device));
    }
}

gen attached_devices() {
    // rely on firmware to configure PCI buses properly (setting up PCI to PCI bridges to forward requests
    // from one bus to another)
    // Systems must provide a mechanism that allows software to generate PCI configuration transactions
    // This mechanism is typically located in the host bridge
    // This spec specifies the mechanism for PC-AT compatible systems
    // Main host bridge/controller at (0, 0, 0)
    if (1) {
        co_yield_from(enumerate_pci_bus(0));
    } else {
        // Other PCI host bridges at (0, 0, X), responsible for bus X
        for (u8 bus_as_function_number = 0; bus_as_function_number < 8; ++bus_as_function_number) {
            co_yield_from(enumerate_pci_bus(bus_as_function_number));
        }
    }
}

int main() {
	print("hi\n");
	for (auto x : attached_devices()) {
		const char msg[]{'0' + x.bus / 10, '0' + x.bus % 10, ' ', '0' + x.deviceno / 10, '0' + x.deviceno % 10, ' ', '0' + x.function / 10, '0' + x.function / 10, '\n', 0};
		print(msg);
	}
	return 42;
}
