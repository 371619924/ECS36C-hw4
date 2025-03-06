#ifndef MULTIMAP_H_
#define MULTIMAP_H_

#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <stdexcept>

template <typename K, typename V>
class Multimap {
 public:
  // 返回树中节点的个数
  unsigned int Size();
  // 返回与 key 关联的第一个值
  const V& Get(const K& key);
  // 返回树中是否存在 key
  bool Contains(const K& key);
  // 返回树中最大的 key
  const K& Max();
  // 返回树中最小的 key
  const K& Min();
  // 插入 key-value 对；若 key 已存在，则在对应的值列表末尾追加 value
  void Insert(const K &key, const V &value);
  // 删除 key；如果该 key 对应多个值，则只删除第一个值，否则删除整个节点
  void Remove(const K &key);
  // 中序打印整棵树（调试时有帮助）
  void Print();

 private:
  enum Color { RED, BLACK };
  struct Node {
    K key;
    std::vector<V> values;  // 改为保存多个值
    bool color;
    std::unique_ptr<Node> left;
    std::unique_ptr<Node> right;
  };
  std::unique_ptr<Node> root;
  unsigned int cur_size = 0;

  // 迭代查找
  Node* Get(Node *n, const K &key);

  // 递归辅助方法
  Node* Min(Node *n);
  void Insert(std::unique_ptr<Node> &n, const K &key, const V &value);
  void Remove(std::unique_ptr<Node> &n, const K &key);
  void Print(Node *n);

  // 自平衡辅助方法
  bool IsRed(Node *n);
  void FlipColors(Node *n);
  void RotateRight(std::unique_ptr<Node> &prt);
  void RotateLeft(std::unique_ptr<Node> &prt);
  void FixUp(std::unique_ptr<Node> &n);
  void MoveRedRight(std::unique_ptr<Node> &n);
  void MoveRedLeft(std::unique_ptr<Node> &n);
  void DeleteMin(std::unique_ptr<Node> &n);
};

template <typename K, typename V>
unsigned int Multimap<K, V>::Size() {
  return cur_size;
}

template <typename K, typename V>
typename Multimap<K, V>::Node* Multimap<K, V>::Get(Node *n, const K &key) {
  while (n) {
    if (key == n->key)
      return n;
    if (key < n->key)
      n = n->left.get();
    else
      n = n->right.get();
  }
  return nullptr;
}

template <typename K, typename V>
const V& Multimap<K, V>::Get(const K &key) {
  Node *n = Get(root.get(), key);
  if (!n || n->values.empty())
    throw std::runtime_error("Error: cannot find key");
  return n->values.front();
}

template <typename K, typename V>
bool Multimap<K, V>::Contains(const K &key) {
  return Get(root.get(), key) != nullptr;
}

template <typename K, typename V>
const K& Multimap<K, V>::Max(void) {
  Node *n = root.get();
  while (n->right)
    n = n->right.get();
  return n->key;
}

template <typename K, typename V>
const K& Multimap<K, V>::Min(void) {
  return Min(root.get())->key;
}

template <typename K, typename V>
typename Multimap<K, V>::Node* Multimap<K, V>::Min(Node *n) {
  if (n->left)
    return Min(n->left.get());
  else
    return n;
}

template <typename K, typename V>
bool Multimap<K, V>::IsRed(Node *n) {
  if (!n) return false;
  return (n->color == RED);
}

template <typename K, typename V>
void Multimap<K, V>::FlipColors(Node *n) {
  n->color = !n->color;
  n->left->color = !n->left->color;
  n->right->color = !n->right->color;
}

template <typename K, typename V>
void Multimap<K, V>::RotateRight(std::unique_ptr<Node> &prt) {
  std::unique_ptr<Node> chd = std::move(prt->left);
  prt->left = std::move(chd->right);
  chd->color = prt->color;
  prt->color = RED;
  chd->right = std::move(prt);
  prt = std::move(chd);
}

template <typename K, typename V>
void Multimap<K, V>::RotateLeft(std::unique_ptr<Node> &prt) {
  std::unique_ptr<Node> chd = std::move(prt->right);
  prt->right = std::move(chd->left);
  chd->color = prt->color;
  prt->color = RED;
  chd->left = std::move(prt);
  prt = std::move(chd);
}

template <typename K, typename V>
void Multimap<K, V>::FixUp(std::unique_ptr<Node> &n) {
  if (IsRed(n->right.get()) && !IsRed(n->left.get()))
    RotateLeft(n);
  if (IsRed(n->left.get()) && IsRed(n->left->left.get()))
    RotateRight(n);
  if (IsRed(n->left.get()) && IsRed(n->right.get()))
    FlipColors(n.get());
}

template <typename K, typename V>
void Multimap<K, V>::MoveRedRight(std::unique_ptr<Node> &n) {
  FlipColors(n.get());
  if (IsRed(n->left->left.get())) {
    RotateRight(n);
    FlipColors(n.get());
  }
}

template <typename K, typename V>
void Multimap<K, V>::MoveRedLeft(std::unique_ptr<Node> &n) {
  FlipColors(n.get());
  if (IsRed(n->right->left.get())) {
    RotateRight(n->right);
    RotateLeft(n);
    FlipColors(n.get());
  }
}

template <typename K, typename V>
void Multimap<K, V>::DeleteMin(std::unique_ptr<Node> &n) {
  if (!n->left) {
    n = nullptr;
    return;
  }
  if (!IsRed(n->left.get()) && !IsRed(n->left->left.get()))
    MoveRedLeft(n);
  DeleteMin(n->left);
  FixUp(n);
}

template <typename K, typename V>
void Multimap<K, V>::Remove(const K &key) {
  if (!Contains(key))
    return;
  Remove(root, key);
  cur_size--;
  if (root)
    root->color = BLACK;
}

template <typename K, typename V>
void Multimap<K, V>::Remove(std::unique_ptr<Node> &n, const K &key) {
  if (!n) return;
  if (key < n->key) {
    if (n->left && !IsRed(n->left.get()) && !IsRed(n->left->left.get()))
      MoveRedLeft(n);
    Remove(n->left, key);
  } else {
    if (n->left && IsRed(n->left.get()))
      RotateRight(n);
    if (key == n->key) {
      // 如果存在多个值，则只删除第一个，否则删除整个节点
      if (n->values.size() > 1) {
        n->values.erase(n->values.begin());
        return;
      }
      if (!n->right) {
        n = nullptr;
        return;
      }
      Node *n_min = Min(n->right.get());
      n->key = n_min->key;
      n->values = n_min->values;
      DeleteMin(n->right);
    } else {
      if (n->right && !IsRed(n->right.get()) && !IsRed(n->right->left.get()))
        MoveRedRight(n);
      Remove(n->right, key);
    }
  }
  FixUp(n);
}

template <typename K, typename V>
void Multimap<K, V>::Insert(const K &key, const V &value) {
  Insert(root, key, value);
  cur_size++;
  root->color = BLACK;
}

template <typename K, typename V>
void Multimap<K, V>::Insert(std::unique_ptr<Node> &n, const K &key, const V &value) {
  if (!n)
    n = std::unique_ptr<Node>(new Node{key, std::vector<V>{value}, RED});
  else if (key < n->key)
    Insert(n->left, key, value);
  else if (key > n->key)
    Insert(n->right, key, value);
  else
    // key 已存在则追加 value
    n->values.push_back(value);
  FixUp(n);
}

template <typename K, typename V>
void Multimap<K, V>::Print() {
  Print(root.get());
  std::cout << std::endl;
}

template <typename K, typename V>
void Multimap<K, V>::Print(Node *n) {
  if (!n) return;
  Print(n->left.get());
  std::cout << "<" << n->key << ",[";
  for (size_t i = 0; i < n->values.size(); ++i) {
    std::cout << n->values[i];
    if (i < n->values.size() - 1) std::cout << ",";
  }
  std::cout << "]> ";
  Print(n->right.get());
}

#endif  // MULTIMAP_H_
