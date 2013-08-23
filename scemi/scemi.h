/*
 *
 * (c) 2004, 2005 Cadence Design Systems, Inc. All rights reserved.
 * This work may not be copied, modified, re-published, uploaded, executed, 
 * or distributed in any way, in any medium, whether in whole or in part, 
 * without prior written permission from Cadence Design Systems, Inc. 
 *
 * Main include file for SCE-MI.
 *
 * Filename: scemi.h
 *
*/

#ifndef SCEMI_H
#define SCEMI_H 1

#define SCEMI_MAJOR_VERSION 2
#define SCEMI_MINOR_VERSION 0 
#define SCEMI_PATCH_VERSION 0
#define SCEMI_VERSION_STRING "2.0.0"

typedef unsigned int SceMiU32;
typedef unsigned long long SceMiU64;

// G function pointer
typedef int (*SceMiServiceLoopHandler)(void *context, bool pending);

// Init Done Handler (Cadence extention)
typedef void (*SceMiInitDoneHandler)(void *context);


// Error Type and Class
typedef enum
{
    SceMiOK,
    SceMiError
} SceMiErrorType;

typedef struct
{
    const char *Culprit;
    const char *Message;
    SceMiErrorType Type;
    int Id;
} SceMiEC;

typedef void (*SceMiErrorHandler)(void *context, SceMiEC *ec);


// Info Type and Class
typedef enum
{
    SceMiInfo,
    SceMiWarning,
    SceMiNonFatalError
} SceMiInfoType;

typedef struct
{
    const char *Originator;
    const char *Message;
    SceMiInfoType Type;
    int Id;
} SceMiIC;

typedef void (*SceMiInfoHandler)(void *context, SceMiIC *ic);

typedef struct
{
    void *Context;
    void (*IsReady)(void *context);
    void (*Close)(void *context);
} SceMiMessageInPortBinding;

#ifdef __cplusplus

// Forward Declarations
class SceMiMessageData;

typedef struct
{
    void *Context;
    void (*Receive) (void *context, const SceMiMessageData *data);
    void (*Close) (void *context);
} SceMiMessageOutPortBinding;


namespace CDS_SceMi
{
    class ParametersDB;
    class MessageInPortBinding;
    class MessageOutPortBinding;
    class MessagePortBinding;
    class MessageRep;
    class MessagePortProxy;
    class InternalSceMi;
};

class SceMiParameters
{
public:
//+    SceMiParameters(const char *paramFile,
//+		    SceMiEC *ec = 0);

//+    ~SceMiParameters();

//+    unsigned int NumberOfObjects(const char *objectKind,
//+				 SceMiEC *ec = 0) const;

//+    int AttributeIntegerValue(const char *objectKind,
//+			      unsigned int index,
//+			      const char *attributeName,
//+			      SceMiEC *ec = 0) const;

//+    const char *AttributeStringValue(const char *objectKind,
//+				     unsigned int index,
//+				     const char *attributeName,
//+				     SceMiEC *ec = 0) const;

//+    void OverrideAttributeIntegerValue(const char *objectKind,
//+				       unsigned int index,
//+				       const char *attributeName,
//+				       int value,
//+				       SceMiEC *ec = 0);

//+    void OverrideAttributeStringValue(const char *objectKind,
//+				      unsigned int index,
//+				      const char *attributeName,
//+				      const char *value,
//+				      SceMiEC *ec = 0);
private:
//+    friend class SceMi;
};


class SceMiMessageInPortProxy
{
public:
//+    const char* TransactorName() const;
//+    const char* PortName() const;
//+    unsigned int PortWidth() const;

//+    void Send(const SceMiMessageData &data,
//+	      SceMiEC *ec = 0);

//+    void ReplaceBinding(const SceMiMessageInPortBinding *binding = 0,
//+			SceMiEC *ec = 0);

private:
//+    friend class SceMi;
//+    friend class SceMiMessageData;
//+    friend class CDS_SceMi::InternalSceMi;


//+    SceMiMessageInPortProxy(const char *TransactorName,
//+			    const char *PortName,
//+			    const unsigned int portWidth,
//+			    unsigned int PortIndex,
//+			    SceMiEC * ec);

//+    ~SceMiMessageInPortProxy();


//+    SceMiMessageInPortProxy();
//+    SceMiMessageInPortProxy(const SceMiMessageInPortProxy &);
//+    SceMiMessageInPortProxy &operator=(const SceMiMessageInPortProxy &);
//+    CDS_SceMi::MessagePortProxy * _proxy;
};

class SceMiMessageOutPortProxy
{
public:
//+    const char* TransactorName() const;
//+    const char* PortName() const;
//+    unsigned int PortWidth() const;

//+    void ReplaceBinding(const SceMiMessageOutPortBinding *binding,
//+			SceMiEC *ec = 0);

private:
//+    friend class SceMi;
//+    friend class SceMiMessageData;
//+    friend class CDS_SceMi::InternalSceMi;

//+    SceMiMessageOutPortProxy(const char *TransactorName,
//+			     const char *PortName,
//+			     const unsigned int PortWidth,
//+			     unsigned int PortIndex,
//+			     SceMiEC * ec);
//+    ~SceMiMessageOutPortProxy();


//+    SceMiMessageOutPortProxy();
//+    SceMiMessageOutPortProxy(const SceMiMessageOutPortProxy &);
//+    SceMiMessageOutPortProxy &operator=(const SceMiMessageOutPortProxy &);
//+    CDS_SceMi::MessagePortProxy * _proxy;
};

class SceMiMessageData
{
public:
//+    SceMiMessageData(const SceMiMessageInPortProxy& messageInPortProxy,
//+		     SceMiEC* ec = 0);

//+    ~SceMiMessageData();

//+    // Return size of vector in bits
//+    unsigned int WidthInBits() const;

//+    // Return size of array in 32 bit words
//+    unsigned int WidthInWords() const;

//+    // Set the value of a given word of the array
//+    void Set (unsigned int i,
//+	      SceMiU32 word,
//+	      SceMiEC* ec = 0);

//+    // Set the value of a given bit of a given word of the array
//+    void SetBit (unsigned int i,
//+		 int bit,
//+		 SceMiEC* ec = 0);

//+    void SetBitRange(unsigned int i,
//+		     unsigned int range,
//+		     SceMiU32 bits,
//+		     SceMiEC* ec = 0);

//+    SceMiU32 Get (unsigned int i,
//+		  SceMiEC* ec = 0) const;

//+    int GetBit (unsigned int i,
//+		SceMiEC* ec = 0) const;

//+    SceMiU32 GetBitRange(unsigned int i,
//+			 unsigned int range,
//+			 SceMiEC* ec = 0) const;

//+    SceMiU64 CycleStamp() const;
private:
//+    friend class CDS_SceMi::MessagePortBinding;
//+    friend class CDS_SceMi::MessageOutPortBinding;
//+    friend class CDS_SceMi::MessageInPortBinding;

//+    SceMiMessageData(const SceMiMessageOutPortProxy * messageOutPortProxy,
//+		     SceMiEC* ec = 0);

//+    CDS_SceMi::MessageRep * messageRep;
};

class SceMi
{
public:

    static int
    Version(const char* versionString);

    static SceMi*
    Init(int version,
	 const SceMiParameters * parameters,
	 SceMiEC* ec = 0);

//+    static SceMi*
//+    Init(int version,
//+	 const SceMiParameters * parameters,
//+	 SceMiInitDoneHandler initDone,
//+	 void *initDoneContext = 0,
//+	 SceMiEC* ec = 0);

//+    static void
//+    Shutdown(SceMi* mct,
//+	     SceMiEC* ec = 0);

//+    SceMiMessageInPortProxy *
//+    BindMessageInPort(const char *transactorName,
//+		      const char *portName,
//+		      const SceMiMessageInPortBinding *binding = 0,
//+		      SceMiEC *ec = 0);

//+    SceMiMessageOutPortProxy *
//+    BindMessageOutPort(const char *transactorName,
//+		       const char *portName,
//+		       const SceMiMessageOutPortBinding *binding = 0,
//+		       SceMiEC *ec = 0);

//+    int
//+    ServiceLoop(SceMiServiceLoopHandler g = 0,
//+		void *context = 0,
//+		SceMiEC *ec = 0);

    static void
    RegisterErrorHandler(SceMiErrorHandler errorHandler,
			 void *context);


//+    static void
//+    RegisterInfoHandler(SceMiInfoHandler infoHandler,
//+			void *context);

//+    void
//+    CDSRegisterServiceRequestCB(void (*SRCB)(void *Context),
//+				void *Context = 0);

//+    static SceMi *
//+    Pointer(SceMiEC * ec = 0);

private:
//+    SceMi();
//+    ~SceMi();
//+    SceMi(const SceMi &);
//+    SceMi &operator=(const SceMi &);
//+    static SceMi *singleSceMi;
};

extern "C" void
tba_profile(void);

#else

#error "SCEMI C API interface is not yet supported"

//+typedef unsigned SceMiU32;
//+typedef unsigned long long SceMiU64;
//+typedef void SceMi;
//+typedef void SceMiParameters;
//+typedef void SceMiMessageData;
//+typedef void SceMiMessageInPortProxy;
//+typedef void SceMiMessageOutPortProxy;

//+/* SCE-MI API */
//+void SceMiRegisterErrorHandler(SceMiErrorHandler errorHandler, void *context);

//+void SceMiRegisterInfoHandler(SceMiInfoHandler infoHandler, void *context);

//+int SceMiVersion(const char *versionString);

//+SceMi *SceMiInit(int version, SceMiParameters *parameterObjectHandle, SceMiEC *ec);

//+SceMi *SceMiInitWithCallback(int version, SceMiParameters *parameterObjectHandle, SceMiInitDoneHandler initDone, void *context, SceMiEC *ec);

//+void SceMiShutdown(SceMi *sceMiHandle, SceMiEC *ec);

//+SceMiMessageInPortProxy *SceMiBindMessageInPort(SceMi *sceMiHandle, 
//+						const char *transactorName, 
//+						const char *portName, 
//+						const SceMiMessageInPortBinding *binding,
//+						SceMiEC *ec);
//+SceMiMessageOutPortProxy *SceMiBindMessageOutPort(SceMi *sceMiHandle, 
//+						  const char *transactorName, 
//+						  const char *portName, 
//+						  const SceMiMessageOutPortBinding *binding,
//+						  SceMiEC *ec);
//+int SceMiServiceLoop(SceMi *sceMiHandle,
//+		     SceMiServiceLoopHandler g,
//+		     void *context,
//+		     SceMiEC *ec);

//+/* Paramters API */
//+SceMiParameters *SceMiParametersNew(const char *paramsFile,
//+				    SceMiEC *ec);

//+void SceMiParametersDelete(SceMiParameters *parametersHandle);

//+unsigned int SceMiParametersNumberOfObjects(const SceMiParameters *parametersHandle,
//+					    const char *objectKind,
//+					    SceMiEC *ec);

//+int SceMiParametersAttributeIntegerValue(const SceMiParameters *parametersHandle,
//+					 const char *objectKind,
//+					 unsigned int index,
//+					 const char *attributeName,
//+					 SceMiEC *ec);

//+const char *SceMiParametersAttributeStringValue(const SceMiParameters *parametersHandle,
//+						const char *objectKind,
//+						unsigned int index,
//+						const char *attributeName,
//+						SceMiEC *ec);

//+void SceMiParametersOverrideAttributeIntegerValue(const SceMiParameters *parametersHandle,
//+						  const char *objectKind,
//+						  unsigned int index,
//+						  const char *attributeName,
//+						  int value,
//+						  SceMiEC *ec);

//+void SceMiParametersOverrideAttributeStringValue(const SceMiParameters *parametersHandle,
//+						 const char *objectKind,
//+						 unsigned int index,
//+						 const char *attributeName,
//+						 const char *value,
//+						 SceMiEC *ec);

//+/* Message Data API */
//+SceMiMessageData *SceMiMessageDataNew(SceMiMessageInPortProxy *messageInPortProxyHandle,
//+				      SceMiEC *ec);
//+void SceMiMessageDataDelete(SceMiMessageData *messageDataHandle);
//+unsigned int SceMiMessageDataWidthInBits(const SceMiMessageData *messageDataHandle);
//+unsigned int SceMiMessageDataWidthInWords(const SceMiMessageData *messageDataHandle);
//+void SceMiMessageDataSet(SceMiMessageData *messageDataHandle,
//+			 unsigned int i,
//+			 SceMiU32 word,
//+			 SceMiEC *ec);
//+void SceMiMessageDataSetBit(const SceMiMessageData *messageDataHandle,
//+			    unsigned int i,
//+			    int bit,
//+			    SceMiEC *ec);
//+void SceMiMessageDataSetBitRange(const SceMiMessageData *messageDataHandle,
//+				 unsigned int i,
//+				 unsigned int range,
//+				 SceMiU32 bits,
//+				 SceMiEC *ec);
//+SceMiU32 SceMiMessageDataGet(const SceMiMessageData *messageDataHandle,
//+			     unsigned int i,
//+			     SceMiEC *ec);
//+int SceMiMessageDataGetBit(const SceMiMessageData *messageDataHandle,
//+			       unsigned int i,
//+			       SceMiEC *ec);
//+SceMiU32 SceMiMessageDataGetBitRange(const SceMiMessageData *messageDataHandle,
//+				     unsigned int i,
//+				     unsigned int range,
//+				     SceMiEC *ec);
//+SceMiU64 SceMiMessageDataCycleStamp(const SceMiMessageData *messageDataHandle);


//+/* In Proxy API */
//+void SceMiMessageInPortProxySend(SceMiMessageInPortProxy *messageInPortProxyHandle,
//+				 const SceMiMessageData *messageDataHandle,
//+				 SceMiEC *ec);
//+void SceMiMessageInPortProxyReplaceBinding(SceMiMessageInPortProxy *messageInPortProxyHandle,
//+					   const SceMiMessageInPortBinding *binding,
//+					   SceMiEC *ec);
//+const char *SceMiMessageInPortProxyTransactorName(const SceMiMessageInPortProxy *messageInPortProxyHandle);
//+const char *SceMiMessageInPortProxyPortName(const SceMiMessageInPortProxy *messageInPortProxyHandle);
//+unsigned SceMiMessageInPortProxyPortWidth(const SceMiMessageInPortProxy *messageInPortProxyHandle);


//+/* Out Proxy API */
//+void SceMiMessageOutPortProxyReplaceBinding(SceMiMessageOutPortProxy *messageOutPortProxyHandle,
//+					    const SceMiMessageOutPortBinding *binding,
//+					    SceMiEC *ec);
//+const char *SceMiMessageOutPortProxyTransactorName(const SceMiMessageOutPortProxy *messageOutPortProxyHandle);
//+const char *SceMiMessageOutPortProxyPortName(const SceMiMessageOutPortProxy *messageOutPortProxyHandle);
//+unsigned SceMiMessageOutPortProxyPortWidth(const SceMiMessageOutPortProxy *messageOutPortProxyHandle);
//+#endif
#endif
