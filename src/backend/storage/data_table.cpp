/*-------------------------------------------------------------------------
 *
 * table.cpp
 * file description
 *
 * Copyright(c) 2015, CMU
 *
 * /n-store/src/storage/table.cpp
 *
 *-------------------------------------------------------------------------
 */

#include "backend/storage/data_table.h"
#include "backend/storage/database.h"

#include "backend/common/exception.h"
#include "backend/index/index.h"
#include "backend/common/logger.h"
#include "backend/catalog/manager.h"

#include <mutex>

namespace peloton {
namespace storage {

DataTable::DataTable(catalog::Schema *schema,
                     AbstractBackend *backend,
                     std::string table_name,
                     oid_t table_oid,
                     size_t tuples_per_tilegroup)
: AbstractTable(schema, backend, tuples_per_tilegroup),
  table_name(table_name), table_oid(table_oid) {
}

DataTable::~DataTable() {
  // clean up indices
  for (auto index : indexes) {
    delete index;
  }
}

void DataTable::AddIndex(index::Index *index) {
  std::lock_guard<std::mutex> lock(table_mutex);
  indexes.push_back(index);
}

void DataTable::AddUniqueIndex(index::Index *index) {
  std::lock_guard<std::mutex> lock(table_unique_index_mutex);
  unique_indexes.push_back(index);
}

void DataTable::AddReferenceTable( catalog::ReferenceTableInfo *reference_table_info){

  std::lock_guard<std::mutex> lock( table_reference_table_mutex );
  //FIXME? TODO?
  catalog::ReferenceTableInfo* ref = new catalog::ReferenceTableInfo( *reference_table_info );
  reference_table_infos.push_back( ref );

// FIXME
//  catalog::Schema* schema = this->GetSchema();
//  for( auto column_name : reference_table_info->GetFKColumnNames() )
//  {
//    catalog::Constraint *constraint = new catalog::Constraint( CONSTRAINT_TYPE_FOREIGN );
//    constraint->SetReferenceTablePosition( this->GetReferenceTableCount() ) ;
//    schema->AddConstraintByColumnName( column_name, constraint );
//  }

}

void DataTable::SetPrimaryIndex(index::Index *index) {
  PrimaryKey_Index = index;
}

void DataTable::SetRawCheckExpr(Node* _raw_check_expr){
  raw_check_expr = (Node*) copyObject ( (void*) _raw_check_expr );
}

index::Index*  DataTable::GetPrimaryIndex(){
  return PrimaryKey_Index;
}

ItemPointer DataTable::InsertTuple(txn_id_t transaction_id, const storage::Tuple *tuple, bool update) {

  // TODO: Check basic integrity constraints!
  //       We don't want to do uniqueness or fkey checks here because
  //       that would be additional index look-ups per tuple
  //       But this is where we can do basic things like evaluate non-negative checks

  // Insert the tuples in the base table
  ItemPointer location = AbstractTable::InsertTuple(transaction_id, tuple);

  // Index checks and updates
  if (update == false) {
    if (TryInsertInIndexes(tuple, location) == false) {

      // FIXME
      // Need to think about how we want to actually do this, because right
      // now there is no way to get back the target TileGroup and slot from
      // AbstractTable::InsertTuple
      // tile_group->ReclaimTuple(tuple_slot);

      throw ConstraintException("Index constraint violated : " + tuple->GetInfo());
      return INVALID_ITEMPOINTER;
    }
  }
  else {
    // just do a blind insert
    InsertInIndexes(tuple, location);
    return location;
  }

  return location;
}
storage::DataTable* DataTable::GetReferenceTable(int position) {
  assert( position < reference_table_infos.size() );

  peloton::storage::Database* db = peloton::storage::Database::GetDatabaseById( GetCurrentDatabaseOid() );

  oid_t relation_id = reference_table_infos[position]->GetPrimaryKeyTableId();

  return db->GetTableById( relation_id );
}



void DataTable::InsertInIndexes(const storage::Tuple *tuple, ItemPointer location) {
  for (auto index : indexes) {
    if (index->InsertEntry(tuple, location) == false) {
      throw ExecutorException("Failed to insert tuple into index");
    }
  }
}

bool DataTable::TryInsertInIndexes(const storage::Tuple *tuple, ItemPointer location) {

  int index_count = GetIndexCount();
  for (int index_itr = index_count - 1; index_itr >= 0; --index_itr) {

    // No need to check if it does not have unique keys
    if(indexes[index_itr]->HasUniqueKeys() == false) {
      bool status = indexes[index_itr]->InsertEntry(tuple, location);
      if (status == true)
        continue;
    }

    // Check if key already exists
    if (indexes[index_itr]->Exists(tuple) == true) {
      LOG_ERROR("Failed to insert into index %s.%s [%s]",
                GetName().c_str(), indexes[index_itr]->GetName().c_str(),
                indexes[index_itr]->GetTypeName().c_str());
    }
    else {
      bool status = indexes[index_itr]->InsertEntry(tuple, location);
      if (status == true)
        continue;
    }

    // Undo insert in other indexes as well
    for (int prev_index_itr = index_itr + 1; prev_index_itr < index_count; ++prev_index_itr) {
      indexes[prev_index_itr]->DeleteEntry(tuple);
    }
    return false;
  }

  return true;
}

void DataTable::DeleteInIndexes(const storage::Tuple *tuple) {
  for (auto index : indexes) {
    if (index->DeleteEntry(tuple) == false) {
      throw ExecutorException("Failed to delete tuple from index " +
                              GetName() + "." + index->GetName() + " " +index->GetTypeName());
    }
  }
}


std::ostream& operator<<(std::ostream& os, const DataTable& table) {
  os << "=====================================================\n";
  os << "TABLE :\n";

  oid_t tile_group_count = table.GetTileGroupCount();
  std::cout << "Tile Group Count : " << tile_group_count << "\n";

  oid_t tuple_count = 0;
  for (oid_t tile_group_itr = 0 ; tile_group_itr < tile_group_count ; tile_group_itr++) {
    auto tile_group = table.GetTileGroup(tile_group_itr);
    auto tile_tuple_count = tile_group->GetNextTupleSlot();

    std::cout << "Tile Group Id  : " << tile_group_itr << " Tuple Count : " << tile_tuple_count << "\n";
    std::cout << (*tile_group);

    tuple_count += tile_tuple_count;
  }

  std::cout << "Table Tuple Count :: " << tuple_count << "\n";

  os << "=====================================================\n";

  return os;
}

} // End storage namespace
} // End peloton namespace

