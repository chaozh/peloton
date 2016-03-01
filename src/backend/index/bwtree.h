//===----------------------------------------------------------------------===//
//
//                         PelotonDB
//
// BWTree.h
//
// Identification: src/backend/index/BWTree.h
//
// Copyright (c) 2015, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

namespace peloton {
namespace index {

// Look up the stx btree interface for background.
// peloton/third_party/stx/btree.h
template <typename KeyType, typename ValueType, typename KeyComparator, typename KeyEqualityChecker>
class BWTree {
public:
	//static const bool                   allow_duplicates = false;
    /// Typedef of our own type
    typedef BWTree<KeyType, ValueType, KeyComparator, KeyEqualityChecker> btree_self;

    /// Size type used to count keys
    typedef size_t                              size_type;

    /// The pair of KeyType and ValueType.
    typedef std::pair<KeyType, ValueType>      pair_type;

    // *** Iterators and Reverse Iterators

    class iterator;
    class const_iterator;
    class reverse_iterator;
    class const_reverse_iterator;


  	// Add your declarations here
	inline ~BWTree()
	{
	    clear();
	}

	/// Fast swapping of two identical B+ tree objects.
    void swap(btree_self& from)

    bool exists(const KeyType &key) const;
    iterator find(const KeyType &key);
    const_iterator find(const KeyType &key) const;
    size_type count(const KeyType &key) const;

    std::pair<iterator, bool> insert(const pair_type &);
    size_type erase(const KeyType &key);
private:
	// *** Node Classes for In-Memory Nodes
	struct Node
    {

    }

    struct InnerNode : public Node
    {

    }

	struct LeafNode : public Node
    {

    }


	// *** Tree Object Data Members

    /// Pointer to the B+ tree's root node, either leaf or inner node
    node*       root_;

    /// Pointer to first leaf in the double linked leaf chain
    LeafNode   *headleaf_;

    /// Pointer to last leaf in the double linked leaf chain
    LeafNode   *tailleaf_;
};

}  // End index namespace
}  // End peloton namespace
