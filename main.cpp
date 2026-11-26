#include <cstdint>
#include <cstdio>
#include <memory>
#include <vector>

class Input {
    char buffer[65536];
    size_t pos = 0, length = 0;
    int get() {
        if (pos == length) {
            length = std::fread(buffer, 1, sizeof(buffer), stdin);
            pos = 0;
            if (!length) return EOF;
        }
        return buffer[pos++];
    }
public:
    bool read(int64_t& value) {
        int c;
        do { c = get(); } while (c != EOF && c <= ' ');
        if (c == EOF) return false;
        bool negative = c == '-';
        if (negative) c = get();
        uint64_t magnitude = 0;
        while (c >= '0' && c <= '9') {
            magnitude = magnitude * 10 + (c - '0');
            c = get();
        }
        value = negative ? -static_cast<int64_t>(magnitude - 1) - 1
                         : static_cast<int64_t>(magnitude);
        return true;
    }
};

class Sets {
    struct Node {
        int64_t key = 0;
        uint32_t left = 0, right = 0, size = 0, priority = 0;
    };
    static constexpr uint32_t block_size = 4096;
    std::vector<std::unique_ptr<Node[]>> blocks;
    uint32_t next = 1, free_head = 0;
    uint64_t random_state = 0x123456789abcdefULL;
    Node& node(uint32_t id) { return blocks[id / block_size][id % block_size]; }
    uint32_t size(uint32_t id) { return id ? node(id).size : 0; }
    void update(uint32_t id) {
        node(id).size = 1 + size(node(id).left) + size(node(id).right);
    }
    uint32_t allocate() {
        if (free_head) {
            uint32_t id = free_head;
            free_head = node(id).left;
            return id;
        }
        uint32_t id = next++;
        if (id / block_size == blocks.size())
            blocks.push_back(std::make_unique<Node[]>(block_size));
        return id;
    }
    uint32_t random() {
        random_state ^= random_state >> 12;
        random_state ^= random_state << 25;
        random_state ^= random_state >> 27;
        return (random_state * 2685821657736338717ULL) >> 32;
    }
    uint32_t rotate(uint32_t root, bool left) {
        uint32_t child = left ? node(root).right : node(root).left;
        if (left) {
            node(root).right = node(child).left;
            node(child).left = root;
        } else {
            node(root).left = node(child).right;
            node(child).right = root;
        }
        update(root);
        update(child);
        return child;
    }
    uint32_t merge(uint32_t a, uint32_t b) {
        if (!a || !b) return a ? a : b;
        if (node(a).priority > node(b).priority) {
            node(a).right = merge(node(a).right, b);
            update(a);
            return a;
        }
        node(b).left = merge(a, node(b).left);
        update(b);
        return b;
    }
public:
    uint32_t insert(uint32_t root, int64_t key, bool& added) {
        if (!root) {
            root = allocate();
            node(root) = Node{key, 0, 0, 1, random()};
            added = true;
            return root;
        }
        if (key == node(root).key) return root;
        if (key < node(root).key) {
            node(root).left = insert(node(root).left, key, added);
            if (node(node(root).left).priority > node(root).priority)
                root = rotate(root, false);
        } else {
            node(root).right = insert(node(root).right, key, added);
            if (node(node(root).right).priority > node(root).priority)
                root = rotate(root, true);
        }
        update(root);
        return root;
    }
    uint32_t erase(uint32_t root, int64_t key) {
        if (!root) return 0;
        if (key == node(root).key) {
            uint32_t replacement = merge(node(root).left, node(root).right);
            node(root).left = free_head;
            free_head = root;
            return replacement;
        }
        if (key < node(root).key) node(root).left = erase(node(root).left, key);
        else node(root).right = erase(node(root).right, key);
        update(root);
        return root;
    }
    uint32_t copy(uint32_t root) {
        if (!root) return 0;
        uint32_t result = allocate();
        node(result) = node(root);
        node(result).left = copy(node(root).left);
        node(result).right = copy(node(root).right);
        return result;
    }
    bool contains(uint32_t root, int64_t key) {
        while (root) {
            if (key == node(root).key) return true;
            root = key < node(root).key ? node(root).left : node(root).right;
        }
        return false;
    }
    uint32_t rank(uint32_t root, int64_t key, bool inclusive) {
        uint32_t result = 0;
        while (root) {
            if (node(root).key < key || (inclusive && node(root).key == key)) {
                result += size(node(root).left) + 1;
                root = node(root).right;
            } else root = node(root).left;
        }
        return result;
    }
    bool neighbor(uint32_t root, int64_t& key, bool forward) {
        uint32_t best = 0;
        while (root) {
            if (forward ? node(root).key > key : node(root).key < key) {
                best = root;
                root = forward ? node(root).left : node(root).right;
            } else root = forward ? node(root).right : node(root).left;
        }
        if (!best) return false;
        key = node(best).key;
        return true;
    }
};

int main() {
    Input input;
    Sets sets;
    std::vector<uint32_t> roots(25, 0);
    size_t last_set = 0;
    size_t iterator_set = 0;
    int64_t iterator_key = 0, op, a = 0, b = 0, c = 0;
    bool valid = false;
    while (input.read(op)) {
        if (op <= 4) input.read(a);
        if (op == 0 || op == 1 || op == 3 || op == 4) input.read(b);
        if (op == 4) input.read(c);
        if (op == 0) {
            bool added = false;
            roots[a] = sets.insert(roots[a], b, added);
            if (added) { iterator_set = a; iterator_key = b; valid = true; }
        } else if (op == 1) {
            if (valid && iterator_set == static_cast<size_t>(a) && iterator_key == b)
                valid = false;
            roots[a] = sets.erase(roots[a], b);
        } else if (op == 2) {
            uint32_t copied = sets.copy(roots[a]);
            if (++last_set == roots.size()) roots.push_back(copied);
            else roots[last_set] = copied;
        } else if (op == 3) {
            bool found = sets.contains(roots[a], b);
            std::puts(found ? "true" : "false");
            if (found) { iterator_set = a; iterator_key = b; valid = true; }
        } else if (op == 4) {
            uint32_t count = b > c ? 0 : sets.rank(roots[a], c, true)
                                             - sets.rank(roots[a], b, false);
            std::printf("%u\n", count);
        } else {
            if (valid) valid = sets.neighbor(roots[iterator_set], iterator_key, op == 6);
            std::printf("%lld\n", static_cast<long long>(valid ? iterator_key : -1));
        }
    }
}
