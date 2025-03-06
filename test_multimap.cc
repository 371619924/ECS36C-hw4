#include <gtest/gtest.h>
#include <algorithm>
#include <vector>
#include <stdexcept>

#include "multimap.h"  // include multimap header

// Test: one key with one value
TEST(Multimap, OneKeyOneValue) {
  // create multimap object
  Multimap<int, int> mmap;
  // insert key 2 with value 20
  mmap.Insert(2, 20);
  // check that size is 1
  EXPECT_EQ(mmap.Size(), 1);
  // verify that key 2 exists
  EXPECT_TRUE(mmap.Contains(2));
  // verify that value for key 2 is 20
  EXPECT_EQ(mmap.Get(2), 20);
}

// Test: one key with multiple values
TEST(Multimap, OneKeyMultipleValues) {
  // create multimap object
  Multimap<int, int> mmap;
  // insert key 5 with value 10
  mmap.Insert(5, 10);
  // insert key 5 with value 20
  mmap.Insert(5, 20);
  // insert key 5 with value 30
  mmap.Insert(5, 30);
  // check that size is 3 (3 values for one key)
  EXPECT_EQ(mmap.Size(), 3);
  // verify that key 5 exists
  EXPECT_TRUE(mmap.Contains(5));
  // check that the first inserted value is returned
  EXPECT_EQ(mmap.Get(5), 10);
}

// Test: multiple keys insertion and retrieval
TEST(Multimap, MultipleKeys) {
  // create multimap object
  Multimap<int, int> mmap;
  // list of keys to insert
  std::vector<int> keys = {3, 1, 5, 2, 4};
  // insert each key with value equal to key*10
  for (int k : keys) {
    mmap.Insert(k, k * 10);
  }
  // check that size equals the number of keys
  EXPECT_EQ(mmap.Size(), keys.size());
  // verify that each key exists and has value key*10
  for (int k : keys) {
    EXPECT_TRUE(mmap.Contains(k));
    EXPECT_EQ(mmap.Get(k), k * 10);
  }
}

// Test: removal of a key with a single value
TEST(Multimap, RemoveSingleValue) {
  // create multimap object
  Multimap<int, int> mmap;
  // insert key 10 with value 100
  mmap.Insert(10, 100);
  // remove key 10
  mmap.Remove(10);
  // check that the multimap is empty
  EXPECT_EQ(mmap.Size(), 0);
  // verify that key 10 no longer exists
  EXPECT_FALSE(mmap.Contains(10));
}

// Test: removal of a key with multiple values
TEST(Multimap, RemoveMultipleValues) {
  // create multimap object
  Multimap<int, int> mmap;
  // insert key 7 with value 70
  mmap.Insert(7, 70);
  // insert key 7 with value 71
  mmap.Insert(7, 71);
  // insert key 7 with value 72
  mmap.Insert(7, 72);
  // remove the first occurrence for key 7
  mmap.Remove(7);
  // check that size is reduced to 2
  EXPECT_EQ(mmap.Size(), 2);
  // verify that the new first value is 71
  EXPECT_EQ(mmap.Get(7), 71);
  // remove the next occurrence for key 7
  mmap.Remove(7);
  // check that size is reduced to 1
  EXPECT_EQ(mmap.Size(), 1);
  // verify that the new first value is 72
  EXPECT_EQ(mmap.Get(7), 72);
  // remove the final occurrence for key 7
  mmap.Remove(7);
  // check that the multimap is empty
  EXPECT_EQ(mmap.Size(), 0);
  // verify that key 7 no longer exists
  EXPECT_FALSE(mmap.Contains(7));
}

// Test: size update after multiple operations on the same key
TEST(Multimap, SizeAfterOperations) {
  // create multimap object with string keys
  Multimap<std::string, int> mmap;
  // check that initial size is 0
  EXPECT_EQ(mmap.Size(), 0);
  // insert key "test" with value 1
  mmap.Insert("test", 1);
  // check that size is 1
  EXPECT_EQ(mmap.Size(), 1);
  // insert key "test" with value 2
  mmap.Insert("test", 2);
  // check that size is 2
  EXPECT_EQ(mmap.Size(), 2);
  // remove the first occurrence for key "test"
  mmap.Remove("test");
  // check that size is 1
  EXPECT_EQ(mmap.Size(), 1);
  // remove the next occurrence for key "test"
  mmap.Remove("test");
  // check that the multimap is empty
  EXPECT_EQ(mmap.Size(), 0);
}

// Test: retrieval after consecutive removals
TEST(Multimap, GetAfterRemoves) {
  // create multimap object with int keys and string values
  Multimap<int, std::string> mmap;
  // insert key 1 with value "first"
  mmap.Insert(1, "first");
  // insert key 1 with value "second"
  mmap.Insert(1, "second");
  // insert key 1 with value "third"
  mmap.Insert(1, "third");
  // remove the first occurrence ("first")
  mmap.Remove(1);
  // check that the value is now "second"
  EXPECT_EQ(mmap.Get(1), "second");
  // remove the next occurrence ("second")
  mmap.Remove(1);
  // check that the value is now "third"
  EXPECT_EQ(mmap.Get(1), "third");
}

// Test: minimum and maximum key retrieval
TEST(Multimap, MinMaxKeys) {
  // create multimap object
  Multimap<int, int> mmap;
  // insert key 5 with value 50
  mmap.Insert(5, 50);
  // insert key 2 with value 20
  mmap.Insert(2, 20);
  // insert key 8 with value 80
  mmap.Insert(8, 80);
  // insert key 2 with an additional value 21
  mmap.Insert(2, 21);
  // insert key 8 with an additional value 81
  mmap.Insert(8, 81);
  // check that the minimum key is 2
  EXPECT_EQ(mmap.Min(), 2);
  // check that the maximum key is 8
  EXPECT_EQ(mmap.Max(), 8);
}

// Test: inserting a key after all its values have been removed
TEST(Multimap, InsertAfterRemoveAll) {
  // create multimap object with int keys and char values
  Multimap<int, char> mmap;
  // insert key 3 with value 'a'
  mmap.Insert(3, 'a');
  // remove key 3
  mmap.Remove(3);
  // verify that key 3 no longer exists
  EXPECT_FALSE(mmap.Contains(3));
  // insert key 3 with new value 'b'
  mmap.Insert(3, 'b');
  // check that key 3 exists
  EXPECT_TRUE(mmap.Contains(3));
  // verify that value for key 3 is 'b'
  EXPECT_EQ(mmap.Get(3), 'b');
}

// Test: behavior when querying a key that is not present
TEST(Multimap, KeyNotPresent) {
  // create multimap object
  Multimap<int, int> mmap;
  // insert key 4 with value 40
  mmap.Insert(4, 40);
  // check that key 2 does not exist
  EXPECT_FALSE(mmap.Contains(2));
  // expect an error when getting key 2
  EXPECT_THROW(mmap.Get(2), std::runtime_error);
}

// Test: removal operation on a key that does not exist
TEST(Multimap, RemoveNonExistentKey) {
  // create multimap object
  Multimap<int, int> mmap;
  // insert key 1 with value 10
  mmap.Insert(1, 10);
  // attempt to remove non-existent key 2
  mmap.Remove(2);
  // check that size remains unchanged
  EXPECT_EQ(mmap.Size(), 1);
}

// Test: preservation of value insertion order for a key
TEST(Multimap, ValuesOrderPreserved) {
  // create multimap object
  Multimap<int, int> mmap;
  // insert key 9 with value 90
  mmap.Insert(9, 90);
  // insert key 9 with value 91
  mmap.Insert(9, 91);
  // insert key 9 with value 92
  mmap.Insert(9, 92);
  // expected order of values
  std::vector<int> expected = {90, 91, 92};
  // remove the first occurrence (value 90)
  mmap.Remove(9);
  // remove first element from expected vector
  expected.erase(expected.begin());
  // check that the first value matches the expected value
  EXPECT_EQ(mmap.Get(9), expected.front());
}

// Test: operations on an empty multimap
TEST(Multimap, EmptyMapOperations) {
  // create an empty multimap object
  Multimap<int, int> mmap;
  // expect an error when retrieving the min key
  EXPECT_THROW(mmap.Min(), std::runtime_error);
  // expect an error when retrieving the max key
  EXPECT_THROW(mmap.Max(), std::runtime_error);
  // attempt to remove a non-existent key; should not crash
  mmap.Remove(42);
}

// Test: multiple keys with multiple values and subsequent removals
TEST(Multimap, MultipleKeysMultipleValues) {
  // create multimap object
  Multimap<int, int> mmap;
  // insert key 1 with value 10
  mmap.Insert(1, 10);
  // insert key 2 with value 20
  mmap.Insert(2, 20);
  // insert key 1 with an additional value 11
  mmap.Insert(1, 11);
  // insert key 3 with value 30
  mmap.Insert(3, 30);
  // insert key 2 with an additional value 21
  mmap.Insert(2, 21);
  // insert key 3 with an additional value 31
  mmap.Insert(3, 31);
  // check that total size is 6
  EXPECT_EQ(mmap.Size(), 6);
  // remove the first occurrence for key 1
  mmap.Remove(1);
  // remove the first occurrence for key 2
  mmap.Remove(2);
  // remove the first occurrence for key 3
  mmap.Remove(3);
  // check that remaining size is 3
  EXPECT_EQ(mmap.Size(), 3);
  // verify that the remaining value for key 1 is 11
  EXPECT_EQ(mmap.Get(1), 11);
  // verify that the remaining value for key 2 is 21
  EXPECT_EQ(mmap.Get(2), 21);
  // verify that the remaining value for key 3 is 31
  EXPECT_EQ(mmap.Get(3), 31);
}

// Test: complex sequence of insertions and removals across multiple keys
TEST(Multimap, ComplexInsertRemoveSequence) {
  // create multimap object
  Multimap<int, int> mmap;
  // insert two values for keys 0 to 4
  for (int i = 0; i < 5; ++i) {
    // insert key i with value i
    mmap.Insert(i, i);
    // insert key i with value i*10
    mmap.Insert(i, i * 10);
  }
  // check that total size is 10
  EXPECT_EQ(mmap.Size(), 10);
  // remove both occurrences for each key from 0 to 4
  for (int i = 0; i < 5; ++i) {
    // remove first occurrence for key i
    mmap.Remove(i);
    // remove second occurrence for key i
    mmap.Remove(i);
    // verify that key i is completely removed
    EXPECT_FALSE(mmap.Contains(i));
  }
  // check that the multimap is empty
  EXPECT_EQ(mmap.Size(), 0);
}

// Main function to run all tests
int main(int argc, char **argv) {
  // initialize googletest framework
  ::testing::InitGoogleTest(&argc, argv);
  // run all tests and return the result
  return RUN_ALL_TESTS();
}
