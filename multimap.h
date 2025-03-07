/*
================================================================================
Multimap.h – Detailed Annotated Version
================================================================================

Purpose:
This file implements a template class "Multimap" that supports multiple values 
per key. Internally, the data is stored in a Left-Leaning Red-Black tree (LLRB). 
Each tree node holds a key and a vector of associated values. In addition to the 
usual map operations (insertion, lookup, removal), this container allows duplicate 
keys (each duplicate value is stored in the vector). Self-balancing is maintained 
via rotations, color flips, and other helper functions to guarantee average-case 
O(log N) complexity for most operations.

Structure:
1. Public API:
   - Functions for getting the size, checking emptiness, inserting key-value 
     pairs, removing a value for a given key (removing only the first value if 
     multiple exist), querying for the first value, and retrieving minimum/maximum 
     keys.
2. Private Members and Helper Functions:
   - The internal Node structure (with key, values vector, color, left/right children)
   - Iterative and recursive helper methods for search, insertion, removal, and 
     in-order printing.
3. Self-balancing Helpers:
   - Functions to check node color, rotate left/right, flip colors, move red 
     links, and fix up the tree structure.
   
Each line in the implementation is annotated in detail to explain its purpose.
================================================================================
*/

#ifndef MULTIMAP_H_
#define MULTIMAP_H_

#include <iostream>     // For basic I/O (if needed for debugging)
#include <memory>       // For std::unique_ptr
#include <vector>       // For std::vector to store multiple values per key
#include <string>       // For string handling (if needed)
#include <utility>      // For std::pair and std::move
#include <stdexcept>    // For throwing runtime errors

// Template class Multimap supporting keys of type K and values of type V.
template <typename K, typename V>
class Multimap {
 public:
  // ---------------- Public API ----------------

  // Returns the total number of key-value pairs stored in the tree.
  unsigned int Size();
  
  // Returns a reference to the first value associated with the given key.
  // Throws a runtime_error if the key is not found or has no associated values.
  const V& Get(const K& key);
  
  // Checks whether the multimap contains the specified key.
  bool Contains(const K& key);
  
  // Returns a reference to the maximum key in the tree.
  // Throws a runtime_error if the tree is empty.
  const K& Max();
  
  // Returns a reference to the minimum key in the tree.
  // Throws a runtime_error if the tree is empty.
  const K& Min();
  
  // Inserts a key-value pair into the tree.
  // If the key already exists, appends the value to the existing vector.
  void Insert(const K &key, const V &value);
  
  // Removes the first occurrence of the value associated with the given key.
  // If the key has only one value, the entire node is removed.
  void Remove(const K &key);
  
  // Prints the tree in an in-order traversal.
  void Print();

 private:
  // ---------------- Internal Data Structures ----------------

  // Color enumeration for red-black balancing.
  enum Color { RED, BLACK };
  
  // Node structure for the LLRB tree.
  struct Node {
    K key;                   // The key stored in the node.
    std::vector<V> values;   // Vector of values associated with this key.
    bool color;              // Node color: RED or BLACK.
    std::unique_ptr<Node> left;   // Left child pointer.
    std::unique_ptr<Node> right;  // Right child pointer.
    
    // Constructor: initializes node with key, an empty vector of values,
    // default color RED, and null left/right children.
    Node(const K &k)
      : key(k), values(), color(RED), left(nullptr), right(nullptr) {}
  };

  // Root of the red-black tree.
  std::unique_ptr<Node> root;
  // Total number of key-value pairs (each occurrence counts) in the multimap.
  unsigned int cur_size = 0;

  // ---------------- Helper Methods ----------------

  // Iterative helper: search for a node with the given key, starting at node n.
  // Returns a raw pointer to the found node, or nullptr if not found.
  Node* Get(Node *n, const K &key);

  // Recursive helper: finds the minimum node (leftmost) in the subtree rooted at n.
  Node* Min(Node *n);
  
  // Recursive helper: inserts a key-value pair into the subtree rooted at n.
  void Insert(std::unique_ptr<Node> &n, const K &key, const V &value);
  
  // Recursive helper: removes one occurrence of the key from the subtree rooted at n.
  void Remove(std::unique_ptr<Node> &n, const K &key);
  
  // Recursive helper: performs an in-order traversal of the subtree rooted at n and prints nodes.
  void Print(Node *n);

  // ---------------- Self-Balancing Helper Methods ----------------

  // Checks if node n is red; returns false if n is null.
  bool IsRed(Node *n);
  
  // Flips the color of node n and its immediate children.
  void FlipColors(Node *n);
  
  // Performs a right rotation on the subtree rooted at prt.
  void RotateRight(std::unique_ptr<Node> &prt);
  
  // Performs a left rotation on the subtree rooted at prt.
  void RotateLeft(std::unique_ptr<Node> &prt);
  
  // Fixes up the subtree rooted at n to restore red-black properties.
  void FixUp(std::unique_ptr<Node> &n);
  
  // Moves a red node to the right in preparation for deletion from the right subtree.
  void MoveRedRight(std::unique_ptr<Node> &n);
  
  // Moves a red node to the left in preparation for deletion from the left subtree.
  void MoveRedLeft(std::unique_ptr<Node> &n);
  
  // Deletes the minimum node in the subtree rooted at n.
  void DeleteMin(std::unique_ptr<Node> &n);
};

// ============================================================================
// Implementation of Public API Methods
// ============================================================================

// Returns the total number of key-value pairs stored in the multimap.
template <typename K, typename V>
unsigned int Multimap<K, V>::Size() {
  return cur_size;  // Return the maintained count.
}

// ============================================================================
// Implementation of Iterative Helper Methods
// ============================================================================

// Iteratively searches for a node with the given key, starting at node n.
template <typename K, typename V>
typename Multimap<K, V>::Node* Multimap<K, V>::Get(Node *n, const K &key) {
  while (n) {                         // Loop until reaching a null pointer.
    if (key == n->key)                // If the current node's key matches...
      return n;                       // ...return this node.
    if (key < n->key)                 // If search key is less...
      n = n->left.get();              // ...move to left child.
    else                              // Otherwise...
      n = n->right.get();             // ...move to right child.
  }
  return nullptr;                     // Key not found.
}

// ============================================================================
// Implementation of Public API: Get, Contains, Max, Min
// ============================================================================

// Returns the first value associated with the given key.
// Throws runtime_error if key not found or value list is empty.
template <typename K, typename V>
const V& Multimap<K, V>::Get(const K &key) {
  Node *n = Get(root.get(), key);     // Search for node from root.
  if (!n)
    throw std::runtime_error("Error: cannot find key"); // Key missing.
  if (n->values.empty())
    throw std::runtime_error("Error: key has no values"); // No values stored.
  return n->values.front();           // Return the first stored value.
}

// Returns true if the multimap contains the given key.
template <typename K, typename V>
bool Multimap<K, V>::Contains(const K &key) {
  return Get(root.get(), key) != nullptr; // Key exists if search result is non-null.
}

// Returns the maximum key in the multimap.
// Traverses to the rightmost node.
template <typename K, typename V>
const K& Multimap<K, V>::Max(void) {
  Node *n = root.get();               // Start at root.
  if (!n)
    throw std::runtime_error("Error: empty tree"); // If tree is empty.
  while (n->right)                    // While right child exists...
    n = n->right.get();               // ...move to right child.
  return n->key;                      // Return key of rightmost node.
}

// Returns the minimum key in the multimap.
template <typename K, typename V>
const K& Multimap<K, V>::Min(void) {
  Node *n = Min(root.get());          // Get leftmost node recursively.
  if (!n)
    throw std::runtime_error("Error: empty tree");
  return n->key;                      // Return its key.
}

// ============================================================================
// Implementation of Recursive Helper Methods
// ============================================================================

// Recursively finds the minimum node in the subtree rooted at n.
template <typename K, typename V>
typename Multimap<K, V>::Node* Multimap<K, V>::Min(Node *n) {
  if (!n)
    return nullptr;                   // Return null if node is null.
  if (n->left)
    return Min(n->left.get());        // Recurse left if possible.
  else
    return n;                         // Found the leftmost node.
}

// ============================================================================
// Implementation of Self-Balancing Helper Methods
// ============================================================================

// Returns true if node n is red; null is considered black.
template <typename K, typename V>
bool Multimap<K, V>::IsRed(Node *n) {
  if (!n) return false;
  return (n->color == RED);
}

// Flips the color of node n and its two children.
template <typename K, typename V>
void Multimap<K, V>::FlipColors(Node *n) {
  n->color = !n->color;                // Flip color of current node.
  n->left->color = !n->left->color;    // Flip color of left child.
  n->right->color = !n->right->color;  // Flip color of right child.
}

// Performs a right rotation on the subtree rooted at prt.
template <typename K, typename V>
void Multimap<K, V>::RotateRight(std::unique_ptr<Node> &prt) {
  std::unique_ptr<Node> chd = std::move(prt->left);  // Temporarily hold left child.
  prt->left = std::move(chd->right);                 // Assign right subtree of child to prt's left.
  chd->color = prt->color;                           // Inherit parent's color.
  prt->color = RED;                                  // Set parent's color to RED.
  chd->right = std::move(prt);                       // Make original prt the right child.
  prt = std::move(chd);                              // Update prt to new subtree root.
}

// Performs a left rotation on the subtree rooted at prt.
template <typename K, typename V>
void Multimap<K, V>::RotateLeft(std::unique_ptr<Node> &prt) {
  std::unique_ptr<Node> chd = std::move(prt->right); // Temporarily hold right child.
  prt->right = std::move(chd->left);                 // Assign left subtree of child to prt's right.
  chd->color = prt->color;                           // Inherit parent's color.
  prt->color = RED;                                  // Set parent's color to RED.
  chd->left = std::move(prt);                         // Make original prt the left child.
  prt = std::move(chd);                              // Update prt to new subtree root.
}

// Fixes up the subtree rooted at n to restore red-black properties.
template <typename K, typename V>
void Multimap<K, V>::FixUp(std::unique_ptr<Node> &n) {
  if (IsRed(n->right.get()) && !IsRed(n->left.get()))
    RotateLeft(n);                                   // Rotate left if right child is red.
  if (IsRed(n->left.get()) && IsRed(n->left->left.get()))
    RotateRight(n);                                  // Rotate right if left grandchild is red.
  if (IsRed(n->left.get()) && IsRed(n->right.get()))
    FlipColors(n.get());                             // Flip colors if both children are red.
}

// Moves a red node to the right in preparation for deletion.
template <typename K, typename V>
void Multimap<K, V>::MoveRedRight(std::unique_ptr<Node> &n) {
  FlipColors(n.get());                               // Flip colors at current node.
  if (IsRed(n->left->left.get())) {
    RotateRight(n);                                  // Rotate right if left-left child is red.
    FlipColors(n.get());                             // Flip colors after rotation.
  }
}

// Moves a red node to the left in preparation for deletion.
template <typename K, typename V>
void Multimap<K, V>::MoveRedLeft(std::unique_ptr<Node> &n) {
  FlipColors(n.get());                               // Flip colors at current node.
  if (IsRed(n->right->left.get())) {
    RotateRight(n->right);                           // Rotate right on right child.
    RotateLeft(n);                                   // Then rotate left on current node.
    FlipColors(n.get());                             // Flip colors after rotations.
  }
}

// Deletes the minimum node in the subtree rooted at n.
template <typename K, typename V>
void Multimap<K, V>::DeleteMin(std::unique_ptr<Node> &n) {
  if (!n->left) {                                    // If no left child, this is minimum.
    n = nullptr;                                     // Delete node by setting pointer to null.
    return;
  }
  if (!IsRed(n->left.get()) && !IsRed(n->left->left.get()))
    MoveRedLeft(n);                                  // Adjust subtree if necessary.
  DeleteMin(n->left);                                // Recurse into left subtree.
  FixUp(n);                                        // Fix up subtree after deletion.
}

// ============================================================================
// Implementation of Public API: Remove, Insert, Print
// ============================================================================

// Public Remove: removes one occurrence of the key's value.
// If the key has multiple values, only the first value is removed.
template <typename K, typename V>
void Multimap<K, V>::Remove(const K &key) {
  if (!Contains(key))
    return;                                          // If key not found, do nothing.
  Remove(root, key);                                 // Call recursive removal.
  cur_size--;                                        // Decrement total count.
  if (root)
    root->color = BLACK;                             // Ensure root remains black.
}

// Recursive Remove: removes one occurrence of key from subtree rooted at n.
template <typename K, typename V>
void Multimap<K, V>::Remove(std::unique_ptr<Node> &n, const K &key) {
  if (!n) return;                                    // Base case: if node is null, return.
  
  if (key < n->key) {
    if (!IsRed(n->left.get()) && !IsRed(n->left->left.get()))
      MoveRedLeft(n);                                // Adjust left subtree if needed.
    Remove(n->left, key);                            // Recurse into left subtree.
  } else {
    if (IsRed(n->left.get()))
      RotateRight(n);                                // Rotate right if left child is red.
    if (key == n->key && !n->right) {                // Found key and no right child.
      if (n->values.size() == 1) {
        n = nullptr;                                 // Remove node entirely.
      } else {
        n->values.erase(n->values.begin());         // Else, remove first value.
      }
      return;
    }
    if (!IsRed(n->right.get()) && !IsRed(n->right->left.get()))
      MoveRedRight(n);                               // Adjust right subtree if needed.
    if (key == n->key) {
      if (n->values.size() == 1) {
        Node *n_min = Min(n->right.get());           // Find minimum node in right subtree.
        n->key = n_min->key;                           // Replace key with successor's key.
        n->values = std::move(n_min->values);          // Move successor's values.
        DeleteMin(n->right);                           // Delete successor node.
      } else {
        n->values.erase(n->values.begin());          // Remove first value if multiple.
      }
    } else {
      Remove(n->right, key);                           // Recurse into right subtree.
    }
  }
  FixUp(n);                                          // Fix subtree to restore properties.
}

// Public Insert: inserts a key-value pair.
template <typename K, typename V>
void Multimap<K, V>::Insert(const K &key, const V &value) {
  Insert(root, key, value);                          // Recursively insert into tree.
  cur_size++;                                        // Increment total count.
  if (root)
    root->color = BLACK;                             // Ensure root is black.
}

// Recursive Insert: inserts a key-value pair into subtree rooted at n.
template <typename K, typename V>
void Multimap<K, V>::Insert(std::unique_ptr<Node> &n, const K &key, const V &value) {
  if (!n) {
    // Create a new node with key and initialize values vector with the inserted value.
    n = std::unique_ptr<Node>(new Node{key, {value}, RED});
  } else if (key < n->key) {
    Insert(n->left, key, value);                     // Recurse into left subtree.
  } else if (key > n->key) {
    Insert(n->right, key, value);                    // Recurse into right subtree.
  } else {
    n->values.push_back(value);                      // Key exists, append value.
  }
  FixUp(n);                                          // Fix subtree to maintain balance.
}

// Public Print: prints the multimap in-order.
template <typename K, typename V>
void Multimap<K, V>::Print() {
  Print(root.get());                                 // Start at root.
  std::cout << std::endl;                            // End with a newline.
}

// Recursive Print: in-order traversal to print each node's key and its values.
template <typename K, typename V>
void Multimap<K, V>::Print(Node *n) {
  if (!n) return;                                    // Base case: if node is null, do nothing.
  Print(n->left.get());                              // Print left subtree.
  // Print current node: key and list of values.
  std::cout << "<" << n->key << ", [";               // Begin with key and opening bracket.
  for (size_t i = 0; i < n->values.size(); ++i) {    // Loop through values vector.
    std::cout << n->values[i];                       // Print individual value.
    if (i != n->values.size() - 1)
      std::cout << ", ";                             // Comma separator if not last.
  }
  std::cout << "]> ";                                // Close the bracket and node.
  Print(n->right.get());                             // Print right subtree.
}

#endif  // MULTIMAP_H_
