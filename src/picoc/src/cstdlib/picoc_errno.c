/**
 * @brief errno implementation
 * @details
 * Errono variables are stored in a table by reference. For this reason,
 * the defines must be captured as static values so we have an actual
 * address to reference.
 */
#include "picoc/picoc_interpreter.h"

#include <errno.h>

#ifdef PICOC_CONSTANTS_IN_RAM
#define PICOC_STORAGE_TYPE static
#else
#define PICOC_STORAGE_TYPE static const
#endif

#ifdef EACCES
PICOC_STORAGE_TYPE int EACCESValue = EACCES;
#endif

#ifdef EADDRINUSE
PICOC_STORAGE_TYPE int EADDRINUSEValue = EADDRINUSE;
#endif

#ifdef EADDRNOTAVAIL
PICOC_STORAGE_TYPE int EADDRNOTAVAILValue = EADDRNOTAVAIL;
#endif

#ifdef EAFNOSUPPORT
PICOC_STORAGE_TYPE int EAFNOSUPPORTValue = EAFNOSUPPORT;
#endif

#ifdef EAGAIN
PICOC_STORAGE_TYPE int EAGAINValue = EAGAIN;
#endif

#ifdef EALREADY
PICOC_STORAGE_TYPE int EALREADYValue = EALREADY;
#endif

#ifdef EBADF
PICOC_STORAGE_TYPE int EBADFValue = EBADF;
#endif

#ifdef EBADMSG
PICOC_STORAGE_TYPE int EBADMSGValue = EBADMSG;
#endif

#ifdef EBUSY
PICOC_STORAGE_TYPE int EBUSYValue = EBUSY;
#endif

#ifdef ECANCELED
PICOC_STORAGE_TYPE int ECANCELEDValue = ECANCELED;
#endif

#ifdef ECHILD
PICOC_STORAGE_TYPE int ECHILDValue = ECHILD;
#endif

#ifdef ECONNABORTED
PICOC_STORAGE_TYPE int ECONNABORTEDValue = ECONNABORTED;
#endif

#ifdef ECONNREFUSED
PICOC_STORAGE_TYPE int ECONNREFUSEDValue = ECONNREFUSED;
#endif

#ifdef ECONNRESET
PICOC_STORAGE_TYPE int ECONNRESETValue = ECONNRESET;
#endif

#ifdef EDEADLK
PICOC_STORAGE_TYPE int EDEADLKValue = EDEADLK;
#endif

#ifdef EDESTADDRREQ
PICOC_STORAGE_TYPE int EDESTADDRREQValue = EDESTADDRREQ;
#endif

#ifdef EDOM
// https://developer.arm.com/documentation/ihi0039/e/?lang=en#errno-h
#if defined(__ARM_EABI__)
PICOC_STORAGE_TYPE int EDOMValue = 33;
#else
PICOC_STORAGE_TYPE int EDOMValue = EDOM;
#endif
#endif

#ifdef EDQUOT
PICOC_STORAGE_TYPE int EDQUOTValue = EDQUOT;
#endif

#ifdef EEXIST
PICOC_STORAGE_TYPE int EEXISTValue = EEXIST;
#endif

#ifdef EFAULT
PICOC_STORAGE_TYPE int EFAULTValue = EFAULT;
#endif

#ifdef EFBIG
PICOC_STORAGE_TYPE int EFBIGValue = EFBIG;
#endif

#ifdef EHOSTUNREACH
PICOC_STORAGE_TYPE int EHOSTUNREACHValue = EHOSTUNREACH;
#endif

#ifdef EIDRM
PICOC_STORAGE_TYPE int EIDRMValue = EIDRM;
#endif

#ifdef EILSEQ
// https://developer.arm.com/documentation/ihi0039/e/?lang=en#errno-h
#if defined(__ARM_EABI__)
PICOC_STORAGE_TYPE int EILSEQValue = 47;
#else
PICOC_STORAGE_TYPE int EILSEQValue = EILSEQ;
#endif
#endif

#ifdef EINPROGRESS
PICOC_STORAGE_TYPE int EINPROGRESSValue = EINPROGRESS;
#endif

#ifdef EINTR
PICOC_STORAGE_TYPE int EINTRValue = EINTR;
#endif

#ifdef EINVAL
PICOC_STORAGE_TYPE int EINVALValue = EINVAL;
#endif

#ifdef EIO
PICOC_STORAGE_TYPE int EIOValue = EIO;
#endif

#ifdef EISCONN
PICOC_STORAGE_TYPE int EISCONNValue = EISCONN;
#endif

#ifdef EISDIR
PICOC_STORAGE_TYPE int EISDIRValue = EISDIR;
#endif

#ifdef ELOOP
PICOC_STORAGE_TYPE int ELOOPValue = ELOOP;
#endif

#ifdef EMFILE
PICOC_STORAGE_TYPE int EMFILEValue = EMFILE;
#endif

#ifdef EMLINK
PICOC_STORAGE_TYPE int EMLINKValue = EMLINK;
#endif

#ifdef EMSGSIZE
PICOC_STORAGE_TYPE int EMSGSIZEValue = EMSGSIZE;
#endif

#ifdef EMULTIHOP
PICOC_STORAGE_TYPE int EMULTIHOPValue = EMULTIHOP;
#endif

#ifdef ENAMETOOLONG
PICOC_STORAGE_TYPE int ENAMETOOLONGValue = ENAMETOOLONG;
#endif

#ifdef ENETDOWN
PICOC_STORAGE_TYPE int ENETDOWNValue = ENETDOWN;
#endif

#ifdef ENETRESET
PICOC_STORAGE_TYPE int ENETRESETValue = ENETRESET;
#endif

#ifdef ENETUNREACH
PICOC_STORAGE_TYPE int ENETUNREACHValue = ENETUNREACH;
#endif

#ifdef ENFILE
PICOC_STORAGE_TYPE int ENFILEValue = ENFILE;
#endif

#ifdef ENOBUFS
PICOC_STORAGE_TYPE int ENOBUFSValue = ENOBUFS;
#endif

#ifdef ENODATA
PICOC_STORAGE_TYPE int ENODATAValue = ENODATA;
#endif

#ifdef ENODEV
PICOC_STORAGE_TYPE int ENODEVValue = ENODEV;
#endif

#ifdef ENOENT
PICOC_STORAGE_TYPE int ENOENTValue = ENOENT;
#endif

#ifdef ENOEXEC
PICOC_STORAGE_TYPE int ENOEXECValue = ENOEXEC;
#endif

#ifdef ENOLCK
PICOC_STORAGE_TYPE int ENOLCKValue = ENOLCK;
#endif

#ifdef ENOLINK
PICOC_STORAGE_TYPE int ENOLINKValue = ENOLINK;
#endif

#ifdef ENOMEM
PICOC_STORAGE_TYPE int ENOMEMValue = ENOMEM;
#endif

#ifdef ENOMSG
PICOC_STORAGE_TYPE int ENOMSGValue = ENOMSG;
#endif

#ifdef ENOPROTOOPT
PICOC_STORAGE_TYPE int ENOPROTOOPTValue = ENOPROTOOPT;
#endif

#ifdef ENOSPC
PICOC_STORAGE_TYPE int ENOSPCValue = ENOSPC;
#endif

#ifdef ENOSR
PICOC_STORAGE_TYPE int ENOSRValue = ENOSR;
#endif

#ifdef ENOSTR
PICOC_STORAGE_TYPE int ENOSTRValue = ENOSTR;
#endif

#ifdef ENOSYS
PICOC_STORAGE_TYPE int ENOSYSValue = ENOSYS;
#endif

#ifdef ENOTCONN
PICOC_STORAGE_TYPE int ENOTCONNValue = ENOTCONN;
#endif

#ifdef ENOTDIR
PICOC_STORAGE_TYPE int ENOTDIRValue = ENOTDIR;
#endif

#ifdef ENOTEMPTY
PICOC_STORAGE_TYPE int ENOTEMPTYValue = ENOTEMPTY;
#endif

#ifdef ENOTRECOVERABLE
PICOC_STORAGE_TYPE int ENOTRECOVERABLEValue = ENOTRECOVERABLE;
#endif

#ifdef ENOTSOCK
PICOC_STORAGE_TYPE int ENOTSOCKValue = ENOTSOCK;
#endif

#ifdef ENOTSUP
PICOC_STORAGE_TYPE int ENOTSUPValue = ENOTSUP;
#endif

#ifdef ENOTTY
PICOC_STORAGE_TYPE int ENOTTYValue = ENOTTY;
#endif

#ifdef ENXIO
PICOC_STORAGE_TYPE int ENXIOValue = ENXIO;
#endif

#ifdef EOPNOTSUPP
PICOC_STORAGE_TYPE int EOPNOTSUPPValue = EOPNOTSUPP;
#endif

#ifdef EOVERFLOW
PICOC_STORAGE_TYPE int EOVERFLOWValue = EOVERFLOW;
#endif

#ifdef EOWNERDEAD
PICOC_STORAGE_TYPE int EOWNERDEADValue = EOWNERDEAD;
#endif

#ifdef EPERM
PICOC_STORAGE_TYPE int EPERMValue = EPERM;
#endif

#ifdef EPIPE
PICOC_STORAGE_TYPE int EPIPEValue = EPIPE;
#endif

#ifdef EPROTO
PICOC_STORAGE_TYPE int EPROTOValue = EPROTO;
#endif

#ifdef EPROTONOSUPPORT
PICOC_STORAGE_TYPE int EPROTONOSUPPORTValue = EPROTONOSUPPORT;
#endif

#ifdef EPROTOTYPE
PICOC_STORAGE_TYPE int EPROTOTYPEValue = EPROTOTYPE;
#endif

#ifdef ERANGE
// https://developer.arm.com/documentation/ihi0039/e/?lang=en#errno-h
#if defined(__ARM_EABI__)
PICOC_STORAGE_TYPE int ERANGEValue = 34;
#else
PICOC_STORAGE_TYPE int ERANGEValue = ERANGE;
#endif
#endif

#ifdef EROFS
PICOC_STORAGE_TYPE int EROFSValue = EROFS;
#endif

#ifdef ESPIPE
PICOC_STORAGE_TYPE int ESPIPEValue = ESPIPE;
#endif

#ifdef ESRCH
PICOC_STORAGE_TYPE int ESRCHValue = ESRCH;
#endif

#ifdef ESTALE
PICOC_STORAGE_TYPE int ESTALEValue = ESTALE;
#endif

#ifdef ETIME
PICOC_STORAGE_TYPE int ETIMEValue = ETIME;
#endif

#ifdef ETIMEDOUT
PICOC_STORAGE_TYPE int ETIMEDOUTValue = ETIMEDOUT;
#endif

#ifdef ETXTBSY
PICOC_STORAGE_TYPE int ETXTBSYValue = ETXTBSY;
#endif

#ifdef EWOULDBLOCK
PICOC_STORAGE_TYPE int EWOULDBLOCKValue = EWOULDBLOCK;
#endif

#ifdef EXDEV
PICOC_STORAGE_TYPE int EXDEVValue = EXDEV;
#endif

/* creates various system-dependent definitions */
void StdErrnoSetupFunc(Picoc* pc) {
    /* defines */
#ifdef EACCES
    VariableDefinePlatformVar(pc, NULL, "EACCES", &pc->IntType, (union AnyValue*)&EACCESValue, false);
#endif

#ifdef EADDRINUSE
    VariableDefinePlatformVar(pc, NULL, "EADDRINUSE", &pc->IntType, (union AnyValue*)&EADDRINUSEValue, false);
#endif

#ifdef EADDRNOTAVAIL
    VariableDefinePlatformVar(pc, NULL, "EADDRNOTAVAIL", &pc->IntType, (union AnyValue*)&EADDRNOTAVAILValue, false);
#endif

#ifdef EAFNOSUPPORT
    VariableDefinePlatformVar(pc, NULL, "EAFNOSUPPORT", &pc->IntType, (union AnyValue*)&EAFNOSUPPORTValue, false);
#endif

#ifdef EAGAIN
    VariableDefinePlatformVar(pc, NULL, "EAGAIN", &pc->IntType, (union AnyValue*)&EAGAINValue, false);
#endif

#ifdef EALREADY
    VariableDefinePlatformVar(pc, NULL, "EALREADY", &pc->IntType, (union AnyValue*)&EALREADYValue, false);
#endif

#ifdef EBADF
    VariableDefinePlatformVar(pc, NULL, "EBADF", &pc->IntType, (union AnyValue*)&EBADFValue, false);
#endif

#ifdef EBADMSG
    VariableDefinePlatformVar(pc, NULL, "EBADMSG", &pc->IntType, (union AnyValue*)&EBADMSGValue, false);
#endif

#ifdef EBUSY
    VariableDefinePlatformVar(pc, NULL, "EBUSY", &pc->IntType, (union AnyValue*)&EBUSYValue, false);
#endif

#ifdef ECANCELED
    VariableDefinePlatformVar(pc, NULL, "ECANCELED", &pc->IntType, (union AnyValue*)&ECANCELEDValue, false);
#endif

#ifdef ECHILD
    VariableDefinePlatformVar(pc, NULL, "ECHILD", &pc->IntType, (union AnyValue*)&ECHILDValue, false);
#endif

#ifdef ECONNABORTED
    VariableDefinePlatformVar(pc, NULL, "ECONNABORTED", &pc->IntType, (union AnyValue*)&ECONNABORTEDValue, false);
#endif

#ifdef ECONNREFUSED
    VariableDefinePlatformVar(pc, NULL, "ECONNREFUSED", &pc->IntType, (union AnyValue*)&ECONNREFUSEDValue, false);
#endif

#ifdef ECONNRESET
    VariableDefinePlatformVar(pc, NULL, "ECONNRESET", &pc->IntType, (union AnyValue*)&ECONNRESETValue, false);
#endif

#ifdef EDEADLK
    VariableDefinePlatformVar(pc, NULL, "EDEADLK", &pc->IntType, (union AnyValue*)&EDEADLKValue, false);
#endif

#ifdef EDESTADDRREQ
    VariableDefinePlatformVar(pc, NULL, "EDESTADDRREQ", &pc->IntType, (union AnyValue*)&EDESTADDRREQValue, false);
#endif

#ifdef EDOM
    VariableDefinePlatformVar(pc, NULL, "EDOM", &pc->IntType, (union AnyValue*)&EDOMValue, false);
#endif

#ifdef EDQUOT
    VariableDefinePlatformVar(pc, NULL, "EDQUOT", &pc->IntType, (union AnyValue*)&EDQUOTValue, false);
#endif

#ifdef EEXIST
    VariableDefinePlatformVar(pc, NULL, "EEXIST", &pc->IntType, (union AnyValue*)&EEXISTValue, false);
#endif

#ifdef EFAULT
    VariableDefinePlatformVar(pc, NULL, "EFAULT", &pc->IntType, (union AnyValue*)&EFAULTValue, false);
#endif

#ifdef EFBIG
    VariableDefinePlatformVar(pc, NULL, "EFBIG", &pc->IntType, (union AnyValue*)&EFBIGValue, false);
#endif

#ifdef EHOSTUNREACH
    VariableDefinePlatformVar(pc, NULL, "EHOSTUNREACH", &pc->IntType, (union AnyValue*)&EHOSTUNREACHValue, false);
#endif

#ifdef EIDRM
    VariableDefinePlatformVar(pc, NULL, "EIDRM", &pc->IntType, (union AnyValue*)&EIDRMValue, false);
#endif

#ifdef EILSEQ
    VariableDefinePlatformVar(pc, NULL, "EILSEQ", &pc->IntType, (union AnyValue*)&EILSEQValue, false);
#endif

#ifdef EINPROGRESS
    VariableDefinePlatformVar(pc, NULL, "EINPROGRESS", &pc->IntType, (union AnyValue*)&EINPROGRESSValue, false);
#endif

#ifdef EINTR
    VariableDefinePlatformVar(pc, NULL, "EINTR", &pc->IntType, (union AnyValue*)&EINTRValue, false);
#endif

#ifdef EINVAL
    VariableDefinePlatformVar(pc, NULL, "EINVAL", &pc->IntType, (union AnyValue*)&EINVALValue, false);
#endif

#ifdef EIO
    VariableDefinePlatformVar(pc, NULL, "EIO", &pc->IntType, (union AnyValue*)&EIOValue, false);
#endif

#ifdef EISCONN
    VariableDefinePlatformVar(pc, NULL, "EISCONN", &pc->IntType, (union AnyValue*)&EISCONNValue, false);
#endif

#ifdef EISDIR
    VariableDefinePlatformVar(pc, NULL, "EISDIR", &pc->IntType, (union AnyValue*)&EISDIRValue, false);
#endif

#ifdef ELOOP
    VariableDefinePlatformVar(pc, NULL, "ELOOP", &pc->IntType, (union AnyValue*)&ELOOPValue, false);
#endif

#ifdef EMFILE
    VariableDefinePlatformVar(pc, NULL, "EMFILE", &pc->IntType, (union AnyValue*)&EMFILEValue, false);
#endif

#ifdef EMLINK
    VariableDefinePlatformVar(pc, NULL, "EMLINK", &pc->IntType, (union AnyValue*)&EMLINKValue, false);
#endif

#ifdef EMSGSIZE
    VariableDefinePlatformVar(pc, NULL, "EMSGSIZE", &pc->IntType, (union AnyValue*)&EMSGSIZEValue, false);
#endif

#ifdef EMULTIHOP
    VariableDefinePlatformVar(pc, NULL, "EMULTIHOP", &pc->IntType, (union AnyValue*)&EMULTIHOPValue, false);
#endif

#ifdef ENAMETOOLONG
    VariableDefinePlatformVar(pc, NULL, "ENAMETOOLONG", &pc->IntType, (union AnyValue*)&ENAMETOOLONGValue, false);
#endif

#ifdef ENETDOWN
    VariableDefinePlatformVar(pc, NULL, "ENETDOWN", &pc->IntType, (union AnyValue*)&ENETDOWNValue, false);
#endif

#ifdef ENETRESET
    VariableDefinePlatformVar(pc, NULL, "ENETRESET", &pc->IntType, (union AnyValue*)&ENETRESETValue, false);
#endif

#ifdef ENETUNREACH
    VariableDefinePlatformVar(pc, NULL, "ENETUNREACH", &pc->IntType, (union AnyValue*)&ENETUNREACHValue, false);
#endif

#ifdef ENFILE
    VariableDefinePlatformVar(pc, NULL, "ENFILE", &pc->IntType, (union AnyValue*)&ENFILEValue, false);
#endif

#ifdef ENOBUFS
    VariableDefinePlatformVar(pc, NULL, "ENOBUFS", &pc->IntType, (union AnyValue*)&ENOBUFSValue, false);
#endif

#ifdef ENODATA
    VariableDefinePlatformVar(pc, NULL, "ENODATA", &pc->IntType, (union AnyValue*)&ENODATAValue, false);
#endif

#ifdef ENODEV
    VariableDefinePlatformVar(pc, NULL, "ENODEV", &pc->IntType, (union AnyValue*)&ENODEVValue, false);
#endif

#ifdef ENOENT
    VariableDefinePlatformVar(pc, NULL, "ENOENT", &pc->IntType, (union AnyValue*)&ENOENTValue, false);
#endif

#ifdef ENOEXEC
    VariableDefinePlatformVar(pc, NULL, "ENOEXEC", &pc->IntType, (union AnyValue*)&ENOEXECValue, false);
#endif

#ifdef ENOLCK
    VariableDefinePlatformVar(pc, NULL, "ENOLCK", &pc->IntType, (union AnyValue*)&ENOLCKValue, false);
#endif

#ifdef ENOLINK
    VariableDefinePlatformVar(pc, NULL, "ENOLINK", &pc->IntType, (union AnyValue*)&ENOLINKValue, false);
#endif

#ifdef ENOMEM
    VariableDefinePlatformVar(pc, NULL, "ENOMEM", &pc->IntType, (union AnyValue*)&ENOMEMValue, false);
#endif

#ifdef ENOMSG
    VariableDefinePlatformVar(pc, NULL, "ENOMSG", &pc->IntType, (union AnyValue*)&ENOMSGValue, false);
#endif

#ifdef ENOPROTOOPT
    VariableDefinePlatformVar(pc, NULL, "ENOPROTOOPT", &pc->IntType, (union AnyValue*)&ENOPROTOOPTValue, false);
#endif

#ifdef ENOSPC
    VariableDefinePlatformVar(pc, NULL, "ENOSPC", &pc->IntType, (union AnyValue*)&ENOSPCValue, false);
#endif

#ifdef ENOSR
    VariableDefinePlatformVar(pc, NULL, "ENOSR", &pc->IntType, (union AnyValue*)&ENOSRValue, false);
#endif

#ifdef ENOSTR
    VariableDefinePlatformVar(pc, NULL, "ENOSTR", &pc->IntType, (union AnyValue*)&ENOSTRValue, false);
#endif

#ifdef ENOSYS
    VariableDefinePlatformVar(pc, NULL, "ENOSYS", &pc->IntType, (union AnyValue*)&ENOSYSValue, false);
#endif

#ifdef ENOTCONN
    VariableDefinePlatformVar(pc, NULL, "ENOTCONN", &pc->IntType, (union AnyValue*)&ENOTCONNValue, false);
#endif

#ifdef ENOTDIR
    VariableDefinePlatformVar(pc, NULL, "ENOTDIR", &pc->IntType, (union AnyValue*)&ENOTDIRValue, false);
#endif

#ifdef ENOTEMPTY
    VariableDefinePlatformVar(pc, NULL, "ENOTEMPTY", &pc->IntType, (union AnyValue*)&ENOTEMPTYValue, false);
#endif

#ifdef ENOTRECOVERABLE
    VariableDefinePlatformVar(pc, NULL, "ENOTRECOVERABLE", &pc->IntType, (union AnyValue*)&ENOTRECOVERABLEValue, false);
#endif

#ifdef ENOTSOCK
    VariableDefinePlatformVar(pc, NULL, "ENOTSOCK", &pc->IntType, (union AnyValue*)&ENOTSOCKValue, false);
#endif

#ifdef ENOTSUP
    VariableDefinePlatformVar(pc, NULL, "ENOTSUP", &pc->IntType, (union AnyValue*)&ENOTSUPValue, false);
#endif

#ifdef ENOTTY
    VariableDefinePlatformVar(pc, NULL, "ENOTTY", &pc->IntType, (union AnyValue*)&ENOTTYValue, false);
#endif

#ifdef ENXIO
    VariableDefinePlatformVar(pc, NULL, "ENXIO", &pc->IntType, (union AnyValue*)&ENXIOValue, false);
#endif

#ifdef EOPNOTSUPP
    VariableDefinePlatformVar(pc, NULL, "EOPNOTSUPP", &pc->IntType, (union AnyValue*)&EOPNOTSUPPValue, false);
#endif

#ifdef EOVERFLOW
    VariableDefinePlatformVar(pc, NULL, "EOVERFLOW", &pc->IntType, (union AnyValue*)&EOVERFLOWValue, false);
#endif

#ifdef EOWNERDEAD
    VariableDefinePlatformVar(pc, NULL, "EOWNERDEAD", &pc->IntType, (union AnyValue*)&EOWNERDEADValue, false);
#endif

#ifdef EPERM
    VariableDefinePlatformVar(pc, NULL, "EPERM", &pc->IntType, (union AnyValue*)&EPERMValue, false);
#endif

#ifdef EPIPE
    VariableDefinePlatformVar(pc, NULL, "EPIPE", &pc->IntType, (union AnyValue*)&EPIPEValue, false);
#endif

#ifdef EPROTO
    VariableDefinePlatformVar(pc, NULL, "EPROTO", &pc->IntType, (union AnyValue*)&EPROTOValue, false);
#endif

#ifdef EPROTONOSUPPORT
    VariableDefinePlatformVar(pc, NULL, "EPROTONOSUPPORT", &pc->IntType, (union AnyValue*)&EPROTONOSUPPORTValue, false);
#endif

#ifdef EPROTOTYPE
    VariableDefinePlatformVar(pc, NULL, "EPROTOTYPE", &pc->IntType, (union AnyValue*)&EPROTOTYPEValue, false);
#endif

#ifdef ERANGE
    VariableDefinePlatformVar(pc, NULL, "ERANGE", &pc->IntType, (union AnyValue*)&ERANGEValue, false);
#endif

#ifdef EROFS
    VariableDefinePlatformVar(pc, NULL, "EROFS", &pc->IntType, (union AnyValue*)&EROFSValue, false);
#endif

#ifdef ESPIPE
    VariableDefinePlatformVar(pc, NULL, "ESPIPE", &pc->IntType, (union AnyValue*)&ESPIPEValue, false);
#endif

#ifdef ESRCH
    VariableDefinePlatformVar(pc, NULL, "ESRCH", &pc->IntType, (union AnyValue*)&ESRCHValue, false);
#endif

#ifdef ESTALE
    VariableDefinePlatformVar(pc, NULL, "ESTALE", &pc->IntType, (union AnyValue*)&ESTALEValue, false);
#endif

#ifdef ETIME
    VariableDefinePlatformVar(pc, NULL, "ETIME", &pc->IntType, (union AnyValue*)&ETIMEValue, false);
#endif

#ifdef ETIMEDOUT
    VariableDefinePlatformVar(pc, NULL, "ETIMEDOUT", &pc->IntType, (union AnyValue*)&ETIMEDOUTValue, false);
#endif

#ifdef ETXTBSY
    VariableDefinePlatformVar(pc, NULL, "ETXTBSY", &pc->IntType, (union AnyValue*)&ETXTBSYValue, false);
#endif

#ifdef EWOULDBLOCK
    VariableDefinePlatformVar(pc, NULL, "EWOULDBLOCK", &pc->IntType, (union AnyValue*)&EWOULDBLOCKValue, false);
#endif

#ifdef EXDEV
    VariableDefinePlatformVar(pc, NULL, "EXDEV", &pc->IntType, (union AnyValue*)&EXDEVValue, false);
#endif

    VariableDefinePlatformVar(pc, NULL, "errno", &pc->IntType, (union AnyValue*)&errno, true);
}
