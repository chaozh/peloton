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

#define BTREE_NODE_SIZE  256

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

    typedef unsigned long                       PID;

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
        /// Level in the b-tree, if level == 0 -> leaf node
        unsigned short  level;

        /// Number of key slotuse use, so number of valid children or data
        /// pointers
        unsigned short  slotuse;

        /// Delayed initialisation of constructed node
        inline void initialize(const unsigned short l)
        {
            level = l;
            slotuse = 0;
        }

        /// True if this is a leaf node
        inline bool isleafnode() const
        {
            return (level == 0);
        }
    }

    struct InnerNode : public Node
    {
        /// Keys of children or data pointers
        KeyType        slotkey[innerslotmax];

        /// Pointers to children
        PID           childid[innerslotmax+1];

        /// Set variables to initial values
        inline void initialize(const unsigned short l)
        {
            node::initialize(l);
        }

        /// True if the node's slots are full
        inline bool isfull() const
        {
            return (node::slotuse == innerslotmax);
        }

        /// True if few used entries, less than half full
        inline bool isfew() const
        {
            return (node::slotuse <= mininnerslots);
        }

        /// True if node has too few entries
        inline bool isunderflow() const
        {
            return (node::slotuse < mininnerslots);
        }
    }

    struct LeafNode : public Node
    {
        /// Double linked list pointers to traverse the leaves
        PID       prevleaf;

        /// Double linked list pointers to traverse the leaves
        PID       nextleaf;

        /// Keys of children or data pointers
        KeyType        slotkey[leafslotmax];

        /// Array of data
        DataType       slotdata[leafslotmax];

        /// Set variables to initial values
        inline void initialize()
        {
            node::initialize(0);
            prevleaf = nextleaf = NULL;
        }

        /// True if the node's slots are full
        inline bool isfull() const
        {
            return (node::slotuse == leafslotmax);
        }

        /// True if few used entries, less than half full
        inline bool isfew() const
        {
            return (node::slotuse <= minleafslots);
        }

        /// True if node has too few entries
        inline bool isunderflow() const
        {
            return (node::slotuse < minleafslots);
        }

        /// Set the (key,data) pair in slot. Overloaded function used by
        /// bulk_load().
        inline void set_slot(unsigned short slot, const pair_type& value)
        {
            assert(used_as_set == false);
            assert(slot < node::slotuse);
            slotkey[slot] = value.first;
            slotdata[slot] = value.second;
        }

        /// Set the key pair in slot. Overloaded function used by
        /// bulk_load().
        inline void set_slot(unsigned short slot, const key_type& key)
        {
            assert(used_as_set == true);
            assert(slot < node::slotuse);
            slotkey[slot] = key;
        }
    }


    // *** Tree Object Data Members

    /// Pointer to the B+ tree's root node, either leaf or inner node
    PID   root_;

    /// Pointer to first leaf in the double linked leaf chain
    PID   headleaf_;

    /// Pointer to last leaf in the double linked leaf chain
    PID   tailleaf_;
};

}  // End index namespace
}  // End peloton namespace
