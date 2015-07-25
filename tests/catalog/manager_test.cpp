/*-------------------------------------------------------------------------
 *
 * catalog_test.cpp
 * file description
 *
 * Copyright(c) 2015, CMU
 *
 * /n-store/tests/catalog/catalog_test.cpp
 *
 *-------------------------------------------------------------------------
 */

#include "backend/catalog/manager.h"
#include "gtest/gtest.h"

#include "harness.h"
#include "backend/storage/tile_group.h"
#include "backend/storage/tile_group_factory.h"

namespace peloton {
namespace test {

//===--------------------------------------------------------------------===//
// Manager Tests
//===--------------------------------------------------------------------===//

void AddTileGroup() {
  // TILES
  std::vector<std::string> tile_column_names;
  std::vector<std::vector<std::string> > column_names;

  tile_column_names.push_back("INTEGER COL");
  column_names.push_back(tile_column_names);

  std::vector<catalog::Schema> schemas;
  std::vector<catalog::Column> columns;

  // SCHEMA
  catalog::Column column1(VALUE_TYPE_INTEGER, GetTypeSize(VALUE_TYPE_INTEGER),
                          "A", true);
  columns.push_back(column1);

  catalog::Schema *schema1 = new catalog::Schema(columns);
  schemas.push_back(*schema1);

  storage::AbstractBackend *backend = new storage::VMBackend();

  for (oid_t txn_itr = 0; txn_itr < 100; txn_itr++) {
    storage::TileGroup *tile_group = storage::TileGroupFactory::GetTileGroup(
        INVALID_OID, INVALID_OID, INVALID_OID, nullptr, backend, schemas, 3);

    delete tile_group;
  }

  delete schema1;
  delete backend;
}

TEST(ManagerTests, TransactionTest) {
  LaunchParallelTest(8, AddTileGroup);

  std::cout << "Catalog allocations :: "
            << catalog::Manager::GetInstance().GetCurrentOid() << "\n";

  EXPECT_EQ(catalog::Manager::GetInstance().GetCurrentOid(), 800);
}

}  // End test namespace
}  // End peloton namespace
