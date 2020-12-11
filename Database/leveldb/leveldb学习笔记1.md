
### leveldb程序示例
使用leveldb，第一步是一定是设置相关配置，然后创建一个新的数据库或者打开一个已存在的数据库。示例[leveldb_demo.cpp](./leveldb_demo.cpp)如下：
```c++
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
```
编译`g++ leveldb_demo.cpp -lleveldb`，运行后，在`data`目录下生成了一些文件，因为目前数据非常少，还少了一些sst数据文件。结果如下：
```
data/
├── 000007.log  // 数据日志
├── CURRENT     // 文件内容为MANIFEST-000006，保存最新的MANIFEST
├── LOCK
├── LOG         // 当前程序运行日志
├── LOG.old     // 历史运行日志
└── MANIFEST-000006   // 保存一些非常重要的元信息，记录不同level的SST文件，包括每个SST文件的key range、大小等。
```
我们就看一下，这段代码背后发生了什么。

### `DB`及`Options`

###### `DB`
最开始是`leveldb::DB* db;`，`DB`是一个抽象类，是对KV操作的抽象接口， 具体实现是在`DBImpl`，后面会分析。其操作有点类似于`HashMap`一类的数据结构。定义如下：
```c++
// A DB is a persistent ordered map from keys to values. A DB is safe for concurrent access from multiple threads without any external synchronization.
class DB {
 public:
  // Open the database with the specified "name".
  // Stores a pointer to a heap-allocated database in *dbptr and returns OK on success.
  // Stores NULL in *dbptr and returns a non-OK status on error.
  // Caller should delete *dbptr when it is no longer needed.
  static Status Open(const Options& options, const std::string& name, DB** dbptr);

  DB() { }
  virtual ~DB();

  // Set the database entry for "key" to "value".  Returns OK on success, and a non-OK status on error. Note: consider setting options.sync = true.
  virtual Status Put(const WriteOptions& options, const Slice& key, const Slice& value) = 0;

  // Remove the database entry (if any) for "key".  Returns OK on success, and a non-OK status on error.  It is not an error if "key" did not exist in the database.
  // Note: consider setting options.sync = true.
  virtual Status Delete(const WriteOptions& options, const Slice& key) = 0;

  // Apply the specified updates to the database. Returns OK on success, non-OK on failure. Note: consider setting options.sync = true.
  virtual Status Write(const WriteOptions& options, WriteBatch* updates) = 0;

  // If the database contains an entry for "key" store the corresponding value in *value and return OK.
  // If there is no entry for "key" leave *value unchanged and return a status for which Status::IsNotFound() returns true. May return some other Status on an error.
  virtual Status Get(const ReadOptions& options, const Slice& key, std::string* value) = 0;

  // Return a heap-allocated iterator over the contents of the database.
  // The result of NewIterator() is initially invalid (caller must call one of the Seek methods on the iterator before using it).
  // Caller should delete the iterator when it is no longer needed. The returned iterator should be deleted before this db is deleted.
  virtual Iterator* NewIterator(const ReadOptions& options) = 0;

  // Return a handle to the current DB state.  Iterators created with this handle will all observe a stable snapshot of the current DB state.  The caller must call ReleaseSnapshot(result) when the snapshot is no longer needed.
  virtual const Snapshot* GetSnapshot() = 0;
  virtual void ReleaseSnapshot(const Snapshot* snapshot) = 0;   // Release a previously acquired snapshot.  The caller must not use "snapshot" after this call.

  // DB implementations can export properties about their state via this method.  If "property" is a valid property understood by this
  // DB implementation, fills "*value" with its current value and returns true.  Otherwise returns false.
  //
  // Valid property names include:
  //
  //  "leveldb.num-files-at-level<N>" - return the number of files at level <N>,
  //     where <N> is an ASCII representation of a level number (e.g. "0").
  //  "leveldb.stats" - returns a multi-line string that describes statistics
  //     about the internal operation of the DB.
  //  "leveldb.sstables" - returns a multi-line string that describes all
  //     of the sstables that make up the db contents.
  //  "leveldb.approximate-memory-usage" - returns the approximate number of
  //     bytes of memory in use by the DB.
  virtual bool GetProperty(const Slice& property, std::string* value) = 0;

  // For each i in [0,n-1], store in "sizes[i]", the approximate
  // file system space used by keys in "[range[i].start .. range[i].limit)".
  //
  // Note that the returned sizes measure file system space usage, so
  // if the user data compresses by a factor of ten, the returned
  // sizes will be one-tenth the size of the corresponding user data size.
  //
  // The results may not include the sizes of recently written data.
  virtual void GetApproximateSizes(const Range* range, int n, uint64_t* sizes) = 0;

  // Compact the underlying storage for the key range [*begin,*end].
  // In particular, deleted and overwritten versions are discarded,
  // and the data is rearranged to reduce the cost of operations
  // needed to access the data.  This operation should typically only
  // be invoked by users who understand the underlying implementation.
  //
  // begin==NULL is treated as a key before all keys in the database.
  // end==NULL is treated as a key after all keys in the database.
  // Therefore the following call will compact the entire database:
  //    db->CompactRange(NULL, NULL);
  virtual void CompactRange(const Slice* begin, const Slice* end) = 0;

 private:
  DB(const DB&);  // No copying allowed
  void operator=(const DB&);
};
```
对KV数据库最基本的操作就是`Open`，`Put`，`Get`，`Delete`，我们后面会逐个分析。先看一下创建一个数据库实例或打开已有的数据库实例`Open`的参数`Options`。这个对宏观的理解leveldb非常重要。

###### `Options`
`leveldb::Options options;`可以进行相关的配置。但需要注意的是，数据库的某些配置最好不要轻易改变，需要考虑配置修改前后程序是否还会正常工作，比如自定义了`Key`比较大小的`Comparator`，因为`Key`排序对leveldb至关重要，如果修改前后的Key比较大小不相容，就可能会造成异常问题，所以必须保证前后定义的比较器有相同的名字和完全相同的key顺序。另外有些配置是可以动态更改的。后面几个数值相关的参数会影响性能，比如`write_buffer_size`。定义如下：
```c++
// Options to control the behavior of a database (passed to DB::Open)
struct LEVELDB_EXPORT Options {
  Options();

  // ---------影响行为的参数----------

  // Comparator used to define the order of keys in the table. Default: a comparator that uses lexicographic byte-wise ordering
  // REQUIRES: The client must ensure that the comparator supplied here has the same name and orders keys *exactly* the same as the
  // comparator provided to previous open calls on the same DB.
  const Comparator* comparator;   // key排序用的比较器，可自定义，默认字典序
  bool create_if_missing = false;   // If true, the database will be created if it is missing.
  bool error_if_exists = false;     // If true, an error is raised if the database already exists.

  // If true, the implementation will do aggressive checking of the data it is processing and will stop early if it detects any errors.  
  // This may have unforeseen ramifications: for example, a corruption of one DB entry may cause a large number of entries to become unreadable or for the entire DB to become unopenable.
  bool paranoid_checks = false;

  Env* env;  // Use the specified object to interact with the environment, e.g. to read/write files, schedule background work, etc. Default: Env::Default()

  // Any internal progress/error information generated by the db will be written to info_log if it is non-null, or to a file stored in the same directory as the DB contents if info_log is null.
  Logger* info_log = nullptr;

  // ---------影响性能的参数----------

  // Amount of data to build up in memory (backed by an unsorted log on disk) before converting to a sorted on-disk file.
  // Larger values increase performance, especially during bulk loads.
  // Up to two write buffers may be held in memory at the same time, so you may wish to adjust this parameter to control memory usage.
  // Also, a larger write buffer will result in a longer recovery time the next time the database is opened.
  size_t write_buffer_size = 4 * 1024 * 1024;

  // Number of open files that can be used by the DB.  You may need to increase this if your database has a large working set (budget one open file per 2MB of working set).
  int max_open_files = 1000;

  // Control over blocks (user data is stored in a set of blocks, and a block is the unit of reading from disk).
  // If non-null, use the specified cache for blocks. If null, leveldb will automatically create and use an 8MB internal cache.
  Cache* block_cache = nullptr;

  // Approximate size of user data packed per block.  Note that the block size specified here corresponds to uncompressed data.  
  // The actual size of the unit read from disk may be smaller if compression is enabled.  This parameter can be changed dynamically.
  size_t block_size = 4 * 1024;

  // Number of keys between restart points for delta encoding of keys. This parameter can be changed dynamically.  Most clients should leave this parameter alone.
  int block_restart_interval = 16;

  // Leveldb will write up to this amount of bytes to a file before
  // switching to a new one.
  // Most clients should leave this parameter alone.  However if your
  // filesystem is more efficient with larger files, you could
  // consider increasing the value.  The downside will be longer
  // compactions and hence longer latency/performance hiccups.
  // Another reason to increase this parameter might be when you are
  // initially populating a large database.
  size_t max_file_size = 2 * 1024 * 1024;

  // Compress blocks using the specified compression algorithm.  This parameter can be changed dynamically.
  CompressionType compression = kSnappyCompression;     // 压缩算法的选择，默认snappy算法。该压缩方法的速度快过大多数持久化存储的速度，因此可能并没有理由切换成不压缩。即使输入数据是不可压缩的，该方法的实现中也会有效识别并切换到不压缩模式。

  // EXPERIMENTAL: If true, append to existing MANIFEST and log files when a database is opened.  This can significantly speed up open.
  // Default: currently false, but may become true later.
  bool reuse_logs = false;

  // If non-null, use the specified filter policy to reduce disk reads. Many applications will benefit from passing the result of NewBloomFilterPolicy() here.
  const FilterPolicy* filter_policy = nullptr;  // 目的是降低磁盘读取
};
```
设置好配置参数后，就可以进入下一步，创建数据库实例了。
###### `DB::Open`
创建或者打开一个数据库实例，代码如下：
```c++
Status DB::Open(const Options& options, const std::string& dbname, DB** dbptr) {
  *dbptr = nullptr;

  DBImpl* impl = new DBImpl(options, dbname);   // DBImpl继承自DB，是DB抽象类的具体实现
  impl->mutex_.Lock();
  VersionEdit edit;
  // Recover handles create_if_missing, error_if_exists
  bool save_manifest = false;
  Status s = impl->Recover(&edit, &save_manifest);    // 调用Recover函数将LevelDB的历史状态恢复
  if (s.ok() && impl->mem_ == nullptr) {    //如果恢复后LevelDB当前的memtable为空则创建一个memtable和相应的log文件
    // Create new log and a corresponding memtable.
    uint64_t new_log_number = impl->versions_->NewFileNumber();
    WritableFile* lfile;
    s = options.env->NewWritableFile(LogFileName(dbname, new_log_number),
                                     &lfile);
    if (s.ok()) {
      edit.SetLogNumber(new_log_number);
      impl->logfile_ = lfile;
      impl->logfile_number_ = new_log_number;
      impl->log_ = new log::Writer(lfile);
      impl->mem_ = new MemTable(impl->internal_comparator_);    // 非常重要的memtable
      impl->mem_->Ref();
    }
  }
  if (s.ok() && save_manifest) {
    edit.SetPrevLogNumber(0);  // No older logs needed after recovery.
    edit.SetLogNumber(impl->logfile_number_);
    s = impl->versions_->LogAndApply(&edit, &impl->mutex_);
  }
  if (s.ok()) {
    impl->RemoveObsoleteFiles();
    impl->MaybeScheduleCompaction();
  }
  impl->mutex_.Unlock();
  if (s.ok()) {
    assert(impl->mem_ != nullptr);
    *dbptr = impl;
  } else {
    delete impl;
  }
  return s;
}
```

其中`DBImpl`类是比较重要的，定义如下：
```c++
class DBImpl : public DB {
 public:
  DBImpl(const Options& options, const std::string& dbname);

  DBImpl(const DBImpl&) = delete;
  DBImpl& operator=(const DBImpl&) = delete;

  ~DBImpl() override;

  // Implementations of the DB interface
  Status Put(const WriteOptions&, const Slice& key,
             const Slice& value) override;
  Status Delete(const WriteOptions&, const Slice& key) override;
  Status Write(const WriteOptions& options, WriteBatch* updates) override;
  Status Get(const ReadOptions& options, const Slice& key,
             std::string* value) override;
  Iterator* NewIterator(const ReadOptions&) override;
  const Snapshot* GetSnapshot() override;
  void ReleaseSnapshot(const Snapshot* snapshot) override;
  bool GetProperty(const Slice& property, std::string* value) override;
  void GetApproximateSizes(const Range* range, int n, uint64_t* sizes) override;
  void CompactRange(const Slice* begin, const Slice* end) override;

  // Extra methods (for testing) that are not in the public DB interface

  // Compact any files in the named level that overlap [*begin,*end]
  void TEST_CompactRange(int level, const Slice* begin, const Slice* end);

  // Force current memtable contents to be compacted.
  Status TEST_CompactMemTable();

  // Return an internal iterator over the current state of the database.
  // The keys of this iterator are internal keys (see format.h).
  // The returned iterator should be deleted when no longer needed.
  Iterator* TEST_NewInternalIterator();

  // Return the maximum overlapping data (in bytes) at next level for any
  // file at a level >= 1.
  int64_t TEST_MaxNextLevelOverlappingBytes();

  // Record a sample of bytes read at the specified internal key.
  // Samples are taken approximately once every config::kReadBytesPeriod
  // bytes.
  void RecordReadSample(Slice key);

 private:
  friend class DB;
  struct CompactionState;
  struct Writer;

  // Information for a manual compaction
  struct ManualCompaction {
    int level;
    bool done;
    const InternalKey* begin;  // null means beginning of key range
    const InternalKey* end;    // null means end of key range
    InternalKey tmp_storage;   // Used to keep track of compaction progress
  };

  // Per level compaction stats.  stats_[level] stores the stats for
  // compactions that produced data for the specified "level".
  struct CompactionStats {
    CompactionStats() : micros(0), bytes_read(0), bytes_written(0) {}

    void Add(const CompactionStats& c) {
      this->micros += c.micros;
      this->bytes_read += c.bytes_read;
      this->bytes_written += c.bytes_written;
    }

    int64_t micros;
    int64_t bytes_read;
    int64_t bytes_written;
  };

  Iterator* NewInternalIterator(const ReadOptions&,
                                SequenceNumber* latest_snapshot,
                                uint32_t* seed);

  Status NewDB();

  // Recover the descriptor from persistent storage.  May do a significant
  // amount of work to recover recently logged updates.  Any changes to
  // be made to the descriptor are added to *edit.
  Status Recover(VersionEdit* edit, bool* save_manifest)
      EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  void MaybeIgnoreError(Status* s) const;

  // Delete any unneeded files and stale in-memory entries.
  void RemoveObsoleteFiles() EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  // Compact the in-memory write buffer to disk.  Switches to a new
  // log-file/memtable and writes a new descriptor iff successful.
  // Errors are recorded in bg_error_.
  void CompactMemTable() EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  Status RecoverLogFile(uint64_t log_number, bool last_log, bool* save_manifest,
                        VersionEdit* edit, SequenceNumber* max_sequence)
      EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  Status WriteLevel0Table(MemTable* mem, VersionEdit* edit, Version* base)
      EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  Status MakeRoomForWrite(bool force /* compact even if there is room? */)
      EXCLUSIVE_LOCKS_REQUIRED(mutex_);
  WriteBatch* BuildBatchGroup(Writer** last_writer)
      EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  void RecordBackgroundError(const Status& s);

  void MaybeScheduleCompaction() EXCLUSIVE_LOCKS_REQUIRED(mutex_);
  static void BGWork(void* db);
  void BackgroundCall();
  void BackgroundCompaction() EXCLUSIVE_LOCKS_REQUIRED(mutex_);
  void CleanupCompaction(CompactionState* compact)
      EXCLUSIVE_LOCKS_REQUIRED(mutex_);
  Status DoCompactionWork(CompactionState* compact)
      EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  Status OpenCompactionOutputFile(CompactionState* compact);
  Status FinishCompactionOutputFile(CompactionState* compact, Iterator* input);
  Status InstallCompactionResults(CompactionState* compact)
      EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  const Comparator* user_comparator() const {
    return internal_comparator_.user_comparator();
  }

  // Constant after construction
  Env* const env_;
  const InternalKeyComparator internal_comparator_;
  const InternalFilterPolicy internal_filter_policy_;
  const Options options_;  // options_.comparator == &internal_comparator_
  const bool owns_info_log_;
  const bool owns_cache_;
  const std::string dbname_;

  // table_cache_ provides its own synchronization
  TableCache* const table_cache_;

  // Lock over the persistent DB state.  Non-null iff successfully acquired.
  FileLock* db_lock_;

  // State below is protected by mutex_
  port::Mutex mutex_;
  std::atomic<bool> shutting_down_;
  port::CondVar background_work_finished_signal_ GUARDED_BY(mutex_);
  MemTable* mem_;         // 这个非常重要，里面最核心的是跳跃链表
  MemTable* imm_ GUARDED_BY(mutex_);  // Memtable being compacted
  std::atomic<bool> has_imm_;         // So bg thread can detect non-null imm_
  WritableFile* logfile_;
  uint64_t logfile_number_ GUARDED_BY(mutex_);
  log::Writer* log_;
  uint32_t seed_ GUARDED_BY(mutex_);  // For sampling.

  // Queue of writers.
  std::deque<Writer*> writers_ GUARDED_BY(mutex_);
  WriteBatch* tmp_batch_ GUARDED_BY(mutex_);

  SnapshotList snapshots_ GUARDED_BY(mutex_);

  // Set of table files to protect from deletion because they are
  // part of ongoing compactions.
  std::set<uint64_t> pending_outputs_ GUARDED_BY(mutex_);

  // Has a background compaction been scheduled or is running?
  bool background_compaction_scheduled_ GUARDED_BY(mutex_);

  ManualCompaction* manual_compaction_ GUARDED_BY(mutex_);

  VersionSet* const versions_ GUARDED_BY(mutex_);

  // Have we encountered a background error in paranoid mode?
  Status bg_error_ GUARDED_BY(mutex_);

  CompactionStats stats_[config::kNumLevels] GUARDED_BY(mutex_);
};
```

### `MemTable`
在leveldb中，memtable是非常重要的一个概念，里面最核心的是跳跃链表。`memtable`是在内存中，存储最新插入的数据，超过一定大小即转为`immutable memtable`，等待归并到 SST 文件中。
```c++
class MemTable {
 public:
  // MemTables are reference counted.  The initial reference count
  // is zero and the caller must call Ref() at least once.
  explicit MemTable(const InternalKeyComparator& comparator);

  MemTable(const MemTable&) = delete;
  MemTable& operator=(const MemTable&) = delete;

  // Increase reference count.
  void Ref() { ++refs_; }

  // Drop reference count.  Delete if no more references exist.
  void Unref() {
    --refs_;
    assert(refs_ >= 0);
    if (refs_ <= 0) {
      delete this;
    }
  }

  // Returns an estimate of the number of bytes of data in use by this
  // data structure. It is safe to call when MemTable is being modified.
  size_t ApproximateMemoryUsage();

  // Return an iterator that yields the contents of the memtable.
  //
  // The caller must ensure that the underlying MemTable remains live
  // while the returned iterator is live.  The keys returned by this
  // iterator are internal keys encoded by AppendInternalKey in the
  // db/format.{h,cc} module.
  Iterator* NewIterator();

  // Add an entry into memtable that maps key to value at the
  // specified sequence number and with the specified type.
  // Typically value will be empty if type==kTypeDeletion.
  void Add(SequenceNumber seq, ValueType type, const Slice& key,
           const Slice& value);

  // If memtable contains a value for key, store it in *value and return true.
  // If memtable contains a deletion for key, store a NotFound() error
  // in *status and return true.
  // Else, return false.
  bool Get(const LookupKey& key, std::string* value, Status* s);

 private:
  friend class MemTableIterator;
  friend class MemTableBackwardIterator;

  struct KeyComparator {
    const InternalKeyComparator comparator;
    explicit KeyComparator(const InternalKeyComparator& c) : comparator(c) {}
    int operator()(const char* a, const char* b) const;
  };

  typedef SkipList<const char*, KeyComparator> Table;   // 跳跃链表

  ~MemTable();  // Private since only Unref() should be used to delete it

  KeyComparator comparator_;
  int refs_;
  Arena arena_;
  Table table_;
};
```

最重要的是`Add`和`Get`方法。其实就是封装了skiplist的相关方法实现。这里的memtable，可以认为是一个抽象，用于存储目前在内存中的KV数据，其内部实现，可以用跳跃链表，也可以用其他数据结构实现。
```c++
void MemTable::Add(SequenceNumber s, ValueType type, const Slice& key,
                   const Slice& value) {
  // Format of an entry is concatenation of:
  //  key_size     : varint32 of internal_key.size()
  //  key bytes    : char[internal_key.size()]
  //  value_size   : varint32 of value.size()
  //  value bytes  : char[value.size()]
  size_t key_size = key.size();
  size_t val_size = value.size();
  size_t internal_key_size = key_size + 8;
  const size_t encoded_len = VarintLength(internal_key_size) +
                             internal_key_size + VarintLength(val_size) +
                             val_size;
  char* buf = arena_.Allocate(encoded_len);
  char* p = EncodeVarint32(buf, internal_key_size);
  std::memcpy(p, key.data(), key_size);
  p += key_size;
  EncodeFixed64(p, (s << 8) | type);
  p += 8;
  p = EncodeVarint32(p, val_size);
  std::memcpy(p, value.data(), val_size);
  assert(p + val_size == buf + encoded_len);
  table_.Insert(buf);   // 跳跃链表插入
}

bool MemTable::Get(const LookupKey& key, std::string* value, Status* s) {
  Slice memkey = key.memtable_key();
  Table::Iterator iter(&table_);
  iter.Seek(memkey.data());
  if (iter.Valid()) {
    // entry format is:
    //    klength  varint32
    //    userkey  char[klength]
    //    tag      uint64
    //    vlength  varint32
    //    value    char[vlength]
    // Check that it belongs to same user key.  We do not check the
    // sequence number since the Seek() call above should have skipped
    // all entries with overly large sequence numbers.
    const char* entry = iter.key();
    uint32_t key_length;
    const char* key_ptr = GetVarint32Ptr(entry, entry + 5, &key_length);
    if (comparator_.comparator.user_comparator()->Compare(
            Slice(key_ptr, key_length - 8), key.user_key()) == 0) {
      // Correct user key
      const uint64_t tag = DecodeFixed64(key_ptr + key_length - 8);
      switch (static_cast<ValueType>(tag & 0xff)) {
        case kTypeValue: {
          Slice v = GetLengthPrefixedSlice(key_ptr + key_length);
          value->assign(v.data(), v.size());
          return true;
        }
        case kTypeDeletion:
          *s = Status::NotFound(Slice());
          return true;
      }
    }
  }
  return false;
}
```