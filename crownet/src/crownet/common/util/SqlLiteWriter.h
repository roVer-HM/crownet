/*
 * SqlLiteWriter.h
 *
 *  Created on: Jan 26, 2022
 *      Author: vm-sts
 */

#pragma once

#include <fstream>
#include <sstream>
#include "Writer.h"
#include <memory>

namespace crownet {

class SqlPrinter {
public:
    virtual ~SqlPrinter()=default;
    virtual void writeSqlStatement(std::ostream& out) = 0;
    virtual void createSchema(std::ostream& out) = 0;
    virtual void writeInitSqlStatement(std::ostream& out) = 0;
};

class SqlLiteWriter : public BufferWriter,
                      public ActiveWriter  {
public:
    SqlLiteWriter(long bufferSize=8192);
    virtual ~SqlLiteWriter();

    virtual void writeBuffer() override;
    virtual void initWriter() override;
    virtual void writeData() override;
    virtual void finish() override;
    virtual void flush() override;
    virtual void close() override;
    virtual void initialize() override;
    void setPrinter(std::shared_ptr<SqlPrinter> p){ printer = p;}
    std::shared_ptr<SqlPrinter> getPrinter() { return printer;}
    // todo mw
//    void setSqlApi(std::shared_ptr<SqlAPI>);

private:
    std::shared_ptr<SqlPrinter> printer;
    // todo mw add sql handle here
    // std::shared_ptr<SqlAPI> sqlApi;
};
}
