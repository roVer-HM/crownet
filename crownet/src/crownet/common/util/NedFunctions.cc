/*
 * NedFunctions.h
 *
 *  Created on: Dec 10, 2020
 *      Author: sts
 */

#include "inet/common/INETDefs.h"

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


}  // namespace utils
}  // namespace crownet
