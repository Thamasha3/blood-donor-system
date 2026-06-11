#ifndef DOUBLY_LINKED_LIST_H
#define DOUBLY_LINKED_LIST_H

#include <string>
#include <iostream>
#include <vector>

struct DonationRecord {
    std::string date;
    std::string hospital;
    int units;
};

struct DLLNode {
    DonationRecord record;
    DLLNode* next;
    DLLNode* prev;

    DLLNode(DonationRecord r) : record(r), next(nullptr), prev(nullptr) {}
};

class DoublyLinkedList {
private:
    DLLNode* head;
    DLLNode* tail;

public:
    DoublyLinkedList() : head(nullptr), tail(nullptr) {}

    void addRecord(std::string date, std::string hospital, int units) {
        DonationRecord r = {date, hospital, units};
        DLLNode* newNode = new DLLNode(r);
        if (!head) {
            head = tail = newNode;
        } else {
            tail->next = newNode;
            newNode->prev = tail;
            tail = newNode;
        }
    }

    std::vector<DonationRecord> getAllRecords() {
        std::vector<DonationRecord> list;
        DLLNode* current = head;
        while (current) {
            list.push_back(current->record);
            current = current->next;
        }
        return list;
    }
};

#endif
