#ifndef SQLLITEAPI_H
#define SQLLITEAPI_H
#include "sqlite3.h"
#include <iostream>
#include <map>


class WriteInterface{
public:
    virtual int64_t write_map(const int hostId, const double simtime, const double x, const double y,
            const double count, const double measured_t, const double received_t, const int source, std::string selection,
            const double selectionRank, const double sourceHost, const double sourceEntry, const double hostEntry,
            const int rsd_id, const int own_cell, const int owner_rsd_id) = 0;
    virtual int64_t write_map_glb(const double simtime, const double x, const double y, const double count) = 0;
    virtual int64_t write_metadata(const int hostId, const std::string& key, const std::string& value) = 0;
    virtual int64_t write_alg(const std::string& value) = 0;
    virtual int64_t write_glb_node_id_mapping(const int64_t glb_id, const int node_id) = 0;
    virtual int getAlgId(const std::string& value) = 0;
    virtual ~WriteInterface() = default;
};

class SqlLiteApi : public WriteInterface
{
private:
    sqlite3* db;
    std::string fname;
    sqlite3_stmt *add_dcd_map_smt;
    sqlite3_stmt *add_metadata_smt;
    sqlite3_stmt *add_dcd_map_glb_smt;
    sqlite3_stmt *add_alg_smt;
    sqlite3_stmt *add_glb_node_id_mapping_smt;
    int commitCount;
    int maxCommits;
    std::map<std::string, int> alg_index_map;


protected:
    void prepareStatements();
    void cleanup(); // MUST NOT THROW
    void commitAndBeginNew();
    void prepareStatement(sqlite3_stmt *&stmt, const char *sql);
    void finalizeStatement(sqlite3_stmt *&stmt);
    void checkOK(int sqlite3_result);
    void checkDone(int sqlite3_result);
    void error(const char *errormsg);
    void checkCommit();

public:
    SqlLiteApi(int maxCommitCount=100);
    ~SqlLiteApi();
    void initialize(const char *filename);
    virtual int64_t write_map(const int hostId, const double simtime, const double x, const double y,
            const double count, const double measured_t, const double received_t, const int source, std::string selection,
            const double selectionRank, const double sourceHost, const double sourceEntry, const double hostEntry,
            const int rsd_id, const int own_cell, const int owner_rsd_id) override;
    virtual int64_t write_map_glb(const double simtime, const double x, const double y, const double count) override;
    virtual int64_t write_metadata(const int hostId, const std::string& key, const std::string& value) override;
    virtual int64_t write_alg(const std::string& value) override;
    virtual int64_t write_glb_node_id_mapping(const int64_t glb_id, const int node_id) override;
    virtual int getAlgId(const std::string& value) override;

    void flush();
};

#endif
