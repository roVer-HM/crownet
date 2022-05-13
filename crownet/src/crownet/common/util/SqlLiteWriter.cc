/*
 * SqlLiteWriter.cc
 *
 *  Created on: Jan 26, 2022
 *      Author: vm-sts
 */

#include "SqlLiteWriter.h"

namespace crownet {


SqlLiteWriter::SqlLiteWriter(long bufferSize)
        : BufferWriter(bufferSize) {

}

SqlLiteWriter::~SqlLiteWriter() {
    close();
}

void SqlLiteWriter::initialize() {
    //only create Schema once!

    //todo check if sql file exist
    // if yes
        // do nothing (schema already created)
    // else create scheam using printer
        // printer->writeSqlStatement(write());
        // flush Buffer to disc immediately
        //flush();
}

void SqlLiteWriter::initWriter(){
    // insert data on writer init (e.g. metadata)
    printer->writeInitSqlStatement(write());
    // flush Buffer to disc immediately
    flush();
}

void SqlLiteWriter::writeBuffer(){
    //todo send buffer to sql API. buffer contains a string of multiple sql statements.
}

void SqlLiteWriter::writeData() {
    printer->writeSqlStatement(write());
}
void SqlLiteWriter::finish() {
    close();
}
void SqlLiteWriter::flush() {
    writeBuffer();
}
void SqlLiteWriter::close() {
    if (!closed){
        flush();
        closed = true;
    }
}
}

