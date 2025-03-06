#ifndef MULTIMAP_H_
#define MULTIMAP_H_

#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <utility>

// Multimap 实现基于左倾红黑树
template <typename K, typename V>
class Multimap {
 public:
  // 返回树中节点数（键的个数，不计重复值）
  unsigned int Size();

  // 返回与键关联的第一个值
  const V& Get(const K& key);

  // 判断树中是否存在给定键
  bool Contains(const K& key);

  // 返回树中最大键
  const K& Max();

  // 返回树中最小键
  const K& Min();

  // 插入键-值对；如果键已存在，则追加值
  void Insert(const K &key, const V &value);

  // 删除键：如果键对应多个值，仅删除第一个值；否则删除整个节点
  void Remove(const K &key);

  // 按中序遍历打印树，辅助调试
  void Print();

 private:
  enum Color { RED, BLACK };

  struct Node {
    K key;
    std::vector<V> values;   // 存放多个值
    bool color;
    std::unique_ptr<Node> left;
    std::unique_ptr<Node> right;
  };

  std::unique_ptr<Node> root;
  unsigned int cur_size = 0;  // 节点总数

  // 迭代辅助：查找键所在的节点
  Node* Get(Node *n, const K &key);

  // 递归辅助：查找子树中的最小节点
  Node* Min(Node *n);

  // 递归辅助插入（增加一个额外参数 isNew 用于判断是否新建节点）
  void Insert(std::unique_ptr<Node> &n, const K &key, const V &value, bool &isNew);

  // 递归辅助删除，返回是否删除了一个节点（true）还是仅删除了一个值（false）
  bool Remove(std::unique_ptr<Node> &n, const K &key);

  // 递归辅助：中序打印
  void Print(Node *n);

  // 红黑树辅助函数
  bool IsRed(Node *n);
  void FlipColors(Node *n);
  void RotateRight(std::unique_ptr<Node> &prt);
  void RotateLeft(std::unique_ptr<Node> &prt);
  void FixUp(std::unique_ptr<Node> &n);
  void MoveRedRight(std::unique_ptr<Node> &n);
  void MoveRedLeft(std::unique_ptr<Node> &n);
  void DeleteMin(std::unique_ptr<Node> &n);
};

//////////////////////
// 公共接口实现 //
//////////////////////

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
  if (!n)
    throw std::runtime_error("Error: cannot find key");
  return n->values[0];
}

template <typename K, typename V>
bool Multimap<K, V>::Contains(const K &key) {
  return Get(root.get(), key) != nullptr;
}

template <typename K, typename V>
const K& Multimap<K, V>::Max() {
  Node *n = root.get();
  while (n->right)
    n = n->right.get();
  return n->key;
}

template <typename K, typename V>
const K& Multimap<K, V>::Min() {
  return Min(root.get())->key;
}

template <typename K, typename V>
typename Multimap<K, V>::Node* Multimap<K, V>::Min(Node *n) {
  if (n->left)
    return Min(n->left.get());
  else
    return n;
}

////////////////////////
// 红黑树辅助函数实现 //
////////////////////////

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

////////////////////////////
// 插入与删除实现（改造部分） //
////////////////////////////

template <typename K, typename V>
void Multimap<K, V>::Insert(std::unique_ptr<Node> &n, const K &key, const V &value, bool &isNew) {
  if (!n) {
    n = std::unique_ptr<Node>(new Node{key, std::vector<V>{value}, RED});
    isNew = true;
  } else if (key < n->key) {
    Insert(n->left, key, value, isNew);
  } else if (key > n->key) {
    Insert(n->right, key, value, isNew);
  } else {
    // 键已存在：追加值，不增加节点计数
    n->values.push_back(value);
    isNew = false;
  }
  FixUp(n);
}

template <typename K, typename V>
void Multimap<K, V>::Insert(const K &key, const V &value) {
  bool isNew = false;
  Insert(root, key, value, isNew);
  if (isNew)
    cur_size++;
  if (root)
    root->color = BLACK;
}

template <typename K, typename V>
bool Multimap<K, V>::Remove(std::unique_ptr<Node> &n, const K &key) {
  if (!n)
    return false;
  bool nodeRemoved = false;
  if (key < n->key) {
    if (n->left && !IsRed(n->left.get()) && !(n->left->left && IsRed(n->left->left.get())))
      MoveRedLeft(n);
    nodeRemoved = Remove(n->left, key);
  } else {
    if (IsRed(n->left.get()))
      RotateRight(n);
    if (key == n->key) {
      // 如果该键对应多个值，只删除第一个
      if (n->values.size() > 1) {
        n->values.erase(n->values.begin());
        return false;
      }
      if (!n->right) {
        n = nullptr;
        return true;
      }
      if (n->right && !IsRed(n->right.get()) && !(n->right->left && IsRed(n->right->left.get())))
        MoveRedRight(n);
      if (key == n->key) {
        Node *n_min = Min(n->right.get());
        n->key = n_min->key;
        n->values = n_min->values;
        DeleteMin(n->right);
        nodeRemoved = true;
      } else {
        nodeRemoved = Remove(n->right, key);
      }
    } else {
      nodeRemoved = Remove(n->right, key);
    }
  }
  FixUp(n);
  return nodeRemoved;
}

template <typename K, typename V>
void Multimap<K, V>::Remove(const K &key) {
  if (!Contains(key))
    return;
  bool removed = Remove(root, key);
  if (removed)
    cur_size--;
  if (root)
    root->color = BLACK;
}

///////////////////////
// 打印辅助函数实现 //
///////////////////////

template <typename K, typename V>
void Multimap<K, V>::Print() {
  Print(root.get());
  std::cout << std::endl;
}

template <typename K, typename V>
void Multimap<K, V>::Print(Node *n) {
  if (!n)
    return;
  Print(n->left.get());
  std::cout << "<" << n->key << ",[";
  for (size_t i = 0; i < n->values.size(); i++) {
    std::cout << n->values[i];
    if (i != n->values.size() - 1)
      std::cout << ",";
  }
  std::cout << "]> ";
  Print(n->right.get());
}

#endif  // MULTIMAP_H_
