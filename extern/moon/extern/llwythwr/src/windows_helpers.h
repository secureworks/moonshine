#ifndef LLWYTHWR_SRC_WINDOWS_HELPERS_H_
#define LLWYTHWR_SRC_WINDOWS_HELPERS_H_

#include <windows.h>
#include <winternl.h>

#include <llwythwr/windows.h>

/*
 * Defines
 */

// LDR_DATA_TABLE_ENTRY->Flags
#define LDRP_PACKAGED_BINARY 0x00000001
#define LDRP_MARKED_FOR_REMOVAL 0x00000002
#define LDRP_IMAGE_DLL 0x00000004
#define LDRP_LOAD_NOTIFICATIONS_SENT 0x00000008
#define LDRP_TELEMETRY_ENTRY_PROCESSED 0x00000010
#define LDRP_PROCESS_STATIC_IMPORT 0x00000020
#define LDRP_IN_LEGACY_LISTS 0x00000040
#define LDRP_IN_INDEXES 0x00000080
#define LDRP_SHIM_DLL 0x00000100
#define LDRP_IN_EXCEPTION_TABLE 0x00000200
#define LDRP_LOAD_IN_PROGRESS 0x00001000
#define LDRP_LOAD_CONFIG_PROCESSED 0x00002000
#define LDRP_ENTRY_PROCESSED 0x00004000
#define LDRP_PROTECT_DELAY_LOAD 0x00008000 //  LDRP_ENTRY_INSERTED
#define LDRP_DONT_CALL_FOR_THREADS 0x00040000
#define LDRP_PROCESS_ATTACH_CALLED 0x00080000
#define LDRP_PROCESS_ATTACH_FAILED 0x00100000
#define LDRP_COR_DEFERRED_VALIDATE 0x00200000
#define LDRP_COR_IMAGE 0x00400000
#define LDRP_DONT_RELOCATE 0x00800000
#define LDRP_COR_IL_ONLY 0x01000000
#define LDRP_CHPE_IMAGE 0x02000000
#define LDRP_CHPE_EMULATOR_IMAGE 0x04000000
#define LDRP_REDIRECTED 0x10000000
#define LDRP_COMPAT_DATABASE_PROCESSED 0x80000000

#define LDR_DATA_TABLE_ENTRY_SIZE_WINXP FIELD_OFFSET(LDR_DATA_TABLE_ENTRY, DdagNode)
#define LDR_DATA_TABLE_ENTRY_SIZE_WIN7 FIELD_OFFSET(LDR_DATA_TABLE_ENTRY, BaseNameHashValue)
#define LDR_DATA_TABLE_ENTRY_SIZE_WIN8 FIELD_OFFSET(LDR_DATA_TABLE_ENTRY, ImplicitPathOptions)
#define LDR_DATA_TABLE_ENTRY_SIZE_WIN10 FIELD_OFFSET(LDR_DATA_TABLE_ENTRY, SigningLevel)
#define LDR_DATA_TABLE_ENTRY_SIZE_WIN11 sizeof(LDR_DATA_TABLE_ENTRY)

#define LDR_HASH_TABLE_ENTRIES 32

/*
 * Function Typedef
 */

typedef BOOLEAN (NTAPI *PLDR_INIT_ROUTINE)(PVOID DllHandle, ULONG Reason, PVOID Context);

/*
 * Enums
 */

typedef enum LDR_DDAG_STATE
{
  LdrModulesMerged=-5,
  LdrModulesInitError=-4,
  LdrModulesSnapError=-3,
  LdrModulesUnloaded=-2,
  LdrModulesUnloading=-1,
  LdrModulesPlaceHolder=0,
  LdrModulesMapping=1,
  LdrModulesMapped=2,
  LdrModulesWaitingForDependencies=3,
  LdrModulesSnapping=4,
  LdrModulesSnapped=5,
  LdrModulesCondensed=6,
  LdrModulesReadyToInit=7,
  LdrModulesInitializing=8,
  LdrModulesReadyToRun=9
} LDR_DDAG_STATE;

typedef enum LDR_DLL_LOAD_REASON
{
  LoadReasonStaticDependency=0,
  LoadReasonStaticForwarderDependency=1,
  LoadReasonDynamicForwarderDependency=2,
  LoadReasonDelayloadDependency=3,
  LoadReasonDynamicLoad=4,
  LoadReasonAsImageLoad=5,
  LoadReasonAsDataLoad=6,
  LoadReasonUnknown=-1
} LDR_DLL_LOAD_REASON;

/*
 * Structs
 */

typedef struct RTL_BALANCED_NODE1
{
  union {
    struct RTL_BALANCED_NODE1 * Children[2];
    struct {
      struct RTL_BALANCED_NODE1 * Left;
      struct RTL_BALANCED_NODE1 * Right;
    };
  };
  union {
    union {
      struct {
        unsigned char Red: 1;
      };
      struct {
        unsigned char Balance: 2;
      };
    };

    size_t ParentValue;
  };
} RTL_BALANCED_NODE1;

typedef struct LDR_DDAG_NODE1
{
  LIST_ENTRY Modules;
  struct LDR_SERVICE_TAG_RECORD * ServiceTagList;
  unsigned long LoadCount;
  unsigned long ReferenceCount;
  unsigned long DependencyCount;
  SINGLE_LIST_ENTRY RemovalLink;
  void* IncomingDependencies;
  LDR_DDAG_STATE State;
  SINGLE_LIST_ENTRY CondenseLink;
  unsigned long PreorderNumber;
  unsigned long LowestLink;
} LDR_DDAG_NODE1;

typedef struct LDR_DATA_TABLE_ENTRY1
{
  LIST_ENTRY InLoadOrderLinks;
  LIST_ENTRY InMemoryOrderLinks;
  union
  {
    LIST_ENTRY InInitializationOrderLinks;
    LIST_ENTRY InProgressLinks;
  };
  PVOID DllBase;
  PLDR_INIT_ROUTINE EntryPoint;
  ULONG SizeOfImage;
  UNICODE_STRING FullDllName;
  UNICODE_STRING BaseDllName;
  union
  {
    UCHAR FlagGroup[4];
    ULONG Flags;
    struct
    {
      ULONG PackagedBinary : 1;
      ULONG MarkedForRemoval : 1;
      ULONG ImageDll : 1;
      ULONG LoadNotificationsSent : 1;
      ULONG TelemetryEntryProcessed : 1;
      ULONG ProcessStaticImport : 1;
      ULONG InLegacyLists : 1;
      ULONG InIndexes : 1;
      ULONG ShimDll : 1;
      ULONG InExceptionTable : 1;
      ULONG ReservedFlags1 : 2;
      ULONG LoadInProgress : 1;
      ULONG LoadConfigProcessed : 1;
      ULONG EntryProcessed : 1;
      ULONG ProtectDelayLoad : 1;
      ULONG ReservedFlags3 : 2;
      ULONG DontCallForThreads : 1;
      ULONG ProcessAttachCalled : 1;
      ULONG ProcessAttachFailed : 1;
      ULONG CorDeferredValidate : 1;
      ULONG CorImage : 1;
      ULONG DontRelocate : 1;
      ULONG CorILOnly : 1;
      ULONG ChpeImage : 1;
      ULONG ReservedFlags5 : 2;
      ULONG Redirected : 1;
      ULONG ReservedFlags6 : 2;
      ULONG CompatDatabaseProcessed : 1;
    };
  };
  USHORT ObsoleteLoadCount;
  USHORT TlsIndex;
  LIST_ENTRY HashLinks;
  ULONG TimeDateStamp;
  struct _ACTIVATION_CONTEXT *EntryPointActivationContext;
  PVOID Lock; // RtlAcquireSRWLockExclusive
  LDR_DDAG_NODE1* DdagNode;
  LIST_ENTRY NodeModuleLink;
  struct _LDRP_LOAD_CONTEXT *LoadContext;
  PVOID ParentDllBase;
  PVOID SwitchBackContext;
  RTL_BALANCED_NODE1 BaseAddressIndexNode;
  RTL_BALANCED_NODE1 MappingInfoIndexNode;
  ULONG_PTR OriginalBase;
  LARGE_INTEGER LoadTime;
  ULONG BaseNameHashValue;
  LDR_DLL_LOAD_REASON LoadReason;
  ULONG ImplicitPathOptions;
  ULONG ReferenceCount;
  ULONG DependentLoadFlags;
  //UCHAR SigningLevel; // since REDSTONE2
  //ULONG CheckSum; // since 22H1
  //PVOID ActivePatchImageBase;
  //LDR_HOT_PATCH_STATE HotPatchState;
} LDR_DATA_TABLE_ENTRY1;

typedef struct PEB_LDR_DATA1
{
  unsigned long Length;
  unsigned char Initialized;
  void * SsHandle;
  LIST_ENTRY InLoadOrderModuleList;
  LIST_ENTRY InMemoryOrderModuleList;
  LIST_ENTRY InInitializationOrderModuleList;
  void * EntryInProgress;
  unsigned char ShutdownInProgress;
  void * ShutdownThreadId;
} PEB_LDR_DATA1;

typedef struct RTL_RB_TREE1 {
  RTL_BALANCED_NODE1* Root;
  RTL_BALANCED_NODE1* Min;
} RTL_RB_TREE1;

#endif //LLWYTHWR_SRC_WINDOWS_HELPERS_H_
