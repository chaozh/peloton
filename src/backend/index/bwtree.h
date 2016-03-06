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
    void swap(btree_self& from);
    //op with thread info
    bool exists(const KeyType &key) const;
    iterator find(const KeyType &key);
    const_iterator find(const KeyType &key) const;
    size_type count(const KeyType &key) const;

    std::pair<iterator, bool> insert(const pair_type &);
    size_type erase(const KeyType &key);
private:
    void splitPage(const PID splitPage, const PID splitPageParent);
    // *** Node Classes for In-Memory Nodes
    struct Node
    {
        /// Level in the b-tree, if level == 0 -> leaf Node
        unsigned short  level;

        /// Number of key slotuse use, so number of valid children or data
        /// pointers
        unsigned short  slotuse;

        Node() = delete;
        ~Node() = delete;

        /// Delayed initialisation of constructed Node
        inline void initialize(const unsigned short l)
        {
            level = l;
            slotuse = 0;
        }

        /// True if this is a leaf Node
        inline bool isLeafNode() const
        {
            return (level == 0);
        }
    };

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
            Node::initialize(l);
        }

        /// True if the Node's slots are full
        inline bool isFull() const
        {
            return (Node::slotuse == innerslotmax);
        }

        /// True if few used entries, less than half full
        inline bool isFew() const
        {
            return (Node::slotuse <= mininnerslots);
        }

        /// True if Node has too few entries
        inline bool isUnderFlow() const
        {
            return (Node::slotuse < mininnerslots);
        }
    };

    struct LeafNode : public Node
    {
        /// Double linked list pointers to traverse the leaves
        PID       prevleaf;

        /// Double linked list pointers to traverse the leaves
        PID       nextleaf;

        /// Keys of children or data pointers
        KeyType        slotkey[leafslotmax];

        /// Array of data
        ValueType       slotdata[leafslotmax];

        LeafNode() = delete;
        ~LeafNode() = delete;

        /// Set variables to initial values
        inline void initialize()
        {
            Node::initialize(0);
            prevleaf = nextleaf = NULL;
        }

        /// True if the Node's slots are full
        inline bool isFull() const
        {
            return (Node::slotuse == leafslotmax);
        }

        /// True if few used entries, less than half full
        inline bool isFew() const
        {
            return (Node::slotuse <= minleafslots);
        }

        /// True if Node has too few entries
        inline bool isUnderFlow() const
        {
            return (Node::slotuse < minleafslots);
        }

        /// Set the (key,data) pair in slot. Overloaded function used by
        /// bulk_load().
        inline void setSlot(unsigned short slot, const pair_type& value)
        {
            assert(used_as_set == false);
            assert(slot < Node::slotuse);
            slotkey[slot] = value.first;
            slotdata[slot] = value.second;
        }

        /// Set the key pair in slot. Overloaded function used by
        /// bulk_load().
        inline void setSlot(unsigned short slot, const KeyType& key)
        {
            assert(used_as_set == true);
            assert(slot < Node::slotuse);
            slotkey[slot] = key;
        }
    };

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

    //make node vector for gc
    class GCList
    {
        
    };

    class ThreadInfo
    {
    
    };
   
    //epoch for thread to join
    class Epoch 
    {
        std::atomic<uint64_t> currentEpoch{0};
        //should be LTS for every thread
        GCList gclist;
        //gc
        size_type GCThreshHold;
    public:
        void enterEpoch();
        void exitEpoch();
        void markForGC();
        void showGCRadio(); 
    };

    // *** Tree Object Data Members

    /// Pointer to the B+ tree's root Node, either leaf or inner Node
    std::atomic<PID>   root_;

    /// Pointer to first leaf in the double linked leaf chain
    std::atomic<PID>   headleaf_;

    /// Pointer to last leaf in the double linked leaf chain
    std::atomic<PID>   tailleaf_;

    // Mapping Table with CAS
    std::vector<std::atomic<Node*>> mapping{};

    //all epoch manager
    Epoch epoch{64};

private:
    Node* getNodeByPID(PID pid)
    {
        return mapping[pid];
    }

};

}  // End index namespace
}  // End peloton namespace
