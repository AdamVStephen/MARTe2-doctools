/**
 * @file CfgToDot.cpp
 * @brief Source file for main file CfgToDot
 * @date 25/05/2017
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
 * This tool allows to export the links between several components of a MARTe2
 * RealTimeApplication into several Graphviz dot files.
 */

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "AdvancedErrorManagement.h"
#include "ClassRegistryDatabase.h"
#include "ClassRegistryItem.h"
#include "ConfigurationDatabase.h"
#include "Directory.h"
#include "File.h"
#include "GlobalObjectsDatabase.h"
#include "Object.h"
#include "ObjectRegistryDatabase.h"
#include "ProcessorType.h"
#include "RealTimeApplication.h"
#include "Reference.h"
#include "ReferenceT.h"
#include "StreamString.h"
#include "StandardParser.h"
#include "StateMachine.h"
#include "StaticList.h"

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

/**
 * Wraps the concept of a MARTe2 RealTimeState
 */
class GraphvizState : public ReferenceContainer {
public:
    CLASS_REGISTER_DECLARATION()
    GraphvizState() : ReferenceContainer() {
    }
    virtual ~GraphvizState() {
    }
private:
    
};
CLASS_REGISTER(GraphvizState, "");

/**
 * Wraps the concept of a MARTe2 RealTimeThread
 */
class GraphvizThread : public ReferenceContainer {
public:
    CLASS_REGISTER_DECLARATION()
    GraphvizThread() : ReferenceContainer() {
    }
    virtual ~GraphvizThread() {
    }
private:
    
};
CLASS_REGISTER(GraphvizThread, "");

/**
 * Wraps the concept of a MARTe2 DataSource
 */
class GraphvizDataSource : public Object {
public:
    CLASS_REGISTER_DECLARATION()
    GraphvizDataSource() : Object() {
    }
    virtual ~GraphvizDataSource() {
    }

    void SetClassName(StreamString classNameIn) {
        className = classNameIn;
    }

    StreamString GetClassName() const {
        return className;
    }

private:
    StreamString className;    
};
CLASS_REGISTER(GraphvizDataSource, "");


/**
 * Wraps the concept of a MARTe2 Function
 */
class GraphvizFunction : public Object {
public:
    CLASS_REGISTER_DECLARATION()
    GraphvizFunction() : Object() {
        inputDataSources = Reference(new ReferenceContainer());
        outputDataSources = Reference(new ReferenceContainer());
    }
    virtual ~GraphvizFunction() {
    }

    void SetQualifiedName(StreamString qualifiedNameIn) {
        qualifiedName = qualifiedNameIn;
    }

    StreamString GetQualifiedName() const {
        return qualifiedName;
    }

    void SetClassName(StreamString classNameIn) {
        className = classNameIn;
    }

    StreamString GetClassName() const {
        return className;
    }

    void AddInputDataSource(ReferenceT<GraphvizDataSource> dataSource) {
        uint32 i;
        bool found = false;
        StreamString dataSourceToAddName = dataSource->GetName();
        for (i=0; (i<inputDataSources->Size()) && (!found); i++) {
            StreamString dataSourceName = inputDataSources->Get(i)->GetName();
            found = (dataSourceToAddName == dataSourceName);
        }
        if (!found) {
            inputDataSources->Insert(dataSource);
        }
    }

    void AddOutputDataSource(ReferenceT<GraphvizDataSource> dataSource) {
        uint32 i;
        bool found = false;
        StreamString dataSourceToAddName = dataSource->GetName();
        for (i=0; (i<outputDataSources->Size()) && (!found); i++) {
            StreamString dataSourceName = outputDataSources->Get(i)->GetName();
            found = (dataSourceToAddName == dataSourceName);
        }
        if (!found) {
            outputDataSources->Insert(dataSource);
        }
    }

    ReferenceT<ReferenceContainer> GetInputDataSources() {
        return inputDataSources;
    }

    ReferenceT<ReferenceContainer> GetOutputDataSources() {
        return outputDataSources;
    }

private:
    StreamString className;    
    StreamString qualifiedName;    
    ReferenceT<ReferenceContainer> inputDataSources;    
    ReferenceT<ReferenceContainer> outputDataSources;    
};
CLASS_REGISTER(GraphvizFunction, "");

/**
 * @brief Recursevily adds a MARTe2 function to the existentFunctions list, keeping the fully qualified function name (i.e. keeping the parents path)
 * @param[out] existentFunctions list where to add the GraphvizFunction to.
 * @param[in] threadI the RealTimeThead which executes this function.
 * @param[in] cdb which must point at the +Functions node when it enters here the first time.
 * @param[out] fullFunctionName the function name including all the path not separated by dots.
 * @param[out] qualifiedFunctionName the function name including all the path separated by dots.
 * @return true if all the movements in the ConfigurationDatabase are valid.
 */
static bool AddFunction(ReferenceT<ReferenceContainer> existentFunctions, ReferenceT<GraphvizThread> threadI, StreamString functionToAdd, ConfigurationDatabase cdb, StreamString fullFunctionName, StreamString qualifiedFunctionName) {
    bool ok = cdb.MoveRelative(functionToAdd.Buffer());
    StreamString className;
    if (ok) {
        ok = cdb.Read("Class", className);
    }
    if (ok) {
        if (className == "ReferenceContainer") {
            uint32 c;
            uint32 numberOfChildren = cdb.GetNumberOfChildren();
            StreamString recursiveFunctionName = fullFunctionName;
            StreamString recursiveQualifiedFunctionName = qualifiedFunctionName;
            for (c=0; c<numberOfChildren; c++) {
                fullFunctionName = recursiveFunctionName;
                qualifiedFunctionName = recursiveQualifiedFunctionName; 
                StreamString nextFunctionName = cdb.GetChildName(c);
                if (cdb.MoveRelative(nextFunctionName.Buffer())) {
                    if(fullFunctionName.Size() > 0) {
                        fullFunctionName.Printf(".", voidAnyType);
                        qualifiedFunctionName.Printf(".", voidAnyType);
                    }
                    functionToAdd.Seek(1);
                    fullFunctionName.Printf("%s", functionToAdd);
                    qualifiedFunctionName.Printf("%s", functionToAdd.Buffer());
                    //The recursion will make this move again
                    cdb.MoveToAncestor(1);
                    ok = AddFunction(existentFunctions, threadI, nextFunctionName, cdb, fullFunctionName, qualifiedFunctionName);
                }
            } 
        }
        else {
            //Check if the function already exists in the existentFunctions
            uint32 f;
            bool found = false;
            for (f=0; (f<existentFunctions->Size()) && (!found); f++) {
                ReferenceT<GraphvizFunction> function = existentFunctions->Get(f);
                found = (functionToAdd == function->GetName());
            }
            if (!found) {
                ReferenceT<GraphvizFunction> function = Reference(new GraphvizFunction()); 
                StreamString thisFunctionName = cdb.GetName();
                thisFunctionName.Seek(1);
                if(fullFunctionName.Size() > 0) {
                    fullFunctionName.Printf(".", voidAnyType);
                    qualifiedFunctionName.Printf(".", voidAnyType);
                }
                fullFunctionName.Printf("%s", thisFunctionName);
                qualifiedFunctionName.Printf("%s", thisFunctionName.Buffer());
                function->SetName(fullFunctionName.Buffer());
                function->SetQualifiedName(qualifiedFunctionName);
                if (ok) {
                    function->SetClassName(className);
                    existentFunctions->Insert(function);
                    threadI->Insert(function);
                } 
            }
        }
    }
    return ok;
}

/**
 * @brief Checks which data sources interact with the \a function.
 * @param[in] cdb the ConfigurationDatabase pointing at the RealTimeApplication root.
 * @param[in] dataSourceList list of existent DataSources
 * @param[in] function the Functon to query.
 * @return true if no errors are found while moving to the ConfigurationDatabase subtrees.
 */
static bool AddDataSourcesToFunction (ConfigurationDatabase cdb, ReferenceT<ReferenceContainer> dataSourceList, ReferenceT<GraphvizFunction> function) {
    cdb.MoveRelative("+Functions");
    bool ok = cdb.MoveRelative(function->GetQualifiedName().Buffer());
    if (ok) {
        //Extract from which DataSources this gam reads from/writes to and create as needed
        if(cdb.MoveRelative("InputSignals")) {
            uint32 numberOfInputSignals = cdb.GetNumberOfChildren();
            uint32 n;
            for (n=0; n<numberOfInputSignals; n++) {
                StreamString signalName = cdb.GetChildName(n);
                if (cdb.MoveRelative(signalName.Buffer())) {
                    StreamString dataSourceName;
                    if(cdb.Read("DataSource", dataSourceName)) {
                        //Check if this data source already exists
                        uint32 d;
                        ReferenceT<GraphvizDataSource> dataSource;
                        bool dataSourceExists = false;
                        for (d=0; (d<dataSourceList->Size()) && (!dataSourceExists); d++) {
                            dataSource = dataSourceList->Get(d);
                            StreamString existentDataSourceName = &(dataSource->GetName())[1];
                            dataSourceExists = (existentDataSourceName == dataSourceName);
                        }
                        if (dataSourceExists) {
                            function->AddInputDataSource(dataSource);
                        }
                    }
                    cdb.MoveToAncestor(1);
                }
            }
            cdb.MoveToAncestor(1);
        }
        if(cdb.MoveRelative("OutputSignals")) {
            uint32 numberOfOutputSignals = cdb.GetNumberOfChildren();
            uint32 n;
            for (n=0; n<numberOfOutputSignals; n++) {
                StreamString signalName = cdb.GetChildName(n);
                if (cdb.MoveRelative(signalName.Buffer())) {
                    StreamString dataSourceName;
                    if(cdb.Read("DataSource", dataSourceName)) {
                        //Check if this data source already exists
                        bool dataSourceExists = false;
                        ReferenceT<GraphvizDataSource> dataSource;
                        uint32 d;
                        for (d=0; (d<dataSourceList->Size()) && (!dataSourceExists); d++) {
                            dataSource = dataSourceList->Get(d);
                            StreamString existentDataSourceName = &(dataSource->GetName())[1];
                            dataSourceExists = (existentDataSourceName == dataSourceName);
                        } 
                        if (dataSourceExists) {
                            function->AddOutputDataSource(dataSource);
                        }
                    }
                    cdb.MoveToAncestor(1);
                }
            }
            cdb.MoveToAncestor(1);
        }
    }
    return ok;
}

/**
 * @brief Parses the configuration file from \a inputFilename and adds (and links) all the states to stateList, the functions to functionList and the data sources to the dataSourceList.
 */
static bool ParseConfigurationFile(StreamString inputFilename, ReferenceT<ReferenceContainer> stateList, ReferenceT<ReferenceContainer> functionList, ReferenceT<ReferenceContainer> dataSourceList, ConfigurationDatabase &cdb) {
    BasicFile inputFile;
    bool ok = inputFile.Open(inputFilename.Buffer(), BasicFile::ACCESS_MODE_R);
    if (ok) {
        inputFile.Seek(0);
    }
    else {
        REPORT_ERROR_STATIC(ErrorManagement::OSError, "Failed to open file %s\n", inputFilename.Buffer());
    }
    StreamString err;
    if (ok) {
        StandardParser parser(inputFile, cdb, &err);
        ok = parser.Parse();
    }
    if (!ok) {
        REPORT_ERROR_STATIC(ErrorManagement::FatalError, "Failed to parse %s\n", err.Buffer());
    }
    inputFile.Close();

    //Find the realtime application
    uint32 i;
    bool found = false;
    uint32 numberOfNodesAfterRoot = cdb.GetNumberOfChildren(); 
    for (i=0; (i<numberOfNodesAfterRoot) && (ok) && (!found); i++) {
        ok = cdb.MoveToChild(i);
        StreamString className;
        if (ok) {
            ok = cdb.Read("Class", className);
        }
        if (ok) {
            found = (className == "RealTimeApplication");
            if (!found) {
                ok = cdb.MoveToAncestor(1);
            }
        }
    }
    if (ok) {
        ok = found;
        if (!ok) {
            REPORT_ERROR_STATIC(ErrorManagement::FatalError, "Could not find the RealTimeApplication\n");
        }
    }
 
    ConfigurationDatabase cdbRTApp;
    //Load all the GAMs
    if (ok) {
        cdbRTApp = cdb;
        //Move to the states
        ok = cdb.MoveRelative("+States");
        uint32 numberOfStates = cdb.GetNumberOfChildren();
        
        //For each state get the number of threads
        uint32 s;
        for (s=0; (s<numberOfStates) && (ok); s++) {
            StreamString stateName = cdb.GetChildName(s);
            ConfigurationDatabase cdbState = cdb;
            if (cdb.MoveRelative(stateName.Buffer())) {
                ReferenceT<GraphvizState> state = Reference(new GraphvizState());
                state->SetName(&(stateName.Buffer()[1]));
                stateList->Insert(state);
                uint32 t;
                uint32 numberOfThreads;
                if (ok) {
                    ok = cdb.MoveRelative("+Threads");
                }
                if (ok) {
                    numberOfThreads = cdb.GetNumberOfChildren();
                }
                ConfigurationDatabase cdbThread = cdb;
                for (t=0; (t<numberOfThreads) && (ok); t++) {
                    StreamString threadName = cdb.GetChildName(t);
                    if(cdb.MoveRelative(threadName.Buffer())) { 
                        ReferenceT<GraphvizThread> threadI = Reference(new GraphvizThread()); 
                        threadI->SetName(&(threadName.Buffer()[1]));
                        state->Insert(threadI);
                        //Read all the GAMs that are executed by this thread
                        AnyType functionsType = cdb.GetType("Functions");
                        ok = !functionsType.IsVoid();
                        if (ok) {
                            ok = (functionsType.GetNumberOfDimensions() == 1u);
                            uint32 numberOfFunctions = functionsType.GetNumberOfElements(0u);
                            Vector<StreamString> functions(numberOfFunctions);
                            if (ok) {
                                ok = cdb.Read("Functions", functions);
                            }
                            uint32 f;
                            for (f=0; (f<numberOfFunctions) && (ok); f++) {
                                //Find the class name
                                cdb = cdbRTApp;
                                cdb.MoveRelative("+Functions");
                                StreamString functionName;
                                functionName.Printf("+%s", functions[f].Buffer());
                                ok = AddFunction(functionList, threadI, functionName.Buffer(), cdb, "", "");
                            }
                        }
                    }
                    cdb = cdbThread;
                }
            } 
            cdb = cdbState;
        }
    }
    //Create all the data sources
    cdb = cdbRTApp;
    ok = cdb.MoveRelative("+Data"); 
    uint32 c;
    for (c=0; (c<cdb.GetNumberOfChildren()) && (ok); c++) {
        StreamString dataSourceName = cdb.GetChildName(c);
        if (cdb.MoveRelative(dataSourceName.Buffer())) {
            ReferenceT<GraphvizDataSource> dataSource = Reference(new GraphvizDataSource());
            StreamString className;
            if (ok) {
                ok = cdb.Read("Class", className);
            }
            dataSource->SetName(dataSourceName.Buffer());
            dataSource->SetClassName(className.Buffer());
            dataSourceList->Insert(dataSource);
            cdb.MoveToAncestor(1);
        }
    }
    //Link the data sources to the functions
    uint32 f; 
    for (f=0; (f<functionList->Size()) && (ok); f++) {
        ReferenceT<GraphvizFunction> function = functionList->Get(f);
        ok = AddDataSourcesToFunction(cdbRTApp, dataSourceList, function);
    }
    
    return ok; 
}


//The style to apply to the functions. This could be read from an external configuration file in the future.
#define GRAPHVIZ_FONT_SIZE 12
static void GraphvizFunctionStyle(File &outputFile, const char8 *const functionName, const char8 *const className, const char8 * const style="filled", const char8 *const fillColor="white", const char8 * const color="blue", uint32 fontSize=GRAPHVIZ_FONT_SIZE) {
    outputFile.Printf("[shape=record, style=%s, fillcolor=%s, color=%s,label=<<TABLE border=\"0\" cellborder=\"0\"><TR><TD width=\"60\" height=\"60\"><font point-size=\"%d\">%s <BR/>(%s)</font></TD></TR></TABLE>>]", style, fillColor, color, fontSize, functionName, className);
}

static void GraphvizDataSourceStyle(File &outputFile, const char8 *const dataSourceName, const char8 *const className, const char8 * const style="filled", const char8 *const fillColor="white", const char8 * const color="darkgreen", uint32 fontSize=GRAPHVIZ_FONT_SIZE) {
    outputFile.Printf("[shape=record, style=%s, fillcolor=%s, color=%s,label=<<TABLE border=\"0\" cellborder=\"0\"><TR><TD width=\"60\" height=\"60\"><font point-size=\"%d\">%s <BR/>(%s)</font></TD></TR></TABLE>>]", style, fillColor, color, fontSize, dataSourceName, className);
}

static void GraphvizStateMachineStyle(File &outputFile, const char8 *const stateName, uint32 numberOfEnterActions, const char8* const actionList, const char8 * const style="filled", const char8 *const fillColor="white", const char8 * const color="red", uint32 fontSize=GRAPHVIZ_FONT_SIZE) {
    if (numberOfEnterActions == 0) {
        outputFile.Printf("[style=%s, fillcolor=%s, color=%s,label=<<TABLE border=\"0\" cellborder=\"0\"><TR><TD width=\"60\" height=\"60\"><font point-size=\"%d\">%s</font></TD></TR></TABLE>>]", style, fillColor, color, fontSize, stateName);
    }
    else {
        outputFile.Printf("[style=%s, fillcolor=%s, color=%s,label=<<TABLE border=\"0\" cellborder=\"0\"><TR><TD width=\"60\" height=\"60\"><font point-size=\"%d\">%s</font></TD></TR><TR><TD><font point-size=\"%d\"> / ENTER </font></TD></TR><TR><TD><font point-size=\"%d\">%s</font></TD></TR></TABLE>>]", style, fillColor, color, fontSize, stateName, fontSize, fontSize, actionList);
    }
}

static void GraphvizObjectStyle(File &outputFile, const char8 *const objName, const char8 *const className, const char8 * const style="filled", const char8 *const fillColor="white", const char8 * const color="black", uint32 fontSize=GRAPHVIZ_FONT_SIZE) {
    outputFile.Printf("[shape=record, style=%s, fillcolor=%s, color=%s,label=<<TABLE border=\"0\" cellborder=\"0\"><TR><TD width=\"60\" height=\"60\"><font point-size=\"%d\">%s <BR/>(%s)</font></TD></TR></TABLE>>]", style, fillColor, color, fontSize, objName, className);
}

/**
 * @brief Lists in Graphviz format all the functions that belong to a given state in the configuration file.
 */
static bool ListFunctionsGraph(File &outputFile, ReferenceT<GraphvizState> state) {
    uint32 t;
    bool ok = state.IsValid();
    for (t=0; (t<state->Size()) && (ok); t++) {
        ReferenceT<GraphvizThread> threadI = state->Get(t);
        uint32 f;
        ok = threadI.IsValid(); 
        for (f=0; (f<threadI->Size()) && (ok); f++) {
            ReferenceT<GraphvizFunction> function = threadI->Get(f);
            StreamString stateName = state->GetName();
            StreamString threadName = threadI->GetName();
            StreamString uniqueFunctionName;
            uniqueFunctionName.Printf("\"%s.%s.%s\"", stateName.Buffer(), threadName.Buffer(), function->GetName());
            ok = outputFile.Printf("%s ", uniqueFunctionName.Buffer());
            GraphvizFunctionStyle(outputFile, function->GetName(), function->GetClassName().Buffer());
            ok &= outputFile.Printf("\n", voidAnyType);
        }
    }
    return ok;
}

/**
 * @brief Creates a Graphviz cluster with all the states, threads and functions with-in.
 */
static bool CreateStateClusterGraph(File &outputFile, ReferenceT<GraphvizState> state) {
    bool ok = outputFile.Printf("subgraph cluster_%s {\n", state->GetName());
    if (ok) {
        ok = outputFile.Printf("label = \"State: %s\"\n", state->GetName());
    }
    if (ok) {
        ok = state.IsValid();
    }
    uint32 t;
    for (t=0; (t<state->Size()) && (ok); t++) {
        ReferenceT<GraphvizThread> threadI = state->Get(t);
        ok = outputFile.Printf("subgraph cluster_%s_%s {\n", state->GetName(), threadI->GetName());
        ok &= outputFile.Printf("label = \"Thread: %s\"\n", threadI->GetName());
        ok &= outputFile.Printf("color= \"%s\"\n", "red");
        StreamString executionList;
        uint32 f; 
        for (f=0; (f<threadI->Size()) && (ok); f++) {
            ReferenceT<GraphvizFunction> function = threadI->Get(f);
            ok = function.IsValid();
            if (ok) {
                StreamString stateName = state->GetName();
                StreamString threadName = threadI->GetName();
                StreamString uniqueFunctionName;
                ok = uniqueFunctionName.Printf("\"%s.%s.%s\"", stateName.Buffer(), threadName.Buffer(), function->GetName());
                if(executionList.Size() != 0) {
                    ok &= executionList.Printf("->", voidAnyType);
                }
                ok &= executionList.Printf("%s", uniqueFunctionName.Buffer());
            }
        }
        ok &= outputFile.Printf("%s\n", executionList.Buffer());
        ok &= outputFile.Printf("%s", "}\n");
    }
    if (ok) {
        ok = outputFile.Printf("%s", "}\n");
    }
    return ok;
}

/**
 * @brief Lists in Graphviz format all the data sources from the configuration file.
 */
static bool ListDataSourcesGraph(File &outputFile, ReferenceT<ReferenceContainer> dataSourceList) {
    //Create the dataSource clusters
    bool ok = true;
    uint32 s;
    for (s=0; (s<dataSourceList->Size()) && (ok); s++) {
        ReferenceT<GraphvizDataSource> dataSource = dataSourceList->Get(s);
        ok = dataSource.IsValid();
        ok &= outputFile.Printf("\"%s\" ", dataSource->GetName());
        GraphvizDataSourceStyle(outputFile, dataSource->GetName(), dataSource->GetClassName().Buffer());
        ok &= outputFile.Printf("\n", voidAnyType);
    }
    return ok;
}

/**
 * @brief Exports all the states, threads with-in states and functions in a graph file named %sRTApp.gv (outputFilenamePrefix.Buffer())
 */
static bool ExportRTAppGraph(StreamString outputFilenamePrefix, ReferenceT<ReferenceContainer> stateList, ReferenceT<ReferenceContainer> functionList, ReferenceT<ReferenceContainer> dataSourceList) {
    StreamString outputFilename;
    outputFilename.Printf("%sRTApp.gv", outputFilenamePrefix.Buffer());
    //Delete any existent output file
    Directory d(outputFilename.Buffer());
    d.Delete();
    File outputFile;
    bool ok = outputFile.Open(outputFilename.Buffer(), BasicFile::ACCESS_MODE_W | BasicFile::FLAG_CREAT);
    if (ok) {
        outputFile.Seek(0);
    }
    else {
        REPORT_ERROR_STATIC(ErrorManagement::FatalError,"Failed to open file %s\n", outputFilename.Buffer());
    }

    if (ok) {
        outputFile.Printf("%s", "digraph G {\n");
        outputFile.Printf("%s", "rankdir=LR\n");
        outputFile.Printf("%s", "concentrate=true\n");
    }
   
    //List all the functions
    uint32 s; 
    for (s=0; (s<stateList->Size()) && (ok); s++) {
        ReferenceT<GraphvizState> state = stateList->Get(s);
        ok = ListFunctionsGraph(outputFile, state); 
    }
    //Create the state/thread clusters
    for (s=0; (s<stateList->Size()) && (ok); s++) {
        ReferenceT<GraphvizState> state = stateList->Get(s);
        ok = CreateStateClusterGraph(outputFile, state);
    }
    if (ok) {
        ok = outputFile.Printf("subgraph cluster_DataSources {\n", voidAnyType);
        ok &= outputFile.Printf("label = \"Data Sources\"\n", voidAnyType);
        ok &= ListDataSourcesGraph(outputFile, dataSourceList);
        ok &= outputFile.Printf("%s", "}\n");
    }
    if (ok) {
        outputFile.Printf("%s", "}\n");
    }
    outputFile.Flush();
    outputFile.Close();
    return ok;
}

/**
 * @brief For a given state, connects the functions of this state to the respective data sources.
 */
static bool ConnectFunctionsToDataSources (File &outputFile, ReferenceT<GraphvizState> state, ReferenceT<ReferenceContainer> connectedDataSources) {
    bool ok = true;
    uint32 t; 
    for (t=0; (t<state->Size()) && (ok); t++) {
        ReferenceT<GraphvizThread> threadI = state->Get(t);
        ok = threadI.IsValid();
        uint32 f;
        for (f=0; (f<threadI->Size()) && (ok); f++) {
            ReferenceT<GraphvizFunction> function = threadI->Get(f);
            StreamString stateName = state->GetName();
            StreamString threadName = threadI->GetName();
            StreamString uniqueFunctionName;
            uniqueFunctionName.Printf("\"%s.%s.%s\"", stateName.Buffer(), threadName.Buffer(), function->GetName());
            ReferenceT<ReferenceContainer> inputs = function->GetInputDataSources();
            ReferenceT<ReferenceContainer> outputs = function->GetOutputDataSources();
            uint32 i;
            for (i=0; i<inputs->Size(); i++) {
                outputFile.Printf("\"%s\"->%s\n", inputs->Get(i)->GetName(), uniqueFunctionName.Buffer());
                connectedDataSources->Insert(inputs->Get(i));
            }
            for (i=0; i<outputs->Size(); i++) {
                outputFile.Printf("%s->\"%s\"\n", uniqueFunctionName.Buffer(), outputs->Get(i)->GetName());
                connectedDataSources->Insert(outputs->Get(i));
            }
        }
    }
    return ok;
}

/**
 * @brief Creates one file for each state, named %sState%s.gv (outputFilenamePrefix.Buffer(), state->GetName()) and adds the connections between the functions belonging to these states and the data sources.
 */
static bool ExportRTStatesGraph(StreamString outputFilenamePrefix, ReferenceT<ReferenceContainer> stateList, ReferenceT<ReferenceContainer> functionList, ReferenceT<ReferenceContainer> dataSourceList) {
    bool ok = true; 
    //For each state
    uint32 s; 
    for (s=0; (s<stateList->Size()) && (ok); s++) {
        ReferenceT<GraphvizState> state = stateList->Get(s);
        ok = state.IsValid();
        StreamString outputFilename;
        outputFilename.Printf("%sState%s.gv", outputFilenamePrefix.Buffer(), state->GetName());
        //Delete any existent output file
        Directory d(outputFilename.Buffer());
        d.Delete();
        File outputFile;
        if (ok) {
            ok = outputFile.Open(outputFilename.Buffer(), BasicFile::ACCESS_MODE_W | BasicFile::FLAG_CREAT);
        }
        if (ok) {
            outputFile.Seek(0);
        }
        else {
            REPORT_ERROR_STATIC(ErrorManagement::FatalError, "Failed to open file %s\n", outputFilename.Buffer());
        }

        if (ok) {
            outputFile.Printf("%s", "digraph G {\n");
            outputFile.Printf("%s", "rankdir=LR\n");
            outputFile.Printf("%s", "concentrate=true\n");
        }
        if (ok) {
            ok = ListFunctionsGraph(outputFile, state); 
        }
        if (ok) {
            ok = CreateStateClusterGraph(outputFile, state);
        }
        ReferenceT<ReferenceContainer> connectedDataSources = Reference(new ReferenceContainer()); 
        if (ok) {
            ok = ConnectFunctionsToDataSources(outputFile, state, connectedDataSources);
        }
        if (ok) {
            ok = ListDataSourcesGraph(outputFile, connectedDataSources);
        }

        outputFile.Printf("%s", "}\n");
        outputFile.Flush();
        outputFile.Close();
    }
    return ok;
}

/**
 * @brief Exports a MARTe2 state machine in a graph file named %sStateMachine.gv (outputFilenamePrefix.Buffer())
 */
bool ExportStateMachine(StreamString outputFilenamePrefix, ConfigurationDatabase &cdb) {
    //Look for a StateMachine
    uint32 i;
    bool found = false;
    bool ok = cdb.MoveToRoot();
    uint32 numberOfNodesAfterRoot = cdb.GetNumberOfChildren(); 
    for (i=0; (i<numberOfNodesAfterRoot) && (ok) && (!found); i++) {
        ok = cdb.MoveToChild(i);
        StreamString className;
        if (ok) {
            ok = cdb.Read("Class", className);
        }
        if (ok) {
            found = (className == "StateMachine");
            if (!found) {
                ok = cdb.MoveToAncestor(1);
            }
        }
    }
    bool stateMachineExists = found;
    if (stateMachineExists) {
        if (ok) {
            ok = found;
            if (!ok) {
                REPORT_ERROR_STATIC(ErrorManagement::FatalError, "Could not find the StateMachine\n");
            }
        }
        ReferenceT<StateMachine> stateMachine;
        if (ok) {
            stateMachine = Reference(new StateMachine());
            ok = stateMachine->Initialise(cdb);
            if (!ok) {
                REPORT_ERROR_STATIC(ErrorManagement::FatalError, "Failed to initialise the StateMachine\n");
            }
        }

        StreamString outputFilename;
        if (ok) {
            outputFilename.Printf("%sStateMachine.gv", outputFilenamePrefix.Buffer());
            //Delete any existent output file
            Directory d(outputFilename.Buffer());
            d.Delete();
        }
        File outputFile;
        if (ok) {
            ok = outputFile.Open(outputFilename.Buffer(), BasicFile::ACCESS_MODE_W | BasicFile::FLAG_CREAT);
            if (!ok) {
                REPORT_ERROR_STATIC(ErrorManagement::FatalError, "Failed to open file %s\n", outputFilename.Buffer());
            }
        }
        if (ok) {
            outputFile.Seek(0);
        }
        if (ok) {
            outputFile.Printf("%s", "digraph G {\n");
            outputFile.Printf("%s", "rankdir=TD\n");
            outputFile.Printf("%d", "nodesep=2.5\n");
        }
        uint32 numberOfStates = 0;
        uint32 s;
        //List all the states
        if (ok) {
            numberOfStates = stateMachine->Size();
            for (s=0; (s<numberOfStates) && (ok); s++) {
                ReferenceT<ReferenceContainer> state = stateMachine->Get(s);
                ok = state.IsValid();
                if (ok) {
                    //Check if the event has an ENTER action list
                    StreamString actionList;
                    uint32 numberOfEvents = state->Size();
                    uint32 numberOfEnterActions = 0u;
                    uint i;
                    for (i=0; (i<numberOfEvents) && (ok); i++) {
                        ReferenceT<ReferenceContainer> event = state->Get(i);
                        if (event.IsValid()) {
                            StreamString evtName = event->GetName();    
                            if (evtName == "ENTER") {
                                numberOfEnterActions = event->Size();
                                uint32 a;
                                for (a=0; a<numberOfEnterActions; a++) {
                                    ok &= actionList.Printf("%d. %s <BR/>", (a + 1), event->Get(a)->GetName());
                                }
                            }
                        }
                    }
                    ok = outputFile.Printf("\"%s\" ", state->GetName());
                    GraphvizStateMachineStyle(outputFile, state->GetName(), numberOfEnterActions, actionList.Buffer());
                    ok &= outputFile.Printf("\n", voidAnyType);
                } 
            }
        }
        //Connect the states
        if (ok) {
            for (s=0; (s<numberOfStates) && (ok); s++) {
                ReferenceT<ReferenceContainer> state = stateMachine->Get(s);
                ok = state.IsValid();
                if (ok) {
                    uint32 numberOfEvents = state->Size();
                    uint i;
                    for (i=0; (i<numberOfEvents) && (ok); i++) {
                        ReferenceT<StateMachineEvent> event = state->Get(i);
                        if (event.IsValid()) {
                            //Get the destination
                            StreamString nextState = event->GetNextState();
                            StreamString nextStateError = event->GetNextStateError();
                            uint32 numberOfActions = event->Size();
                            if (numberOfActions > 0) {
                                ok = outputFile.Printf("\"%s\"->\"%s\" [label= <<TABLE border=\"0\" cellborder=\"0\"><TR><TD ROWSPAN=\"%d\"><font point-size=\"%d\">%s</font></TD>", state->GetName(), nextState.Buffer(), numberOfActions, GRAPHVIZ_FONT_SIZE, event->GetName());
                                ok = outputFile.Printf("<TD ALIGN=\"CENTER\" ROWSPAN=\"%d\"><font point-size=\"%d\"> / </font></TD>", numberOfActions, GRAPHVIZ_FONT_SIZE);
                                ok = outputFile.Printf("<TD ALIGN=\"LEFT\"><font point-size=\"%d\">1. %s </font></TD></TR>", GRAPHVIZ_FONT_SIZE, event->Get(0)->GetName());
                                uint32 a;
                                for (a=1; a<numberOfActions; a++) {
                                    ok &= outputFile.Printf("<TR><TD ALIGN=\"LEFT\"><font point-size=\"%d\">%d. %s </font></TD></TR>", GRAPHVIZ_FONT_SIZE, (a + 1), event->Get(a)->GetName());
                                }
                            }
                            else {
                                ok = outputFile.Printf("\"%s\"->\"%s\" [label= <<TABLE border=\"0\" cellborder=\"0\"><TR><TD><font point-size=\"%d\">%s</font></TD></TR>", state->GetName(), nextState.Buffer(), GRAPHVIZ_FONT_SIZE, event->GetName());
                            }
                            ok = outputFile.Printf("</TABLE>>]\n", voidAnyType);
                        }
                    }
                }
            }
        }
        outputFile.Printf("%s", "}\n");
        outputFile.Flush();
        outputFile.Close();
    } 
    else {
        REPORT_ERROR_STATIC(ErrorManagement::Warning, "No state machine defined");
    }

    return ok;
}

/**
 * @brief Exports all the configuraion file objects in a filed named %sObjects.gv (outputFilenamePrefix.Buffer())
 */
bool ExportObjects(File &outputFile, ConfigurationDatabase &cdb, StreamString uniqueName = "") {
    bool ok = true;
    //Number of children
    uint32 numberOfChildren = cdb.GetNumberOfChildren();
    uint32 i;
    bool isLeaf = true;
    StreamString className;
    if (!cdb.Read("Class", className)) {
        className = "";
    }
    StreamString objName = cdb.GetName();
    if (objName[0] == '+') {
        objName = &(objName.Buffer())[1];
    }
    else if (objName[0] == '$') {
        objName = &(objName.Buffer())[1];
    }
    bool clusterCreated = false;
    uniqueName.Printf("%s", objName.Buffer());
    //Replace - and : with _
    (void)uniqueName.Seek(0LLU);
    StreamString newUniqueName;
    StreamString token;
    char8 term;
    while(uniqueName.GetToken(token, ":-", term)) {
        if (newUniqueName.Size() > 0) {
            newUniqueName += "_";
        }
        newUniqueName += token.Buffer();
        token = "";
    }
    uniqueName = newUniqueName;
    for(i=0; i<numberOfChildren; i++) {
        if (cdb.MoveRelative(cdb.GetChildName(i))) {
            StreamString childClassName;
            if (!cdb.Read("Class", childClassName)) {
                childClassName = "";
            }
            if (childClassName.Size() > 0) {
                isLeaf = false;
                if (!clusterCreated) {
                    if(objName.Size() > 0) {
                        outputFile.Printf("subgraph cluster_%s {\nlabel=<<TABLE border=\"0\" cellborder=\"0\"><TR><TD width=\"60\" height=\"60\"><font point-size=\"%d\">%s <BR/>(%s)</font></TD></TR></TABLE>>\n", uniqueName.Buffer(), GRAPHVIZ_FONT_SIZE, objName.Buffer(), className.Buffer());
                        clusterCreated = true;
                    }
                }
            }
            ExportObjects(outputFile, cdb, uniqueName.Buffer());
            cdb.MoveToAncestor(1);
        }
    }
    if (clusterCreated) {
        outputFile.Printf("}\n", voidAnyType);
    }
    if (isLeaf) {
        if (className.Size() > 0) {
            ok = outputFile.Printf("%s ", uniqueName.Buffer(), objName.Buffer());
            GraphvizObjectStyle(outputFile, objName.Buffer(), className.Buffer()); 
            ok &= outputFile.Printf("\n", voidAnyType);
        }
    }
    return ok;
}

/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/
int main(int argc, char **argv) {
    SetErrorProcessFunction(&MainErrorProcessFunction);
    if (argc != 5) {
        REPORT_ERROR_STATIC(ErrorManagement::ParametersError, "Arguments are -i INPUT_FILE -o OUTPUT_FILE_PREFIX\n");
        return -1;
    }
    StreamString argv1 = argv[1];
    StreamString argv3 = argv[3];
    StreamString inputFilename;
    StreamString outputFilenamePrefix;

    if (argv1 == "-i") {
        inputFilename = argv[2];
    }
    else if (argv3 == "-i") {
        inputFilename = argv[4];
    }
    else {
        REPORT_ERROR_STATIC(ErrorManagement::ParametersError, "Arguments are -i INPUT_FILE -o OUTPUT_FILE_PREFIX\n");
        return -1;
    }

    if (argv1 == "-o") {
        outputFilenamePrefix = argv[2];
    }
    else if (argv3 == "-o") {
        outputFilenamePrefix = argv[4];
    }
    else {
        REPORT_ERROR_STATIC(ErrorManagement::ParametersError, "Arguments are -i INPUT_FILE -o OUTPUT_FILE_PREFIX\n");
        return -1;
    }
    ReferenceT<ReferenceContainer> stateList = Reference(new ReferenceContainer()); 
    ReferenceT<ReferenceContainer> functionList = Reference(new ReferenceContainer()); 
    ReferenceT<ReferenceContainer> dataSourceList = Reference(new ReferenceContainer()); 
    ConfigurationDatabase cdb; 
    bool ok = ParseConfigurationFile(inputFilename, stateList, functionList, dataSourceList, cdb);
    if (ok) {
        ok = ExportRTAppGraph(outputFilenamePrefix, stateList, functionList, dataSourceList);
    }
    if (ok) {
        ok = ExportRTStatesGraph(outputFilenamePrefix, stateList, functionList, dataSourceList);
    }
    if (ok) {
        ok = ExportStateMachine(outputFilenamePrefix, cdb);
    }
    if (ok) {
        cdb.MoveToRoot();
        uint32 numberOfNodesAfterRoot = cdb.GetNumberOfChildren();
        //Generate one Objects file for each sub-root node, otherwise it is a mess!
        uint32 i;
        for (i=0; i<numberOfNodesAfterRoot; i++) {
            cdb.MoveToRoot();
            cdb.MoveToChild(i);
            StreamString outputFilename;
            if (ok) {
                outputFilename.Printf("%sObjects_%d.gv", outputFilenamePrefix.Buffer(), i);
                //Delete any existent output file
                Directory d(outputFilename.Buffer());
                d.Delete();
            }
            File outputFile;
            if (ok) {
                ok = outputFile.Open(outputFilename.Buffer(), BasicFile::ACCESS_MODE_W | BasicFile::FLAG_CREAT);
            }
            if (ok) {
                outputFile.Seek(0);
            }
            else {
                REPORT_ERROR_STATIC(ErrorManagement::ParametersError, "Failed to open file %s\n", outputFilename.Buffer());
            }
            if (ok) {
                outputFile.Printf("%s", "digraph G {\n");
                outputFile.Printf("%s", "bgcolor=white\n");
            }
            ok = ExportObjects(outputFile, cdb);
            outputFile.Printf("%s", "}\n");
            outputFile.Flush();
            outputFile.Close();
        }
    }
    return 0;
}

