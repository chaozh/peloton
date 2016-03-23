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

    static const unsigned short SLOT_INNER_MAX = BTREE_NODE_SIZE / (sizeof(KeyType) + sizeof(PID));
    static const unsigned short SLOT_LEAF_MAX = BTREE_NODE_SIZE / sizeof(ValueType);
    static const unsigned short SLOT_INNER_MIN = SLOT_INNER_MAX / 2;
    static const unsigned short SLOT_LEAF_MIN = SLOT_LEAF_MAX / 2;

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

    std::pair<iterator, bool> insert(const pair_type &record);
    size_type erase(const KeyType &key);
private:
    // *** Node Classes for In-Memory Nodes
    enum class NodeTypes: std::int8_t 
    {
        leafNode,
        innerNode,
        deltaInsert,
        deltaDelete,
        deltaUpdate,
        deltaSplit
    };

    struct Node
    {
        NodeTypes type;

         Node() = delete;
        ~Node() = delete;

        /// Delayed initialisation of constructed Node
        inline void initialize(NodeTypes t)
        {
            type = t;
        }
    };

    struct BTNode: Node
    {
        /// Level in the b-tree, if level == 0 -> leaf Node
        unsigned short  level;

        /// Number of key slotuse use, so number of valid children or data
        /// pointers
        unsigned short  slotuse;

        BTNode() = delete;
        ~BTNode() = delete;

        /// Delayed initialisation of constructed Node
        inline void initialize(NodeTypes t, const unsigned short l)
        {
            Node::initialize(t);
            level = l;
            slotuse = 0;
        }

        /// True if this is a leaf Node
        inline bool isLeafNode() const
        {
            return (level == 0);
        }
    };

    struct InnerNode : public BTNode
    {
        /// Keys of children or data pointers
        KeyType        slotkey[SLOT_INNER_MAX];

        /// Pointers to children
        PID           childid[SLOT_INNER_MAX+1];

        InnerNode() = delete;
        ~InnerNode() = delete;

        /// Set variables to initial values
        inline void initialize(const unsigned short l)
        {
            BTNode::initialize(NodeTypes::innerNode, l);
        }

        /// True if the Node's slots are full
        inline bool isFull() const
        {
            return (BTNode::slotuse == SLOT_INNER_MAX);
        }

        /// True if few used entries, less than half full
        inline bool isFew() const
        {
            return (BTNode::slotuse <= SLOT_INNER_MIN);
        }

        /// True if BTNode has too few entries
        inline bool isUnderFlow() const
        {
            return (BTNode::slotuse < SLOT_INNER_MIN);
        }
    };

    struct LeafNode : public BTNode
    {
        /// Double linked list pointers to traverse the leaves
        PID       prevleaf;

        /// Double linked list pointers to traverse the leaves
        PID       nextleaf;

        /// Keys of children or data pointers
        KeyType        slotkey[SLOT_LEAF_MAX];

        /// Array of data
        ValueType       slotdata[SLOT_LEAF_MAX];

        LeafNode() = delete;
        ~LeafNode() = delete;

        /// Set variables to initial values
        inline void initialize()
        {
            BTNode::initialize(NodeTypes::LeafNode, 0);
            prevleaf = nextleaf = NULL;
        }

        /// True if the BTNode's slots are full
        inline bool isFull() const
        {
            return (BTNode::slotuse == SLOT_LEAF_MAX);
        }

        /// True if few used entries, less than half full
        inline bool isFew() const
        {
            return (BTNode::slotuse <= SLOT_LEAF_MIN);
        }

        /// True if BTNode has too few entries
        inline bool isUnderFlow() const
        {
            return (BTNode::slotuse < SLOT_LEAF_MIN);
        }

        /// Set the (key,data) pair in slot. Overloaded function used by
        /// bulk_load().
        inline void setSlot(unsigned short slot, const pair_type& value)
        {
            assert(used_as_set == false);
            assert(slot < BTNode::slotuse);
            slotkey[slot] = value.first;
            slotdata[slot] = value.second;
        }

        /// Set the key pair in slot. Overloaded function used by
        /// bulk_load().
        inline void setSlot(unsigned short slot, const KeyType& key)
        {
            assert(used_as_set == true);
            assert(slot < BTNode::slotuse);
            slotkey[slot] = key;
        }
    };
    // delta record

    struct DeltaNode: public Node
    {
        Node* origin;

        DeltaNode() = delete;
        ~DeltaNode() = delete;

        inline void initialize(NodeTypes t)
        {
            Node::initialize(t);
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
            DeltaNode::initialize(NodeTypes::deltaInsert);
        }
    };

    struct DeltaDelete: public DeltaNode
    {
        KeyType key;

        DeltaDelete() = delete;
        ~DeltaDelete() = delete;

        inline void initialize()
        {
            DeltaNode::initialize(NodeTypes::deltaDelete);
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
            DeltaNode::initialize(NodeTypes::deltaUpdate);
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
            DeltaNode::initialize(NodeTypes::deltaSplit);
        }
    };

    class DelNode
    {
        Node* node;
        uint64_t epoch;
        DelNode(Node* n, uint64_t e): node(n), epoch(e){}
    };

    //make node vector for gc
    class GCList
    {
    private:
        std::atomic<uint64_t> localEpoch{0};
        std::vector<DelNode> list;
    public:
        size_type thresholdCounter{1};
        uint64_t getLocalEpoch() {
            return localEpoch.load();
        }

        void setLocalEpoch(uint64_t epoch){
            localEpoch.store(epoch);
        }

        void add(Node* n) {
            DelNode delNode(n, localEpoch.load());
            list.add(delNode);
            thresholdCounter++;
        }
        void remove(vector<DelNode>::iterator &n) {
            list.erase(it);
        }
    };
   
    //epoch for thread to join & used in consolidation
    class Epoch 
    {
    private:
        std::atomic<uint64_t> currentEpoch{0};
        //should be LTS for every thread
        thread_local GCList gclists;
        //gc
        size_type GCThreshHold;
    public:
        Epoch(size_type startGCThreshhold) : GCThreshHold(startGCThreshhold) { }
        ~Epoch();
        void enterEpoch() {
            //update epoch version
            unsigned long curEpoch = currentEpoch.load();
            //fetch thread local gclist
            auto gclist = gclists.local(); 
            if(curEpoch != gclist.getLocalEpoch()){
                gclist.setLocalEpoch(curEpoch);
            }
        }
        void exitEpoch() {
            auto gclist = gclists.local();
            //if(gclist.thresholdCounter & (64 - 1)) == 0)
            currentEpoch.fetch_add(1);
            //trigger gc and release all nodes under oldest epoch
            if (gclist.thresholdCounter > startGCThreshhold) {
                for(auto it=gclist.begin(); it != gclist.end(); ++it)
                    //if epoch is oldest
                    gclist.remove(it);

                gclist.thresholdCounter = 1;
            }
        }
        void markForGC(Node* node) {
            auto gclist = gclists.local();
            gclist.add(node);
        }
        void showGCRadio() {
            //output some info about del
        }
    };

    // 
    class EpochGuard
    {
    private:
        Epoch &epoch;
    public:
        EpochGuard(Epoch &epoch): epoch(epoch) {
            epoch.enterEpoch();
        }

        ~EpochGuard() {
            epoch.exitEpoch();
        }
    };
private:
    // *** Tree Object Data Members

    /// Pointer to the B+ tree's root Node, either leaf or inner Node
    std::atomic<PID>   root_;

    /// Pointer to first leaf in the double linked leaf chain
    std::atomic<PID>   headleaf_;

    /// Pointer to last leaf in the double linked leaf chain
    std::atomic<PID>   tailleaf_;

    // Mapping Table with CAS
    std::vector<std::atomic<Node*>> mapping{};

    //all epoch manage
    Epoch epoch{64};

    std::atomic<Node*> getNodeByPID(PID pid)
    {
        return mapping[pid];
    }
    Node* findPage(const KeyType &key);
public:
    void splitPage(const PID splitPage, const PID splitPageParent);
    void consolidateLeafPage(const PID page, Node* startNode);
    void consolidateInnerPage(const PID page, Node* startNode);

};

}  // End index namespace
}  // End peloton namespace
