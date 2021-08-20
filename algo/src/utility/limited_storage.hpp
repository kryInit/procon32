#pragma once
#include <cmath>
#include <queue>
#include <vector>
#include <optional>
#include <iostream>

template<class T>
class LimitedStorage {
    using Index = size_t;
public:
    struct Ticket {
        size_t key; Index storage_idx;
        Ticket() : key(), storage_idx() {}
        Ticket(size_t key, Index storage_idx) : key(key), storage_idx(storage_idx) {}
        friend std::ostream& operator << (std::ostream& os, const Ticket& tkt) {
            os << "Ticket {key: " << tkt.key << ", idx: " << tkt.storage_idx << "}";
            return os;
        }
    };

private:
    struct Unit {
        size_t key; T data;
        Unit() : key(), data() {}
        Unit(size_t key, T data) : key(key), data(data) {}
        friend std::ostream& operator << (std::ostream& os, const Unit& unit) {
            os << "Unit {key: " << unit.key << ", data: " << unit.data << "}";
            return os;
        }
    };

    const size_t max_storage_size;
    size_t pushed_count;
    std::queue<Index> popped;
    std::queue<Ticket> pushed;
    std::vector<Unit> storage;

    size_t get_insertable_storage_idx() {
        if (!popped.empty()) {
            Index idx = popped.front();
            popped.pop();
            return idx;
        }
        else if (storage.size() < max_storage_size) return storage.size();
        else {
            while(storage[pushed.front().storage_idx].key != pushed.front().key) { pushed.pop(); }
            Index idx = pushed.front().storage_idx;
            pushed.pop();
            return idx;
        }
    }

public:
    explicit LimitedStorage(size_t max_storage_size) : max_storage_size(max_storage_size), pushed_count(0), storage() {}
    void reserve(size_t n) { storage.reserve(std::min(n, max_storage_size)); }
    const Ticket push(const T& data) {
        size_t key = pushed_count++;
        Index idx = get_insertable_storage_idx();
        if (idx == storage.size()) storage.emplace_back(key, data);
        else storage[idx] = Unit(key, data);
        pushed.emplace(key, idx);
        return Ticket(key, idx);
    }
    std::optional<T> pop(const Ticket& tkt) {
        if (storage[tkt.storage_idx].key != tkt.key) return std::nullopt;
        popped.push(tkt.storage_idx);
        storage[tkt.storage_idx].key = -1;
        return move(storage[tkt.storage_idx].data);
    }
    [[nodiscard]] std::optional<T> get(const Ticket& tkt) const {
        if (storage[tkt.storage_idx].key != tkt.key) return std::nullopt;
        return storage[tkt.storage_idx].data;
    }
    bool erase(const Ticket& tkt) {
        if (storage[tkt.storage_idx].key != tkt.key) return false;
        popped.push(tkt.storage_idx);
        storage[tkt.storage_idx].key = -1;
        return true;
    }
    void dump(std::ostream& os) {
        os << "limited storage dump";
        os << " ============ storage info ============ " << endl;
        os << "max_storage_size: " << max_storage_size << endl;
        os << "pushed_count: " << pushed_count << endl;
        for(int i=0; i < storage.size(); ++i) os << "storage[" << i << "]: " << storage[i] << endl;
        os << " ====================================== " << endl;
    }
    void dump() { dump(std::cout); }
    void edump() { dump(std::cerr); }
};
