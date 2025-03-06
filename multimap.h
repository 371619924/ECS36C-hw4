#ifndef MULTIMAP_H_
#define MULTIMAP_H_

#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <utility>
#include <stdexcept>

template <typename K, typename V>
class Multimap {
 public:
  // Return size of tree
  unsigned int Size();
  // Return first value associated with @key
  const V& Get(const K& key);
  // Return whether @key is found in tree
  bool Contains(const K& key);
  // Return max key in tree
  const K& Max();
  // Return min key in tree
  const K& Min();
  // Insert @key and @value in tree
  void Insert(const K &key, const V &value);
  // Remove first occurrence of @key's value
  void Remove(const K &key);
  // Print tree in-order
  void Print();

 private:
  enum Color { RED, BLACK };
  struct Node{
    K key;
    std::vector<V> values;
    bool color;
    std::unique_ptr<Node> left;
    std::unique_ptr<Node> right;
  };
  std::unique_ptr<Node> root;
  unsigned int cur_size = 0;

  // Iterative helper methods
  Node* Get(Node *n, const K &key);

  // Recursive helper methods
  Node* Min(Node *n);
  void Insert(std::unique_ptr<Node> &n, const K &key, const V &value);
  void Remove(std::unique_ptr<Node> &n, const K &key);
  void Print(Node *n);

  // Helper methods for the self-balancing
  bool IsRed(Node *n);
  void FlipColors(Node *n);
  void RotateRight(std::unique_ptr<Node> &prt);
  void RotateLeft(std::unique_ptr<Node> &prt);
  void FixUp(std::unique_ptr<Node> &n);
  void MoveRedRight(std::unique_ptr<Node> &n);
  void MoveRedLeft(std::unique_ptr<Node> &n);
  void DeleteMin(std::unique_ptr<Node> &n);
};

// Returns total number of key-value pairs in the multimap
template <typename K, typename V>
unsigned int Multimap<K, V>::Size() {
  return cur_size;
}

// Iteratively searches for the node with the
// given key starting from the provided node
template <typename K, typename V>
typename Multimap<K, V>::Node* Multimap<K, V>::Get(Node *n, const K &key) {
  while (n) {
    if (key == n->key)
      return n;  // found matching key

    if (key < n->key)
      n = n->left.get();  // move to left child
    else
      n = n->right.get();  // move to right child
  }
  return nullptr;  // key not found
}

// Returns the first value associated with the given key
// Throws an error if the key is not found or
// if the key has no associated values
template <typename K, typename V>
const V& Multimap<K, V>::Get(const K &key) {
  Node *n = Get(root.get(), key);  // get node with the key
  if (!n)
    // key missing
    throw std::runtime_error("Error: cannot find key");
  if (n->values.empty())
    // key exists but has no values
    throw std::runtime_error("Error: key has no values");
  return n->values.front();  // return first value
}

// Checks if the multimap contains the given key
template <typename K, typename V>
bool Multimap<K, V>::Contains(const K &key) {
  // return true if node is found
  return Get(root.get(), key) != nullptr;
}

// Returns the maximum key in the multimap
// Throws an error if the multimap is empty
template <typename K, typename V>
const K& Multimap<K, V>::Max(void) {
  Node *n = root.get();  // start at root
  if (!n)
    // empty tree error
    throw std::runtime_error("Error: empty tree");
  // traverse to rightmost node
  while (n->right) n = n->right.get();
  return n->key;  // return maximum key
}

// Returns the minimum key in the multimap
// Throws an error if the multimap is empty
template <typename K, typename V>
const K& Multimap<K, V>::Min(void) {
  // find minimum node recursively
  Node *n = Min(root.get());
  if (!n)
    // empty tree error
    throw std::runtime_error("Error: empty tree");
  return n->key;  // return minimum key
}

// Recursively finds the minimum node starting from the given node
template <typename K, typename V>
typename Multimap<K, V>::Node* Multimap<K, V>::Min(Node *n) {
  if (!n)
    return nullptr;  // return null if node is null
  if (n->left)
    return Min(n->left.get());  // recursively go to left child
  else
    return n;  // leftmost node found
}

// Checks if the given node is red
template <typename K, typename V>
bool Multimap<K, V>::IsRed(Node *n) {
  if (!n) return false;  // return false if node is null
  return (n->color == RED);  // return true if node's color is red
}

// Flips the color of the given node and its children
template <typename K, typename V>
void Multimap<K, V>::FlipColors(Node *n) {
  n->color = !n->color;  // flip node color
  n->left->color = !n->left->color;  // flip left child's color
  n->right->color = !n->right->color;  // flip right child's color
}

// Performs a right rotation on the subtree rooted
// at the given node reference
template <typename K, typename V>
void Multimap<K, V>::RotateRight(std::unique_ptr<Node> &prt) {
  std::unique_ptr<Node> chd = std::move(prt->left);  // move left child
  // assign right child of left node as left child
  prt->left = std::move(chd->right);
  chd->color = prt->color;  // copy parent's color to child
  prt->color = RED;  // set parent color to red
  chd->right = std::move(prt);  // rotate parent's pointer
  prt = std::move(chd);  // update parent pointer
}

// Performs a left rotation on the subtree rooted
// at the given node reference
template <typename K, typename V>
void Multimap<K, V>::RotateLeft(std::unique_ptr<Node> &prt) {
  std::unique_ptr<Node> chd = std::move(prt->right);  // move right child
  // assign left child of right node as right child
  prt->right = std::move(chd->left);
  chd->color = prt->color;  // copy parent's color to child
  prt->color = RED;  // set parent color to red
  chd->left = std::move(prt);  // rotate parent's pointer
  prt = std::move(chd);  // update parent pointer
}

// Fixes up the subtree to maintain red-black tree properties
// by performing rotations and color flips
template <typename K, typename V>
void Multimap<K, V>::FixUp(std::unique_ptr<Node> &n) {
  // rotate left if right child is red and left child is not red
  if (IsRed(n->right.get()) && !IsRed(n->left.get()))
    RotateLeft(n);
  // rotate right if left child and its left child are red
  if (IsRed(n->left.get()) && IsRed(n->left->left.get()))
    RotateRight(n);
  // flip colors if both children are red
  if (IsRed(n->left.get()) && IsRed(n->right.get()))
    FlipColors(n.get());
}

// Moves a red node to the right to prepare for deletion in the right subtree.
template <typename K, typename V>
void Multimap<K, V>::MoveRedRight(std::unique_ptr<Node> &n) {
  FlipColors(n.get());  // flip colors at current node
  if (IsRed(n->left->left.get())) {
    RotateRight(n);  // rotate right if left-left grandchild is red
    FlipColors(n.get());  // flip colors after rotation
  }
}

// Moves a red node to the left to prepare for deletion in the left subtree
template <typename K, typename V>
void Multimap<K, V>::MoveRedLeft(std::unique_ptr<Node> &n) {
  FlipColors(n.get());  // flip colors at current node
  if (IsRed(n->right->left.get())) {
    RotateRight(n->right);  // rotate right on right child
    RotateLeft(n);  // rotate left on current node
    FlipColors(n.get());  // flip colors after rotations
  }
}

// Deletes the minimum node in the subtree rooted at the given node reference
template <typename K, typename V>
void Multimap<K, V>::DeleteMin(std::unique_ptr<Node> &n) {
  if (!n->left) {
    n = nullptr;  // remove node if left child is absent
    return;
  }

  if (!IsRed(n->left.get()) && !IsRed(n->left->left.get()))
    MoveRedLeft(n);  // move red left if necessary

  DeleteMin(n->left);  // recursively delete minimum in left subtree

  FixUp(n);  // fix up subtree after deletion
}

// Removes a key-value pair corresponding to the given key from the multimap
// If the key is associated with multiple values
// only the first value is removed
template <typename K, typename V>
void Multimap<K, V>::Remove(const K &key) {
  if (!Contains(key))
    return;  // do nothing if key is not found
  Remove(root, key);  // recursively remove key from tree
  cur_size--;  // decrement size counter
  if (root)
    root->color = BLACK;  // ensure root is always black
}

// Recursively removes a key-value pair from the subtree
// and adjusts the tree structure accordingly
template <typename K, typename V>
void Multimap<K, V>::Remove(std::unique_ptr<Node> &n, const K &key) {
  if (!n) return;  // base case: node is null

  if (key < n->key) {
    if (!IsRed(n->left.get()) && !IsRed(n->left->left.get()))
      MoveRedLeft(n);  // move red left if necessary
    Remove(n->left, key);  // recursively remove from left subtree
  } else {
    if (IsRed(n->left.get())
        )
      RotateRight(n);  // rotate right if left child is red

    if (key == n->key && !n->right) {
      if (n->values.size() == 1) {
        n = nullptr;  // remove node if it has only one value
      } else {
        // erase first value if multiple exist
        n->values.erase(n->values.begin());
      }
      return;
    }

    if (!IsRed(n->right.get()) && !IsRed(n->right->left.get()))
      MoveRedRight(n);  // move red right if necessary

    if (key == n->key) {
      if (n->values.size() == 1) {
        Node *n_min = Min(n->right.get());  // find min node in right subtree
        n->key = n_min->key;  // replace key with min key from right subtree
        n->values = std::move(n_min->values);  // move values from minimum node
        DeleteMin(n->right);  // delete the minimum node from right subtree
      } else {
        // erase first value if multiple exist
        n->values.erase(n->values.begin());
      }
    } else {
      Remove(n->right, key);  // recursively remove from right subtree
    }
  }

  FixUp(n);  // fix up the subtree to maintain tree properties
}

// Inserts a key-value pair into the multimap
// and ensures red-black tree properties
template <typename K, typename V>
void Multimap<K, V>::Insert(const K &key, const V &value) {
  Insert(root, key, value);  // recursively insert key-value pair
  cur_size++;  // increment size counter
  if (root)
    root->color = BLACK;  // ensure root is always black
}

// Recursively inserts a key-value pair into the subtree.
// If the key already exists, appends the value to the existing vector.
template <typename K, typename V>
void Multimap<K, V>::Insert(std::unique_ptr<Node> &n,
                            const K &key, const V &value) {
  if (!n) {
    // create new node with value vector
    n = std::unique_ptr<Node>(new Node{key, {value}, RED});
  } else if (key < n->key) {
    Insert(n->left, key, value);  // insert into left subtree
  } else if (key > n->key) {
    Insert(n->right, key, value);  // insert into right subtree
  } else {
    n->values.push_back(value);  // append value if key exists
  }

  FixUp(n);  // fix up the subtree to maintain tree properties
}

// Prints the entire multimap in-order.
template <typename K, typename V>
void Multimap<K, V>::Print() {
  Print(root.get());  // print subtree starting at root
  std::cout << std::endl;  // print newline at end
}

// Recursively prints the subtree in-order
// displaying each key and its associated values
template <typename K, typename V>
void Multimap<K, V>::Print(Node *n) {
  if (!n) return;  // base case: if node is null, return
  Print(n->left.get());  // print left subtree
  // print key and opening bracket for values
  std::cout << "<" << n->key << ", [";
  for (size_t i = 0; i < n->values.size(); ++i) {
    std::cout << n->values[i];  // print value
    if (i != n->values.size() - 1)
      std::cout << ", ";  // print comma separator in lowercase
  }
  std::cout << "]> ";  // print closing bracket and angle bracket
  Print(n->right.get());  // print right subtree
}

#endif  // MULTIMAP_H_
