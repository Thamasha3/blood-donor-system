#ifndef PRIORITY_HEAP_H
#define PRIORITY_HEAP_H

#include <string>
#include <vector>
#include <iostream>

struct EmergencyRequest {
    std::string requestId;
    std::string bloodGroupNeeded;
    int urgencyScore; // 3 = Critical, 2 = Urgent, 1 = Normal

    // Overload operators for simple comparison in the heap array
    bool operator<(const EmergencyRequest& other) const {
        return urgencyScore < other.urgencyScore;
    }
};

class PriorityHeap {
private:
    std::vector<EmergencyRequest> heap;

    void heapifyUp(int index) {
        while (index > 0) {
            int parent = (index - 1) / 2;
            if (heap[index].urgencyScore > heap[parent].urgencyScore) {
                std::swap(heap[index], heap[parent]);
                index = parent;
            } else break;
        }
    }

    void heapifyDown(int index) {
        int size = heap.size();
        while (2 * index + 1 < size) {
            int left = 2 * index + 1;
            int right = 2 * index + 2;
            int largest = left;

            if (right < size && heap[right].urgencyScore > heap[left].urgencyScore) {
                largest = right;
            }
            if (heap[largest].urgencyScore > heap[index].urgencyScore) {
                std::swap(heap[index], heap[largest]);
                index = largest;
            } else break;
        }
    }

public:
    // FIX: Renamed from insertRequest to insert to match main.cpp
    void insert(std::string id, std::string blood, int score) {
        heap.push_back({id, blood, score});
        heapifyUp(heap.size() - 1);
    }

    EmergencyRequest extractMax() {
        if (heap.empty()) return {"", "", 0};
        EmergencyRequest topPriority = heap[0];
        heap[0] = heap.back();
        heap.pop_back();
        heapifyDown(0);
        return topPriority;
    }

    bool isEmpty() { return heap.empty(); }

    std::vector<EmergencyRequest> getRawHeap() { return heap; }
};

#endif
