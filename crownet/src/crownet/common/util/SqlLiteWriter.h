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
#include "crownet/common/util/SqlLiteApi.h"

namespace crownet {

class SqlPrinter {
public:
    virtual ~SqlPrinter()=default;
    virtual void writeSqlStatement(std::shared_ptr<SqlLiteApi> sqlApi) = 0;
};

class SqlLiteWriter : public ActiveWriter  {
public:
    SqlLiteWriter(std::shared_ptr<SqlLiteApi> api, std::shared_ptr<SqlPrinter> printer)
        : sqlApi(api), printer(printer){}
    virtual ~SqlLiteWriter();

    virtual void initWriter() override;
    virtual void writeData() override;
    virtual void finish() override;

    void setPrinter(std::shared_ptr<SqlPrinter> p){ printer = p;}
    std::shared_ptr<SqlPrinter> getPrinter() { return printer;}

    void setSqlApi(std::shared_ptr<SqlLiteApi>);

private:
    std::shared_ptr<SqlLiteApi> sqlApi;
    std::shared_ptr<SqlPrinter> printer;
};
}
