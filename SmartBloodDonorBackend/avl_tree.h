#ifndef AVL_TREE_H
#define AVL_TREE_H

#include <string>
#include <iostream>
#include <vector>

struct Donor {
    std::string id;
    std::string name;
    std::string bloodGroup;
    std::string district;
    std::string lastDonationDate; // Format: YYYY-MM-DD
    bool isAvailable;
};

struct AVLNode {
    Donor donor;
    AVLNode* left;
    AVLNode* right;
    int height;

    AVLNode(Donor d) : donor(d), left(nullptr), right(nullptr), height(1) {}
};

class AVLTree {
private:
    AVLNode* root;

    int height(AVLNode* n) { return n ? n->height : 0; }
    int getBalance(AVLNode* n) { return n ? height(n->left) - height(n->right) : 0; }

    AVLNode* rightRotate(AVLNode* y) {
        AVLNode* x = y->left;
        AVLNode* T2 = x->right;
        x->right = y;
        y->left = T2;
        y->height = std::max(height(y->left), height(y->right)) + 1;
        x->height = std::max(height(x->left), height(x->right)) + 1;
        return x;
    }

    AVLNode* leftRotate(AVLNode* x) {
        AVLNode* y = x->right;
        AVLNode* T2 = y->left;
        y->left = x;
        x->right = T2;
        x->height = std::max(height(x->left), height(x->right)) + 1;
        y->height = std::max(height(y->left), height(y->right)) + 1;
        return y;
    }

    AVLNode* insertNode(AVLNode* node, Donor d) {
        if (!node) return new AVLNode(d);
        if (d.id < node->donor.id)
            node->left = insertNode(node->left, d);
        else if (d.id > node->donor.id)
            node->right = insertNode(node->right, d);
        else return node;

        node->height = 1 + std::max(height(node->left), height(node->right));
        int balance = getBalance(node);

        if (balance > 1 && d.id < node->left->donor.id) return rightRotate(node);
        if (balance < -1 && d.id > node->right->donor.id) return leftRotate(node);
        if (balance > 1 && d.id > node->left->donor.id) {
            node->left = leftRotate(node->left);
            return rightRotate(node);
        }
        if (balance < -1 && d.id < node->right->donor.id) {
            node->right = rightRotate(node->right);
            return leftRotate(node);
        }
        return node;
    }

    // Finds the node with the smallest key value in a given subtree (Inorder Successor)
    AVLNode* minValueNode(AVLNode* node) {
        AVLNode* current = node;
        while (current && current->left != nullptr)
            current = current->left;
        return current;
    }

    // Recursive helper to execute deletions and handle AVL tree height violations
    AVLNode* deleteNode(AVLNode* root, std::string id) {
        if (!root) return root;

        // 1. Perform standard BST deletion steps
        if (id < root->donor.id) {
            root->left = deleteNode(root->left, id);
        } else if (id > root->donor.id) {
            root->right = deleteNode(root->right, id);
        } else {
            // Node matches the target deletion key ID!
            if ((root->left == nullptr) || (root->right == nullptr)) {
                AVLNode* temp = root->left ? root->left : root->right;

                if (!temp) {
                    temp = root;
                    root = nullptr;
                } else {
                    *root = *temp; // Copy contents of non-empty child node
                }
                delete temp;
            } else {
                // Node has two active children: get the inorder successor
                AVLNode* temp = minValueNode(root->right);
                root->donor = temp->donor; // Pull metadata up into current position
                root->right = deleteNode(root->right, temp->donor.id); // Delete successor node
            }
        }

        if (!root) return root;

        // 2. Update height values for the current node block
        root->height = 1 + std::max(height(root->left), height(root->right));

        // 3. Inspect balance factors to check for height constraint breaches
        int balance = getBalance(root);

        // Case 1: Left Left (LL Rotation)
        if (balance > 1 && getBalance(root->left) >= 0)
            return rightRotate(root);

        // Case 2: Left Right (LR Rotation)
        if (balance > 1 && getBalance(root->left) < 0) {
            root->left = leftRotate(root->left);
            return rightRotate(root);
        }

        // Case 3: Right Right (RR Rotation)
        if (balance < -1 && getBalance(root->right) <= 0)
            return leftRotate(root);

        // Case 4: Right Left (RL Rotation)
        if (balance < -1 && getBalance(root->right) > 0) {
            root->right = rightRotate(root->right);
            return leftRotate(root);
        }

        return root;
    }

    AVLNode* searchNode(AVLNode* node, std::string id) {
        if (!node || node->donor.id == id) return node;
        if (id < node->donor.id) return searchNode(node->left, id);
        return searchNode(node->right, id);
    }

    void collectInorder(AVLNode* node, std::vector<Donor>& list) {
        if (!node) return;
        collectInorder(node->left, list);
        list.push_back(node->donor);
        collectInorder(node->right, list);
    }

public:
    AVLTree() : root(nullptr) {}

    void insert(Donor d) { root = insertNode(root, d); }

    // Public exposure member to trigger node deletions cleanly from main.cpp
    void remove(std::string id) { root = deleteNode(root, id); }

    Donor search(std::string id) {
        AVLNode* res = searchNode(root, id);
        if (res) return res->donor;
        return {"", "", "", "", "", false};
    }

    std::vector<Donor> filterDonors(std::string blood, std::string dist) {
        std::vector<Donor> allDonors;
        std::vector<Donor> filtered;
        collectInorder(root, allDonors);

        for (const auto& d : allDonors) {
            if ((blood == "ALL" || d.bloodGroup == blood) && (dist == "ALL" || d.district == dist)) {
                filtered.push_back(d);
            }
        }
        return filtered;
    }
};

#endif
