/**
 * @file CfgToString.cpp
 * @brief Source file for CfgToStrign
 * @date 07/11/2020
 * @author Andre Neto
 *
 * @copyright Copyright 2015 F4E | European Joint Undertaking for ITER and
 * the Development of Fusion Energy ('Fusion for Energy').
 * Licensed under the EUPL, Version 1.1 or - as soon they will be approved
 * by the European Commission - subsequent versions of the EUPL (the "Licence")
 * You may not use this work except in compliance with the Licence.
 * You may obtain a copy of the Licence at: http://ec.europa.eu/idabc/eupl
 *
 * @warning Unless required by applicable law or agreed to in writing, 
 * software distributed under the Licence is distributed on an "AS IS"
 * basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
 * or implied. See the Licence permissions and limitations under the Licence.

 * @details This source file contains the definition of all the methods for
 * the class Playground (public, protected, and private). Be aware that some 
 * methods, such as those inline could be defined on the header file, instead.
 */

/**
 * Writes the output of a configuration file as C String
 */

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "AdvancedErrorManagement.h"
#include "ConfigurationDatabase.h"
#include "Directory.h"
#include "File.h"
#include "JsonParser.h"
#include "JsonPrinter.h"
#include "ObjectRegistryDatabase.h"
#include "Reference.h"
#include "ReferenceT.h"
#include "StreamString.h"
#include "StandardParser.h"
#include "StandardPrinter.h"
#include "StreamStructuredData.h"
#include "StreamStructuredDataI.h"
#include "XMLParser.h"
#include "XMLPrinter.h"

/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/
using namespace MARTe;

void MainErrorProcessFunction(const MARTe::ErrorManagement::ErrorInformation &errorInfo, const char * const errorDescription) {
    MARTe::StreamString errorCodeStr;
    MARTe::ErrorManagement::ErrorCodeToStream(errorInfo.header.errorType, errorCodeStr);
    MARTe::StreamString err;
    err.Printf("[%s - %s:%d]: %s", errorCodeStr.Buffer(), errorInfo.fileName, errorInfo.header.lineNumber, errorDescription);
    printf("%s\n", err.Buffer());
}

static bool ParseArgument(uint32 nargs, char8 **args, StreamString flag, StreamString &arg) {
    bool found = false;
    for (uint32 i=1u; (i<(nargs - 1u) && (!found)); i++) {
        found = (flag == args[i]);
        if (found) {
            arg = args[i + 1];
        }
    }
    if (!found) {
        REPORT_ERROR_STATIC(ErrorManagement::ParametersError, "Input parameter %s not found\n", flag.Buffer());
    }
    return found;
}

/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/
int main(int argc, char **argv) {
    SetErrorProcessFunction(&MainErrorProcessFunction);
    const char8 *args = "-i INPUT_FILE -o OUTPUT_FILE -if json|xml|cdb -ov cVariableName";
    int32 nargs = 9u;
    if (argc != nargs) {
        REPORT_ERROR_STATIC(ErrorManagement::ParametersError, "Arguments are %s (%d!=%d)\n", args, argc, nargs);
        return -1;
    }
    StreamString inputFilename;
    StreamString outputFilename;
    StreamString inputFormat;
    StreamString cVariableName;
    bool ok = ParseArgument(argc, argv, "-i", inputFilename);
    if (ok) {
       ok = ParseArgument(argc, argv, "-o", outputFilename);
    }
    if (ok) {
       ok = ParseArgument(argc, argv, "-if", inputFormat);
    }
    if (ok) {
       ok = ParseArgument(argc, argv, "-ov", cVariableName);
    }

    File inputFile;
    if (ok) {
        ok = inputFile.Open(inputFilename.Buffer(), BasicFile::ACCESS_MODE_R);
        if (!ok) {
            REPORT_ERROR_STATIC(ErrorManagement::OSError, "Failed to open file %s\n", inputFilename.Buffer());
        }
    }
    if (ok) {
        ok = inputFile.Seek(0);
    }
    ConfigurationDatabase parsedConfiguration;
    if (ok) {
        StreamString parserError;
        if (inputFormat == "xml") {
            XMLParser parser(inputFile, parsedConfiguration, &parserError);
            ok = parser.Parse();
        }
        else if (inputFormat == "json") {
            JsonParser parser(inputFile, parsedConfiguration, &parserError);
            ok = parser.Parse();
        }
        else if (inputFormat == "cdb") {
            StandardParser parser(inputFile, parsedConfiguration, &parserError);
            ok = parser.Parse();
        }
        else {
            REPORT_ERROR_STATIC(ErrorManagement::ParametersError, "Unknown input format specified");
        }
        if (!ok) {
            StreamString errPrint;
            (void) errPrint.Printf("Failed to parse %s", parserError.Buffer());
            REPORT_ERROR_STATIC(ErrorManagement::ParametersError, errPrint.Buffer());
        }
    }
    if (ok) {
        ok = inputFile.Close();
    }
    if (ok) {
        ok = parsedConfiguration.MoveToRoot();
    }
    if (ok) {
    }
    File outputFile;
    if (ok) {
        Directory d(outputFilename.Buffer());
        d.Delete();

        ok = outputFile.Open(outputFilename.Buffer(), BasicFile::ACCESS_MODE_W | BasicFile::FLAG_CREAT);
        if (!ok) {
            REPORT_ERROR_STATIC(ErrorManagement::OSError, "Failed to open file %s\n", outputFilename.Buffer());
        }
    }
    StreamString cfgAsString;
    StreamString output;
    if (ok) {
        cfgAsString.Printf("%!", parsedConfiguration);
        cfgAsString.Seek(0LLU);
        StreamString token;
        char8 term;
        while(cfgAsString.GetToken(token, "\"", term)) {
            if (token.Size() > 0LLU) {
                output += token;
                if (term == '\"') {
                    output += "\\\"";
                }
            }
            token = "";
        }
    }
    if (ok) {
        output.Seek(0LLU);
        StreamString line;
        ok = outputFile.Printf("const char * %s = \"\"\n", cVariableName.Buffer());
        while(output.GetLine(line) && ok) {
            ok = outputFile.Printf("\"%s\\n\"\n", line.Buffer());
            line = "";
        }
        if (ok) {
            outputFile.Printf("%s", ";\n");
        }
        if (ok) {
            ok = outputFile.Flush();
        }
    }
    if (ok) {
        ok = outputFile.Close();
    }
    int32 ret = ok ? 0 : -1;
    return ret;
}

