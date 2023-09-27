#include "windows_helpers.h"

#include <stddef.h>
#include "llwythwr/windows.h"
#include "debug.h"

// Relative Virtual Address to Virtual Address
#define RVA2VA(type, base, rva) (type)((ULONG_PTR) base + rva)

static int deinamig_memcmp(const void *s1, const void *s2, size_t n) {
  if (n != 0) {
    const unsigned char *p1 = s1, *p2 = s2;

    do {
      if (*p1++ != *p2++)
        return (*--p1 - *--p2);
    } while (--n != 0);
  }
  return (0);
}

static void *llwythwr_memcpy(void *restrict dstptr, const void *restrict srcptr, size_t size) {
  unsigned char *dst = (unsigned char *) dstptr;
  const unsigned char *src = (const unsigned char *) srcptr;
  for (size_t i = 0; i < size; i++)
    dst[i] = src[i];
  return dstptr;
}

static int llwythwr_strcmp(const char *s1, const char *s2) {
  while (*s1 == *s2++) {
    if (*s1++ == '\0') return (0);
  }
  return (*(const unsigned char *) s1 - *(const unsigned char *) (s2 - 1));
}

static LDR_DATA_TABLE_ENTRY1* create_ldr_entry(llwythwr_windows_api *api, void* module_base, const wchar_t* dll_name, const wchar_t* dll_path) {
  PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER) module_base;
  PIMAGE_NT_HEADERS nt = RVA2VA(PIMAGE_NT_HEADERS, module_base, dos->e_lfanew);

  UNICODE_STRING unicode_name = {0};
  api->RtlInitUnicodeString(&unicode_name, dll_name);

  UNICODE_STRING unicode_path = {0};
  api->RtlInitUnicodeString(&unicode_path, dll_path);

  LDR_DATA_TABLE_ENTRY1* entry = (LDR_DATA_TABLE_ENTRY1*) api->LocalAlloc(LPTR, sizeof(LDR_DATA_TABLE_ENTRY1));

  if (entry == NULL) {
    DPRINT("Could not allocate memory for entry.\n");
    api->SetLastError(ERROR_OUTOFMEMORY);
    return NULL;
  }

  entry->DdagNode = (LDR_DDAG_NODE1*) api->LocalAlloc(LPTR, sizeof(LDR_DDAG_NODE1));

  if (entry->DdagNode == NULL) {
    DPRINT("Could not allocate memory for entry->DdagNode.\n");
    api->SetLastError(ERROR_OUTOFMEMORY);
    api->LocalFree(entry);
    return NULL;
  }

  entry->ReferenceCount        = 1;
  entry->LoadReason            = LoadReasonDynamicLoad;
  entry->OriginalBase          = nt->OptionalHeader.ImageBase;
  entry->EntryPoint            = RVA2VA(LPVOID, module_base, nt->OptionalHeader.AddressOfEntryPoint);
  entry->DllBase               = module_base;
  entry->SizeOfImage           = nt->OptionalHeader.SizeOfImage;
  entry->TimeDateStamp         = nt->FileHeader.TimeDateStamp;
  entry->BaseDllName           = unicode_name;
  entry->FullDllName           = unicode_path;
  entry->ObsoleteLoadCount     = 1;
  entry->Flags                 = LDRP_IMAGE_DLL | LDRP_PROTECT_DELAY_LOAD | LDRP_ENTRY_PROCESSED | LDRP_PROCESS_ATTACH_CALLED | LDRP_LOAD_NOTIFICATIONS_SENT | LDRP_IN_LEGACY_LISTS | LDRP_IN_INDEXES | LDRP_PROCESS_STATIC_IMPORT;

  api->NtQuerySystemTime(&entry->LoadTime);

  ULONG hash;
  api->RtlHashUnicodeString(&entry->BaseDllName, TRUE, 0, &hash);
  entry->BaseNameHashValue = hash;

  entry->NodeModuleLink.Flink    = &entry->DdagNode->Modules;
  entry->NodeModuleLink.Blink    = &entry->DdagNode->Modules;
  entry->DdagNode->Modules.Flink = &entry->NodeModuleLink;
  entry->DdagNode->Modules.Blink = &entry->NodeModuleLink;
  entry->DdagNode->State         = LdrModulesReadyToRun;
  entry->DdagNode->LoadCount     = 1;

  return entry;
}

PVOID find_ldrp_hash_table(llwythwr_windows_api *api)
{
  PEB_LDR_DATA1 *ldr = (PEB_LDR_DATA1*) NtCurrentTeb()->ProcessEnvironmentBlock->Ldr;
  LDR_DATA_TABLE_ENTRY1 *ntdll = CONTAINING_RECORD(ldr->InInitializationOrderModuleList.Flink, LDR_DATA_TABLE_ENTRY1, InInitializationOrderLinks);

  ULONG ntdl_hash_index;
  api->RtlHashUnicodeString(&ntdll->BaseDllName, TRUE, 0, &ntdl_hash_index);
  ntdl_hash_index &= 0x1F;

  ULONG_PTR NtdllBase = (ULONG_PTR) ntdll->DllBase;
  ULONG_PTR NtdllEndAddress = NtdllBase + ntdll->SizeOfImage - 1;

  // scan hash list to the head (head is located within ntdll)
  BOOL bHeadFound = 0;
  PLIST_ENTRY pNtdllHashHead = NULL;

  for (PLIST_ENTRY e = (PLIST_ENTRY) ntdll->HashLinks.Flink; e != (PLIST_ENTRY) &ntdll->HashLinks; e = e->Flink) {
    if ((ULONG_PTR) e >= NtdllBase && (ULONG_PTR)e < NtdllEndAddress) {
      bHeadFound = 1;
      pNtdllHashHead = e;
      break;
    }
  }

  if (bHeadFound)
    return pNtdllHashHead - ntdl_hash_index;

  return NULL;
}

void insert_tail_list(PLIST_ENTRY ListHead, PLIST_ENTRY Entry)
{
  PLIST_ENTRY PrevEntry = ListHead->Blink;

  Entry->Flink = ListHead;
  Entry->Blink = PrevEntry;

  PrevEntry->Flink = Entry;
  ListHead->Blink  = Entry;
}

int insert_hash_node(llwythwr_windows_api *api, PLIST_ENTRY pNodeLink, ULONG hash)
{
  if (pNodeLink == NULL)
    return FALSE;

  PVOID LdrpHashTable = find_ldrp_hash_table(api);
  if (LdrpHashTable == NULL) {
    DPRINT("Could not find LdrpHashTable\n");
    return FALSE;
  }
  // LrpHashTable record
  PLIST_ENTRY pHashList = (PLIST_ENTRY) (LdrpHashTable + sizeof(LIST_ENTRY) * (hash & 0x1F));
  insert_tail_list(pHashList, pNodeLink);

  return TRUE;
}

void* find_ldrp_module_index_base()
{
  PEB_LDR_DATA1 *ldr = (PEB_LDR_DATA1*) NtCurrentTeb()->ProcessEnvironmentBlock->Ldr;
  LDR_DATA_TABLE_ENTRY1 *ntdll = CONTAINING_RECORD(ldr->InInitializationOrderModuleList.Flink, LDR_DATA_TABLE_ENTRY1, InInitializationOrderLinks);

  RTL_BALANCED_NODE1* pNode = &ntdll->BaseAddressIndexNode;

  // Get root node
  RTL_BALANCED_NODE1* lastNode = 0;
  for(; pNode->ParentValue; ) {
    // Ignore last few bits
    lastNode = (RTL_BALANCED_NODE1*) (pNode->ParentValue & (size_t) -8);
    pNode = lastNode;
  }

  if (!pNode->Red) {
    PIMAGE_NT_HEADERS nt = RVA2VA(PIMAGE_NT_HEADERS, ntdll->DllBase, ((PIMAGE_DOS_HEADER)ntdll->DllBase)->e_lfanew);
    PIMAGE_SECTION_HEADER sh = IMAGE_FIRST_SECTION(nt);

    PVOID begin = 0;
    DWORD length = 0;
    for (INT i = 0; i < nt->FileHeader.NumberOfSections; i++) {
      char dot_data[] = {'.','d','a','t','a','\0'};
      if (llwythwr_strcmp(dot_data, (LPCSTR) sh->Name) == 0) {
        begin = ntdll->DllBase + sh->VirtualAddress;
        length = sh->Misc.VirtualSize;
        break;
      }
      ++sh;
    }

    for (DWORD i = 0; i < length - sizeof(SIZE_T); begin++, i++) {
      if (deinamig_memcmp((PVOID)begin, (PVOID)&pNode, sizeof(SIZE_T)) == 0) {
        return begin;
      }
    }
  }

  return NULL;
}

void* find_ldrp_mapping_info_base()
{
  PEB_LDR_DATA1 *ldr = (PEB_LDR_DATA1*) NtCurrentTeb()->ProcessEnvironmentBlock->Ldr;
  LDR_DATA_TABLE_ENTRY1 *ntdll = CONTAINING_RECORD(ldr->InInitializationOrderModuleList.Flink, LDR_DATA_TABLE_ENTRY1, InInitializationOrderLinks);

  RTL_BALANCED_NODE1* pNode = &ntdll->MappingInfoIndexNode;

  // Get root node
  RTL_BALANCED_NODE1* lastNode = 0;
  for(; pNode->ParentValue; ) {
    // Ignore last few bits
    lastNode = (RTL_BALANCED_NODE1*) (pNode->ParentValue & (size_t) -8);
    pNode = lastNode;
  }

  if (!pNode->Red) {
    PIMAGE_NT_HEADERS nt = RVA2VA(PIMAGE_NT_HEADERS, ntdll->DllBase, ((PIMAGE_DOS_HEADER)ntdll->DllBase)->e_lfanew);
    PIMAGE_SECTION_HEADER sh = IMAGE_FIRST_SECTION(nt);

    PVOID begin = 0;
    DWORD length = 0;
    for (INT i = 0; i < nt->FileHeader.NumberOfSections; i++) {
      char dot_data[] = {'.','d','a','t','a','\0'};
      if (llwythwr_strcmp(dot_data, (LPCSTR) sh->Name) == 0) {
        begin = ntdll->DllBase + sh->VirtualAddress;
        length = sh->Misc.VirtualSize;
        break;
      }
      ++sh;
    }

    for (DWORD i = 0; i < length - sizeof(SIZE_T); begin++, i++) {
      if (deinamig_memcmp((PVOID)begin, (PVOID)&pNode, sizeof(SIZE_T)) == 0) {
        return begin;
      }
    }
  }

  return NULL;
}

int insert_into_ldrp_module_index(llwythwr_windows_api *api, LDR_DATA_TABLE_ENTRY1* entry, void* module_base)
{
  PVOID LdrpModuleIndexBase = find_ldrp_module_index_base();
  if (LdrpModuleIndexBase == NULL) {
    DPRINT("Could not find LdrpModuleIndexBase\n");
    return FALSE;
  }

  LDR_DATA_TABLE_ENTRY1 *pLdrNode = CONTAINING_RECORD(LdrpModuleIndexBase, LDR_DATA_TABLE_ENTRY1, BaseAddressIndexNode);
  LDR_DATA_TABLE_ENTRY1 LdrNode = *pLdrNode;

  BOOL right = FALSE;
  while (TRUE)
  {
    if (module_base < LdrNode.DllBase) {
      if (LdrNode.BaseAddressIndexNode.Left) {
        pLdrNode = CONTAINING_RECORD( LdrNode.BaseAddressIndexNode.Left, LDR_DATA_TABLE_ENTRY1, BaseAddressIndexNode );
        LdrNode = *pLdrNode;
      }
      else
        break;
    }
    else if (module_base  > LdrNode.DllBase) {
      if (LdrNode.BaseAddressIndexNode.Right) {
        pLdrNode = CONTAINING_RECORD( LdrNode.BaseAddressIndexNode.Right, LDR_DATA_TABLE_ENTRY1, BaseAddressIndexNode );
        LdrNode = *pLdrNode;
      }
      else {
        right = TRUE;
        break;
      }
    }
    else { // Already in tree (increase ref counter)
      // pLdrNode->DdagNode->ReferenceCount++;
      return TRUE;
    }
  }

  api->RtlRbInsertNodeEx((PRTL_RB_TREE) LdrpModuleIndexBase,
                         (PRTL_BALANCED_NODE) &pLdrNode->BaseAddressIndexNode, right,
                         (PRTL_BALANCED_NODE) &entry->BaseAddressIndexNode);

  return TRUE;
}

int insert_into_ldrp_mapping_info_index(llwythwr_windows_api *api, LDR_DATA_TABLE_ENTRY1* entry, void* module_base)
{
  PVOID LdrpMappingInfoBase = find_ldrp_mapping_info_base();
  if (LdrpMappingInfoBase == NULL) {
    DPRINT("Could not find LdrpMappingInfoBase\n");
    return FALSE;
  }

  LDR_DATA_TABLE_ENTRY1 *pLdrNode = CONTAINING_RECORD(LdrpMappingInfoBase, LDR_DATA_TABLE_ENTRY1, MappingInfoIndexNode);
  LDR_DATA_TABLE_ENTRY1 LdrNode = *pLdrNode;

  BOOL right = FALSE;
  while (TRUE)
  {
    if (module_base < LdrNode.DllBase) {
      if (LdrNode.MappingInfoIndexNode.Left) {
        pLdrNode = CONTAINING_RECORD( LdrNode.MappingInfoIndexNode.Left, LDR_DATA_TABLE_ENTRY1, MappingInfoIndexNode );
        LdrNode = *pLdrNode;
      }
      else
        break;
    }
    else if (module_base  > LdrNode.DllBase) {
      if (LdrNode.MappingInfoIndexNode.Right) {
        pLdrNode = CONTAINING_RECORD( LdrNode.MappingInfoIndexNode.Right, LDR_DATA_TABLE_ENTRY1, MappingInfoIndexNode );
        LdrNode = *pLdrNode;
      }
      else {
        right = TRUE;
        break;
      }
    }
    else { // Already in tree (increase ref counter)
      // pLdrNode->DdagNode->ReferenceCount++;
      return TRUE;
    }
  }

  api->RtlRbInsertNodeEx((PRTL_RB_TREE) LdrpMappingInfoBase,
                         (PRTL_BALANCED_NODE) &pLdrNode->MappingInfoIndexNode, right,
                         (PRTL_BALANCED_NODE) &entry->MappingInfoIndexNode);

  return TRUE;
}

LDR_DATA_TABLE_ENTRY* llwythwr_link_library_win8plus(llwythwr_windows_api *api, void* module_base, const wchar_t* dll_name, const wchar_t* dll_path) {

  LDR_DATA_TABLE_ENTRY1* entry = create_ldr_entry(api, module_base, dll_name, dll_path);
  if (entry == NULL) {
    DPRINT("Could not create ldr data table entry for %S.\n", dll_name);
    return NULL;
  }
  DPRINT("LDR_DATA_TABLE_ENTRY created for %S with hash %lu.\n", dll_name, entry->BaseNameHashValue);

  if (!insert_into_ldrp_mapping_info_index(api, entry, module_base)) {
    DPRINT("Could not add entry for %S in LdrpMappingInfoIndex.\n", dll_name);
    api->LocalFree(entry->DdagNode);
    api->LocalFree(entry);
    return NULL;
  }
  DPRINT("Entry added to LdrpModuleIndex for %S.\n", dll_name);

  if (!insert_into_ldrp_module_index(api, entry, module_base)) {
    DPRINT("Could not add entry for %S in LdrpModuleIndex.\n", dll_name);
    // TODO remove entry from LdrpModuleBaseAddressIndex!
    api->LocalFree(entry->DdagNode);
    api->LocalFree(entry);
    return NULL;
  }
  DPRINT("Entry added to LdrpModuleIndex for %S.\n", dll_name);

  if (!insert_hash_node(api, &entry->HashLinks, entry->BaseNameHashValue)) {
    DPRINT("Could not add entry for %S to LdrpHashTable.\n", dll_name);
    // TODO remove entry from LdrpModuleIndex!
    // TODO remove entry from LdrpModuleBaseAddressIndex!
    api->LocalFree(entry->DdagNode);
    api->LocalFree(entry);
    return NULL;
  }
  DPRINT("Entry added to LdrpHashTable for %S.\n", dll_name);

  // insert into other lists
  PEB_LDR_DATA1 *ldr = (PEB_LDR_DATA1*) NtCurrentTeb()->ProcessEnvironmentBlock->Ldr;
  insert_tail_list(&ldr->InLoadOrderModuleList, &entry->InLoadOrderLinks);
  DPRINT("Entry added to InLoadOrderModuleList for %S.\n", dll_name);
  insert_tail_list(&ldr->InMemoryOrderModuleList, &entry->InMemoryOrderLinks);
  DPRINT("Entry added to InMemoryOrderModuleList for %S.\n", dll_name);
  insert_tail_list(&ldr->InInitializationOrderModuleList, &entry->InInitializationOrderLinks);
  DPRINT("Entry added to InInitializationOrderModuleList for %S.\n", dll_name);

  return (LDR_DATA_TABLE_ENTRY*) entry;
}




/*
static RTL_RB_TREE* find_ldrp_module_index_base() {
  PEB* peb = (PPEB) NtCurrentTeb()->ProcessEnvironmentBlock;
  PEB_LDR_DATA_W8 *ldr = (PEB_LDR_DATA_W8*) peb->Ldr;
  LDR_DATA_TABLE_ENTRY_W8 *ntdll = CONTAINING_RECORD(ldr->InInitializationOrderModuleList.Flink, LDR_DATA_TABLE_ENTRY_W8, InInitializationOrderLinks);

  RTL_BALANCED_NODE* node = &ntdll->BaseAddressIndexNode;

  do {
    node = (RTL_BALANCED_NODE*)(node->ParentValue & (~7));
  } while (node->ParentValue & (~7));

  if (!node->Red) {
    PIMAGE_NT_HEADERS nt = RVA2VA(PIMAGE_NT_HEADERS, ntdll->DllBase, ((PIMAGE_DOS_HEADER)ntdll->DllBase)->e_lfanew);
    PIMAGE_SECTION_HEADER sh = IMAGE_FIRST_SECTION(nt);

    SIZE_T begin = 0;
    DWORD length = 0;
    for (INT i = 0; i < nt->FileHeader.NumberOfSections; i++) {
      char dot_data[] = {'.','d','a','t','a','\0'};
      if (llwythwr_strcmp(dot_data, (LPCSTR) sh->Name) == 0) {
        begin = (SIZE_T) ntdll->DllBase + sh->VirtualAddress;
        length = sh->Misc.VirtualSize;
        break;
      }
      ++sh;
    }

    size_t end = 0;
    for (DWORD i = 0; i < length - sizeof(SIZE_T); ++begin, ++i) {
      if (deinamig_memcmp((PVOID)begin, (PVOID)&node, sizeof(SIZE_T)) == 0) {
        end = begin;
        break;
      }
    }

    if (end == 0) {
      return NULL;
    }

    RTL_RB_TREE* tree = (RTL_RB_TREE*) end;
    if (tree->Root && tree->Min) {
      return tree;
    }
  }

  return NULL;
}

static BOOL add_base_address_entry(llwythwr_windows_api *api, void* module_base, LDR_DATA_TABLE_ENTRY_W8* entry) {
  //size_t ldpr_module_index_base = find_ldrp_module_index_base2();
  //LDR_DATA_TABLE_ENTRY_W8 *pLdrNode = CONTAINING_RECORD(ldpr_module_index_base, LDR_DATA_TABLE_ENTRY_W8, BaseAddressIndexNode);

  RTL_RB_TREE* mod_base_address_index = find_ldrp_module_index_base();
  if (mod_base_address_index == NULL)
    return FALSE;

  LDR_DATA_TABLE_ENTRY_W8* pLdrNode = (LDR_DATA_TABLE_ENTRY_W8*)((size_t)mod_base_address_index - offsetof(LDR_DATA_TABLE_ENTRY_W8, BaseAddressIndexNode));

  // Walk tree
  BOOL right = FALSE;
  do
  {
    if (module_base < pLdrNode->DllBase) {
      if (!pLdrNode->BaseAddressIndexNode.Left) {
        break;
      }
      pLdrNode = (LDR_DATA_TABLE_ENTRY_W8*)((size_t)pLdrNode->BaseAddressIndexNode.Left - offsetof(LDR_DATA_TABLE_ENTRY_W8, BaseAddressIndexNode));
    }

    else if (module_base > pLdrNode->DllBase) {
      if (!pLdrNode->BaseAddressIndexNode.Right) {
        right = TRUE;
        break;
      }
      pLdrNode = (LDR_DATA_TABLE_ENTRY_W8*)((size_t)pLdrNode->BaseAddressIndexNode.Right - offsetof(LDR_DATA_TABLE_ENTRY_W8, BaseAddressIndexNode));
    }
    else {
      //pLdrNode->DdagNode->LoadCount++;
      //pLdrNode->DdagNode->ReferenceCount++;
    }
  } while (TRUE);

  api->RtlRbInsertNodeEx((PRTL_RB_TREE) mod_base_address_index,
                         (PRTL_BALANCED_NODE) &pLdrNode->BaseAddressIndexNode, right,
                         (PRTL_BALANCED_NODE) &entry->BaseAddressIndexNode);

  return TRUE;
}

PLIST_ENTRY find_ldrp_hash_table(llwythwr_windows_api *api) {
  PEB_LDR_DATA_W8 *ldr = (PEB_LDR_DATA_W8*) NtCurrentTeb()->ProcessEnvironmentBlock->Ldr;
  LIST_ENTRY* head = &ldr->InInitializationOrderModuleList;
  LIST_ENTRY* entry = head->Flink;
  LDR_DATA_TABLE_ENTRY_W8* current_entry = NULL;
  LIST_ENTRY* list = NULL;

  do
  {
    current_entry = CONTAINING_RECORD(entry, LDR_DATA_TABLE_ENTRY_W8, InInitializationOrderLinks);
    entry = entry->Flink;

    if (current_entry->HashLinks.Flink == &current_entry->HashLinks) {
      continue;
    }

    list = current_entry->HashLinks.Flink;

    if (list->Flink == &current_entry->HashLinks) {
      ULONG hash;
      api->RtlHashUnicodeString(&current_entry->BaseDllName, TRUE, 0, &hash);
      hash &= (LDR_HASH_TABLE_ENTRIES - 1);
      list = (PLIST_ENTRY)((size_t) current_entry->HashLinks.Flink - hash * sizeof(LIST_ENTRY));
      break;
    }

    list = NULL;
  } while (head != entry);

  return list;
}

void insert_tail_list(PLIST_ENTRY ListHead, PLIST_ENTRY Entry) {
  PLIST_ENTRY Blink;
  Blink = ListHead->Blink;
  Entry->Flink = ListHead;
  Entry->Blink = Blink;
  Blink->Flink = Entry;
  ListHead->Blink = Entry;
}

#define RtlInitializeListEntry(entry) ((entry)->Blink = (entry)->Flink = (entry))

static BOOL add_hash_entry(llwythwr_windows_api *api, LDR_DATA_TABLE_ENTRY_W8* entry, ULONG hash) {
  RtlInitializeListEntry(&entry->HashLinks);

  PLIST_ENTRY ldrp_hash_table = find_ldrp_hash_table(api);
  if (ldrp_hash_table == NULL) {
    DPRINT("Could not find LdrpHashTable\n");
    return FALSE;
  }

  hash &= (LDR_HASH_TABLE_ENTRIES - 1);

  DPRINT("Using hash %lu as key in LdrpHashTable\n", hash);

  // insert into hash table
  insert_tail_list(&ldrp_hash_table[hash], &entry->HashLinks);
  return TRUE;
}

LDR_DATA_TABLE_ENTRY* llwythwr_link_library_win8(llwythwr_windows_api *api, void* module_base, const wchar_t* dll_name, const wchar_t* dll_path) {
  // Create ldr data table entry
  LDR_DATA_TABLE_ENTRY_W8* entry = create_ldr_entry(api, module_base, dll_name, dll_path);
  if (entry == NULL) {
    DPRINT("Could not create ldr data table entry for %S.\n", dll_name);
    return NULL;
  }
  DPRINT("LDR_DATA_TABLE_ENTRY created for %S with hash %lu.\n", dll_name, entry->BaseNameHashValue);

  // Add module’s base address to the base address index (LdrpModuleBaseAddressIndex)
  if (!add_base_address_entry(api, module_base, entry)) {
    DPRINT("Could not add entry for %S in LdrpModuleBaseAddressIndex.\n", dll_name);
    // TODO remove hash entry!
    api->LocalFree(entry->DdagNode);
    api->LocalFree(entry);
    return NULL;
  }
  DPRINT("Entry added to LdrpModuleBaseAddressIndex for %S.\n", dll_name);

  // Add module’s hash to the hash table (LdrpHashTable)
  if (!add_hash_entry(api, entry, entry->BaseNameHashValue)) {
    DPRINT("Could not add entry for %S to LdrpHashTable.\n", dll_name);
    api->LocalFree(entry->DdagNode);
    api->LocalFree(entry);
    return NULL;
  }
  DPRINT("Entry added to LdrpHashTable for %S.\n", dll_name);

  // insert into other lists
  PEB_LDR_DATA_W8 *ldr = (PEB_LDR_DATA_W8*) NtCurrentTeb()->ProcessEnvironmentBlock->Ldr;
  insert_tail_list(&ldr->InLoadOrderModuleList, &entry->InLoadOrderLinks);
  DPRINT("Entry added to InLoadOrderModuleList for %S.\n", dll_name);
  //insert_tail_list(&ldr->InMemoryOrderModuleList, &entry->InMemoryOrderLinks);
  //DPRINT("Entry added to InMemoryOrderModuleList for %S.\n", dll_name);
  insert_tail_list(&ldr->InInitializationOrderModuleList, &entry->InInitializationOrderLinks);
  DPRINT("Entry added to InInitializationOrderModuleList for %S.\n", dll_name);

  return (LDR_DATA_TABLE_ENTRY*) entry;
}

*/