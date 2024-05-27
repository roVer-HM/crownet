#include "SqlLiteApi.h"
#include "SqlLiteSchema.h"
#include <omnetpp/cexception.h>

SqlLiteApi::SqlLiteApi(int maxCommitCount)
{
    db = nullptr;
    add_dcd_map_smt = nullptr;
    add_metadata_smt = nullptr;
    add_dcd_map_glb_smt = nullptr;
    add_alg_smt = nullptr;
    commitCount = 0;
    maxCommits = maxCommitCount;
}

SqlLiteApi::~SqlLiteApi()
{
    std::cout << "~SqlLiteApi" << std::endl;
    cleanup();
}

int SqlLiteApi::getAlgId(const std::string& value){
    if (alg_index_map.find(value) == alg_index_map.end()){
        int alg_index = write_alg(value);
        alg_index_map[value] = alg_index;
    }
    return alg_index_map[value];
}


inline void SqlLiteApi::checkOK(int sqlite3_result)
{
    if (sqlite3_result != SQLITE_OK)
        error(sqlite3_errmsg(db));
}

inline void SqlLiteApi::checkDone(int sqlite3_result)
{
    if (sqlite3_result != SQLITE_DONE)
        error(sqlite3_errmsg(db));
}

void SqlLiteApi::error(const char *errmsg)
{
    std::string msg = errmsg ? errmsg : "unknown error";
    if (db == nullptr) msg = "Database not open (db=nullptr), or " + msg; // sqlite's own error message is usually 'out of memory' (?)
    std::string fname = this->fname; // cleanup may clear it
    cleanup();
    std::string errorMsq = "SQLite error '" + msg + "' on file '" + fname.c_str() + "'";
    // throw std::runtime_error("SQLite error '%s' on file '%s'", msg.c_str(), fname.c_str());
    throw omnetpp::cRuntimeError("%s", errorMsq.c_str());
    // std::runtime_error(errorMsq); // is a opp_runtime_error in omnetpp code
}

void SqlLiteApi::prepareStatements()
{
    prepareStatement(add_dcd_map_smt,
            "INSERT INTO dcd_map \
            (hostId, simtime, x, y, count, measured_t, received_t, source, selection, selectionRank, sourceHost, sourceEntry, hostEntry, rsd_id, own_cell, owner_rsd_id) \
             VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);"
    );
    prepareStatement(add_dcd_map_glb_smt,
            "INSERT INTO dcd_map_glb \
            (simtime, x, y, count) \
             VALUES (?,?,?,?);"
    );
    prepareStatement(add_metadata_smt, "INSERT INTO metadata (hostId, key, value) VALUES (?, ?, ?);");
    prepareStatement(add_alg_smt, "INSERT INTO alg_mapping (alg_name) VALUES (?)");
    prepareStatement(add_glb_node_id_mapping_smt, "INSERT INTO glb_node_id_mapping (glb_map_uid, node_id) VALUES (?, ?)");
}

void SqlLiteApi::finalizeStatement(sqlite3_stmt *&stmt)
{
    sqlite3_finalize(stmt);
    stmt = nullptr;
}

void SqlLiteApi::prepareStatement(sqlite3_stmt *&stmt, const char *sql)
{
    checkOK(sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr));
}

void SqlLiteApi::initialize(const char *filename)
{
    fname = filename;
    if(db == nullptr){
        checkOK(sqlite3_open(filename, &db));

        checkOK(sqlite3_busy_timeout(db, 10000));
        checkOK(sqlite3_exec(db, crownet::SQL_CREATE_TABLES, nullptr, 0, nullptr));
        prepareStatements();
        checkOK(sqlite3_exec(db, "BEGIN IMMEDIATE TRANSACTION;", nullptr, 0, nullptr));
    }
}

int64_t SqlLiteApi::write_map(const int hostId, const double simtime, const double x, const double y,
        const double count, const double measured_t, const double received_t, const int source, std::string selection,
        const double selectionRank, const double sourceHost, const double sourceEntry, const double hostEntry,
        const int rsd_id, const int own_cell, const int owner_rsd_id)
{
    int selectionId = getAlgId(selection);
    checkOK(sqlite3_reset(add_dcd_map_smt));

    checkOK(sqlite3_bind_int64(add_dcd_map_smt, 1, hostId));
    checkOK(sqlite3_bind_double(add_dcd_map_smt, 2, simtime));
    checkOK(sqlite3_bind_double(add_dcd_map_smt, 3, x));
    checkOK(sqlite3_bind_double(add_dcd_map_smt, 4, y));
    checkOK(sqlite3_bind_double(add_dcd_map_smt, 5, count));
    checkOK(sqlite3_bind_double(add_dcd_map_smt, 6, measured_t));
    checkOK(sqlite3_bind_double(add_dcd_map_smt, 7, received_t));
    checkOK(sqlite3_bind_double(add_dcd_map_smt, 8, source));
    checkOK(sqlite3_bind_int64(add_dcd_map_smt, 9, selectionId));
    checkOK(sqlite3_bind_double(add_dcd_map_smt, 10, selectionRank));
    checkOK(sqlite3_bind_double(add_dcd_map_smt, 11, sourceHost));
    checkOK(sqlite3_bind_double(add_dcd_map_smt, 12, sourceEntry));
    checkOK(sqlite3_bind_double(add_dcd_map_smt, 13, hostEntry));
    checkOK(sqlite3_bind_int64(add_dcd_map_smt, 14, rsd_id));
    checkOK(sqlite3_bind_int64(add_dcd_map_smt, 15, own_cell));
    checkOK(sqlite3_bind_int64(add_dcd_map_smt, 16, owner_rsd_id));
    checkDone(sqlite3_step(add_dcd_map_smt));
    checkOK(sqlite3_clear_bindings(add_dcd_map_smt));
    sqlite3_int64 ret = sqlite3_last_insert_rowid(db);
    checkCommit();
    return (int64_t)ret;
}

int64_t SqlLiteApi::write_map_glb(const double simtime, const double x, const double y,
        const double count) {
    checkOK(sqlite3_reset(add_dcd_map_glb_smt));

    checkOK(sqlite3_bind_double(add_dcd_map_glb_smt, 1, simtime));
    checkOK(sqlite3_bind_double(add_dcd_map_glb_smt, 2, x));
    checkOK(sqlite3_bind_double(add_dcd_map_glb_smt, 3, y));
    checkOK(sqlite3_bind_double(add_dcd_map_glb_smt, 4, count));
    checkDone(sqlite3_step(add_dcd_map_glb_smt));
    checkOK(sqlite3_clear_bindings(add_dcd_map_glb_smt));
    sqlite3_int64 ret = sqlite3_last_insert_rowid(db);
    checkCommit();
    return (int64_t)ret;
}

int64_t SqlLiteApi::write_metadata(const int hostId, const std::string& key, const std::string& value)
{
    checkOK(sqlite3_reset(add_metadata_smt));
    checkOK(sqlite3_bind_int64(add_metadata_smt, 1, hostId));
    checkOK(sqlite3_bind_text(add_metadata_smt, 2, key.c_str(), key.size(), SQLITE_STATIC));
    checkOK(sqlite3_bind_text(add_metadata_smt, 3, value.c_str(), value.size(), SQLITE_STATIC));
    checkDone(sqlite3_step(add_metadata_smt));
    checkOK(sqlite3_clear_bindings(add_metadata_smt));
    sqlite3_int64 ret = sqlite3_last_insert_rowid(db);
    checkCommit();
    return (int64_t)ret;
}

int64_t SqlLiteApi::write_alg(const std::string& value) {
    checkOK(sqlite3_reset(add_alg_smt));
    checkOK(sqlite3_bind_text(add_alg_smt, 1, value.c_str(), value.size(), SQLITE_STATIC));
    checkDone(sqlite3_step(add_alg_smt));
    checkOK(sqlite3_clear_bindings(add_alg_smt));
    sqlite3_int64 ret = sqlite3_last_insert_rowid(db);
    checkCommit();
    return (int64_t)ret;
}

int64_t SqlLiteApi::write_glb_node_id_mapping(const int64_t glb_id, const int node_id) {
    checkOK(sqlite3_reset(add_glb_node_id_mapping_smt));
    checkOK(sqlite3_bind_int64(add_glb_node_id_mapping_smt, 1, glb_id));
    checkOK(sqlite3_bind_int64(add_glb_node_id_mapping_smt, 2, node_id));
    checkDone(sqlite3_step(add_glb_node_id_mapping_smt));
    checkOK(sqlite3_clear_bindings(add_glb_node_id_mapping_smt));
    sqlite3_int64 ret = sqlite3_last_insert_rowid(db);
    checkCommit();
    return (int64_t)ret;

}




void SqlLiteApi::checkCommit()
{
    if(++commitCount >= maxCommits)
    {
        commitAndBeginNew();
    }
}

void SqlLiteApi::cleanup()
{
    if(db){
        finalizeStatement(add_dcd_map_smt);
        finalizeStatement(add_dcd_map_glb_smt);
        finalizeStatement(add_metadata_smt);

        // no checkOK because it whould throw
        sqlite3_exec(db,"COMMIT TRANSACTION;", nullptr, nullptr, nullptr);
        sqlite3_exec(db,"PRAGMA journal_mode = DELETE;", nullptr, nullptr, nullptr);
        sqlite3_close(db);

        db = nullptr;
        fname = "";
    }
}

void SqlLiteApi::commitAndBeginNew()
{
    checkOK(sqlite3_exec(db, "COMMIT TRANSACTION;", nullptr, nullptr, nullptr));
    checkOK(sqlite3_exec(db, "BEGIN IMMEDIATE TRANSACTION;", nullptr, nullptr, nullptr));
    commitCount = 0;
}

void SqlLiteApi::flush()
{
    if(db)
    {
        commitAndBeginNew();
    }
}
