/*
 * SqlLiteWriter.cc
 *
 *  Created on: Jan 26, 2022
 *      Author: vm-sts
 */

#include "SqlLiteWriter.h"

namespace crownet {




SqlLiteWriter::~SqlLiteWriter() {
    finish();
}

void SqlLiteWriter::initWriter(){
    // noting do do. Done by sqlApi
}

void SqlLiteWriter::writeData() {
    printer->writeSqlStatement(sqlApi);
}
void SqlLiteWriter::finish() {
    // Do not close Api. Some other node is still using it.
    // Still call flush to persist data.
    sqlApi->flush();
}
}

