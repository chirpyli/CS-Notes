// leveldb demo

#include <cassert>
#include"leveldb/db.h"
#include<iostream>

using namespace std;

int main() {
    cout << "leveldb demo............" << std::endl;

    leveldb::DB* db;
    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::Status status = leveldb::DB::Open(options, "./data", &db);

    assert(status.ok());

    std::string key1 = "key1";
    std::string value1 = "value1";
    status = db->Put(leveldb::WriteOptions(), key1, value1);
    assert(status.ok());

    std::string value;
    status = db->Get(leveldb::ReadOptions(), key1, &value);
    assert(status.ok());
    cout << "get " << key1 << " = " << value << endl;

    delete db;

    return 0;
}