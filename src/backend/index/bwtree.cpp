//===----------------------------------------------------------------------===//
//
//                         PelotonDB
//
// bwtree.cpp
//
// Identification: src/backend/index/bwtree.cpp
//
// Copyright (c) 2015, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "backend/index/bwtree.h"

namespace peloton {
namespace index {

    // Add your function definitions here
    template<typename KeyType, typename ValueType>
    bool BWTree::exists(const KeyType &key) const {
        if(find(key) != end()){
            return true;
        }else{
            return false;
        }
    }

    template<typename KeyType, typename ValueType>
    BWTree::iterator BWTree::find(const KeyType &key) {
        BWTree::EpochGuard(epoch);
    }
    
    template<typename KeyType, typename ValueType>
    BWTree::const_iterator BWTree::find(const KeyType &key) const {
        BWTree::EpochGuard(epoch);
    }

    template<typename KeyType, typename ValueType>
    BWTree::size_type BWTree::count(const KeyType &key) const {

    }

    template<typename KeyType, typename ValueType>
    BWTree::PairType BWTree::insert(const BWTree::PairType &record) {
        BWTree::EpochGuard(epoch);
        //start delete
        //find record page by key
        auto page = findPage(record.first);
        auto node = getNodeByPID(page->pid);
        BWTree::DeltaInsert* newNode = new BWTree::DeltaInsert(node, record);
        if(!node.compare_exchange_weak(page, newNode)) {
            //try again
        }else{
            //may split
            splitPage();
            //may consolidation
        }
    }

    //may erase many nodes
    template<typename KeyType, typename ValueType>
    BWTree::size_type BWTree::erase(const KeyType &key){
        BWTree::EpochGuard(epoch);
        auto page = findPage(key);
        if(page == nullptr)
            return 0;
        auto node = getNodeByPID(page->pid);
        BWTree::DeltaDelete* newNode = new BWTree::DeltaDelete(page, key);
        if(!node.compare_exchange_weak(page, newNode)) {
            freeNode(newNode);
            //try again
        }else{
            //may split
            splitPage();
            //may consolidation
        }
    }

    //erase only the matching index entry
    template<typename KeyType, typename ValueType>
    BWTree::size_type BWTree::erase(const PairType &key){

    }

    template<typename KeyType, typename ValueType>
    BWTree::Node* BWTree::findPage(const KeyType &key) {
        PID nextPID = root_.load();
        //go through tree
        while(nextPID != MaxPID){
            Node *nextNode = getNodeByPID(nextPID);
            if (isLeaf(nextNode)) {
                break;
            }
            if(nextNode != nullptr){
                switch(nextNode->getType()){
                    case NodeTypes::deltaUpdate:{
                        auto n = static_cast<DeltaUpdate*>(nextNode);

                    };

                    case NodeTypes::innerNode:{
                        auto n = static_cast<InnerNode*>(nextNode);
                    };

                    case NodeTypes::deltaSplit:{
                        auto n = static_cast<DeltaSplit*>(nextNode);
                    };
                }
            }

        }
        //deal with data
        while(nextPID != MaxPID){
            Node *nextNode = getNodeByPID(nextPID);
            if(nextNode != nullptr){
                switch(nextNode->getType()){
                    case NodeTypes::leafNode:{
                        auto n = static_cast<LeafNode*>(nextNode);

                    };

                    case NodeTypes::deltaInsert:{
                        auto n = static_cast<DeltaInsert*>(nextNode);
                    };

                    case NodeTypes::deltaDelete:{
                        auto n = static_cast<DeltaDelete*>(nextNode);
                    };

                    case NodeTypes::deltaSplit:{
                        auto n = static_cast<DeltaSplit*>(nextNode);
                    };
                }
            }
        }
        //may find many results
    }

    template<typename KeyType, typename ValueType>
    void BWTree::splitPage(const PID splitPage, const PID splitPageParent) {
    
    }

    template<typename KeyType, typename ValueType>
    void BWTree::consolidateLeafPage(const PID page, Node* startNode) {
    
    }
    
    template<typename KeyType, typename ValueType>
    void BWTree::consolidateInnerPage(const PID page, Node* startNode) {
    
    }

    template<typename KeyType, typename ValueType>
    void BWTree::freeNode(Node* n){
        
    }

}  // End index namespace
}  // End peloton namespace
