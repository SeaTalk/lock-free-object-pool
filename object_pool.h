#pragma once
#include <algorithm>
#include <atomic>
#include <vector>
#include <memory>

namespace stone::util {
template<typename T>
class ObjectPool {
 public:
    struct Node;
    struct CountedNodePtr {
        CountedNodePtr() noexcept : external_cnt(1), node_idx(-1) { }
        uint32_t external_cnt;
        int32_t node_idx;
    };
    struct Node {
        explicit Node(T *s) : obj(s) { }
        T *obj;
        int node_idx;
        CountedNodePtr next;
    };

    template<typename ...Args>
    ObjectPool(size_t num, Args ...args) {
        objs_.reserve(num);
        nodes_.reserve(num);
        for (size_t i = 0; i < num; ++i) {
            objs_.emplace_back(new T(std::forward(args)...));
            nodes_.emplace_back(objs_.back().get());
            nodes_.back().node_idx = i;
            nodes_[i].next.node_idx = i - 1;
        }
        CountedNodePtr head;
        head.node_idx = nodes_.size() - 1;
        entry_.store(head);
    }

    void Reclaim(CountedNodePtr nd) {
        if (nd.node_idx < 0) { return; }
        ++nd.external_cnt;
        nodes_[nd.node_idx].next = entry_.load(std::memory_order_relaxed);
        while(!entry_.compare_exchange_weak(
                nodes_[nd.node_idx].next, nd,
                std::memory_order_release, std::memory_order_relaxed));
    }

    CountedNodePtr GetOne() {
        if (nodes_.empty()) { return CountedNodePtr(); }
        CountedNodePtr old = entry_.load(std::memory_order_relaxed);
        while (true) {
            old = IncreaseEntryCnt(old);
            auto node_idx = old.node_idx;
            if (node_idx < 0) { break; }
            Node *node = &nodes_[node_idx];
            if (entry_.compare_exchange_strong(old, node->next,
                        std::memory_order_relaxed)) {
                return old;
            }
        }
        return CountedNodePtr();
    }

    T *GetObj(CountedNodePtr &node_cnt) {
        if (node_cnt.node_idx < 0) { return nullptr; }
        return nodes_[node_cnt.node_idx].obj;
    }

 private:
    CountedNodePtr IncreaseEntryCnt(CountedNodePtr &old_cnt) {
        CountedNodePtr new_cnt;
        do {
            new_cnt = old_cnt;
            ++new_cnt.external_cnt;
        } while (!entry_.compare_exchange_strong(
                    old_cnt, new_cnt,
                    std::memory_order_acquire,
                    std::memory_order_relaxed));
        return new_cnt;
    }

    std::vector<std::unique_ptr<T>> objs_;
    std::vector<Node> nodes_;
    std::atomic<CountedNodePtr> entry_;
};
}   // namespace stone::util

