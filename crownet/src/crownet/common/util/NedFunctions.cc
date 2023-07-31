/*
 * NedFunctions.h
 *
 *  Created on: Dec 10, 2020
 *      Author: sts
 */

#include "inet/common/INETDefs.h"

#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <fstream>
#include <iostream>

#if OMNETPP_VERSION < 0x600
  #include <omnetpp/cnedvalue.h>
#endif

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#define DEF(FUNCTION, SIGNATURE, CATEGORY, DESCRIPTION) \
        cValue FUNCTION(cComponent *context, cValue argv[], int argc); \
        Define_NED_Function2(FUNCTION, SIGNATURE, CATEGORY, DESCRIPTION);

#define DEF2(FUNCTION, SIGNATURE, CATEGORY, DESCRIPTION) \
        cValue FUNCTION(cExpression::Context *context, cValue argv[], int argc); \
        Define_NED_Function2(FUNCTION, SIGNATURE, CATEGORY, DESCRIPTION);



using namespace omnetpp;

namespace crownet {

namespace utils {

cNEDValue nedf_ifTrueOrElse(cComponent *context, cNEDValue argv[], int argc) {
  if (argv[0].getType() != cNEDValue::BOOL)
    throw cRuntimeError("useOrElse(): bool arguments expected");
  bool pred = argv[0].boolValue();
  if (pred) {
    return argv[1];
  } else {
    return argv[2];
  }
}

DEF(
    nedf_ifTrueOrElse,
    "string ifTrueOrElse(bool pred, string first, string second)", "misc",
    "if pred is true return first else second.");


DEF2(
    nedf_absFilePath,
    "string absFilePath(string path)", "misc",
    "return absolute file path based on current file the configuration is set.");


cValue nedf_absFilePath(cExpression::Context  *context, cValue argv[], int argc){
    fs::path filename(argv[0].stringValue());
    fs::path filepath;
    if (strlen(context->baseDirectory) == 0){
        filepath = filename;
    } else {
        fs::path base(context->baseDirectory);
        filepath = base / filename;
    }
    return filepath.string();
}

DEF2(nedf_parseXML_Gz,
    "xml xmldocGz(string filename, string xpath?)",
    "xml",
    "Parses the given gziped-XML file into a cXMLElement tree, and returns the root element. "
    "When called with two arguments, it returns the first element from the tree that "
    "matches the expression given in simplified XPath syntax.")

cValue nedf_parseXML_Gz(cExpression::Context *context, cValue argv[], int argc)
{
    fs::path filename(argv[0].stringValue());
    fs::path filepath;
    if (strlen(context->baseDirectory) == 0){
        filepath = filename;
    } else {
        fs::path base(context->baseDirectory);
        filepath = base / filename;
    }
    std::ifstream file(filepath.c_str(), std::ios_base::in | std::ios_base::binary);
    boost::iostreams::filtering_streambuf<boost::iostreams::input> in;
    in.push(boost::iostreams::gzip_decompressor());
    in.push(file);
    std::stringstream out;
    boost::iostreams::copy(in, out);

    const char *xpath = argc == 1 ? nullptr : argv[1].stringValue();
    cXMLElement *node = getEnvir()->getParsedXMLString(out.str().c_str(), xpath);
    if (!node) {
        if (argc == 1)
            throw cRuntimeError("Empty XML document");
        else
            throw cRuntimeError("Element \"%s\" not found in XML document string", xpath);
    }
    return static_cast<cObject*>(node);
}



}  // namespace utils
}  // namespace crownet
