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

#include <algorithm>
#include <functional>
#include <istream>
#include <ostream>
#include <memory>
#include <cstddef>
#include <atomic>
#include <vector>
#include <assert.h>

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
    using btree_self = BWTree<KeyType, ValueType, KeyComparator, KeyEqualityChecker>;

    /// Size type used to count keys
    using size_type = size_t;

    /// The pair of KeyType and ValueType.
    using pair_type = std::pair<KeyType, ValueType>;

    using PID = unsigned long;

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

        Node() = delete;
        ~Node() = delete;

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

        InnerNode() = delete;
        ~InnerNode() = delete;

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

        LeafNode() = delete;
        ~LeafNode() = delete;

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

    //delta record
    enum class DeltaTypes: std::int8_t 
    {
        deltaInsert,
        deltaDelete,
        deltaUpdate,
        deltaSplit
    };

    struct DeltaNode
    {
        DeltaTypes type;
        Node* origin;

        DeltaNode() = delete;
        ~DeltaNode() = delete;

        inline void initialize(DeltaTypes t)
        {
            type = t;
            origin = NULL;
        }
    };

    struct DeltaInsert: public DeltaNode
    {
        pair_type record;
        
        DeltaInsert() = delete;
        ~DeltaInsert() = delete;

        inline void initialize()
        {
            DeltaNode::initialize(DeltaTypes::deltaInsert);
        }
    };

    struct DeltaDelete: public DeltaNode
    {
        KeyType key;

        DeltaDelete() = delete;
        ~DeltaDelete() = delete;

        inline void initialize()
        {
            DeltaNode::initialize(DeltaTypes::deltaDelete);
        }
    };

    struct DeltaUpdate: public DeltaNode
    {
        KeyType keyLeft;
        KeyType keyRight;
        PID child;
        PID oldChild;

        DeltaUpdate() = delete;
        ~DeltaUpdate() = delete;

        inline void initialize()
        {
            DeltaNode::initialize(DeltaTypes::deltaUpdate);
        }
    };

    struct DeltaSplit: public DeltaNode
    {
        KeyType key;
        PID sideLink;
        size_type removedNodes;

        DeltaSplit() = delete;
        ~DeltaSplit() = delete;

        inline void initialize()
        {
            DeltaNode::initialize(DeltaTypes::deltaSplit);
        }
    };

    //epoch for thread to join
    class Epoch 
    {
        std::atomic<uint64_t> currentEpoch{0};
        //delete

        //gc
        size_type GCThreshHold;
    public:
        void enterEpoch();
        void exitEpoch();
        void markForGC();
        void showGCRadio(); 
    }

    // *** Tree Object Data Members

    /// Pointer to the B+ tree's root node, either leaf or inner node
    std::atomic<PID>   root_;

    /// Pointer to first leaf in the double linked leaf chain
    std::atomic<PID>   headleaf_;

    /// Pointer to last leaf in the double linked leaf chain
    std::atomic<PID>   tailleaf_;

    // Mapping Table with CAS
    std::vector<std::atomic<Node*>> mapping{};

    //epoch manager

};

}  // End index namespace
}  // End peloton namespace
