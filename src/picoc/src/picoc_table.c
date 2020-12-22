/* picoc hash table module. This hash table code is used for both symbol tables
 * and the shared string table. */

#include "picoc/picoc_interpreter.h"

static unsigned int TableHash(const char* Key, int Len);
static struct TableEntry* TableSearch(struct Table* Tbl, const char* Key, int* AddAt);
static struct TableEntry* TableSearchIdentifier(struct Table* Tbl, const char* Key, int Len, int* AddAt);

/* initialize the shared string system */
void TableInit(Picoc* pc) {
    TableInitTable(&pc->StringTable, &pc->StringHashTable[0], PICOC_CONFIG_STRING_TABLE_SIZE, true);
    pc->StrEmpty = TableStrRegister(pc, "");
}

/* hash function for strings */
unsigned int TableHash(const char* Key, int Len) {
    unsigned int hash = Len;
    int offset;
    int count;

    for(count = 0, offset = 8; count < Len; count++, offset += 7) {
        if(offset > sizeof(unsigned int) * 8 - 7) {
            offset -= sizeof(unsigned int) * 8 - 6;
        }

        hash ^= *Key++ << offset;
    }

    return hash;
}

/* initialize a table */
void TableInitTable(struct Table* Tbl, struct TableEntry** HashTable, int Size, int OnHeap) {
    Tbl->Size = Size;
    Tbl->OnHeap = OnHeap;
    Tbl->HashTable = HashTable;
    memset((void*)HashTable, '\0', sizeof(struct TableEntry*) * Size);
}

/* check a hash table entry for a key */
struct TableEntry* TableSearch(struct Table* Tbl, const char* Key, int* AddAt) {
    /* shared strings have unique addresses so we don't need to hash them */
    int hashValue = ((unsigned long)Key) % Tbl->Size;
    struct TableEntry* entry = NULL;

    for(entry = Tbl->HashTable[hashValue]; entry != NULL; entry = entry->Next) {
        if(entry->p.v.Key == Key) {
            return entry; /* found */
        }
    }

    *AddAt = hashValue; /* didn't find it in the chain */
    return NULL;
}

/* set an identifier to a value. returns FALSE if it already exists.
 * Key must be a shared string from TableStrRegister() */
int TableSet(Picoc* pc, struct Table* Tbl, char* Key, struct Value* Val, const char* DeclFileName, int DeclLine,
             int DeclColumn) {
    int addAt = 0;
    struct TableEntry* foundEntry = TableSearch(Tbl, Key, &addAt);

    if(foundEntry == NULL) { /* add it to the table */
        struct TableEntry* newEntry = VariableAlloc(pc, NULL, sizeof(struct TableEntry), Tbl->OnHeap);
        newEntry->DeclFileName = DeclFileName;
        newEntry->DeclLine = DeclLine;
        newEntry->DeclColumn = DeclColumn;
        newEntry->p.v.Key = Key;
        newEntry->p.v.Val = Val;
        newEntry->Next = Tbl->HashTable[addAt];
        Tbl->HashTable[addAt] = newEntry;
        return true;
    }

    return false;
}

/* find a value in a table. returns FALSE if not found.
 * Key must be a shared string from TableStrRegister() */
int TableGet(struct Table* Tbl, const char* Key, struct Value** Val, const char** DeclFileName, int* DeclLine,
             int* DeclColumn) {
    int addAt = 0;
    struct TableEntry* foundEntry = TableSearch(Tbl, Key, &addAt);
    if(foundEntry == NULL) {
        return false;
    }

    *Val = foundEntry->p.v.Val;

    if(DeclFileName != NULL) {
        *DeclFileName = foundEntry->DeclFileName;
        *DeclLine = foundEntry->DeclLine;
        *DeclColumn = foundEntry->DeclColumn;
    }

    return true;
}

/* remove an entry from the table */
struct Value* TableDelete(Picoc* pc, struct Table* Tbl, const char* Key) {
    /* shared strings have unique addresses so we don't need to hash them */
    int hashValue = ((unsigned long)Key) % Tbl->Size;
    struct TableEntry** entryPtr;

    for(entryPtr = &Tbl->HashTable[hashValue]; *entryPtr != NULL; entryPtr = &(*entryPtr)->Next) {
        if((*entryPtr)->p.v.Key == Key) {
            struct TableEntry* DeleteEntry = *entryPtr;
            struct Value* Val = DeleteEntry->p.v.Val;
            *entryPtr = DeleteEntry->Next;
            HeapFreeMem(pc, DeleteEntry);

            return Val;
        }
    }

    return NULL;
}

/* check a hash table entry for an identifier */
struct TableEntry* TableSearchIdentifier(struct Table* Tbl, const char* Key, int Len, int* AddAt) {
    int hashValue = TableHash(Key, Len) % Tbl->Size;
    struct TableEntry* entry = NULL;

    for(entry = Tbl->HashTable[hashValue]; entry != NULL; entry = entry->Next) {
        if(strncmp(&entry->p.Key[0], (char*)Key, Len) == 0 && entry->p.Key[Len] == '\0') {
            return entry; /* found */
        }
    }

    *AddAt = hashValue; /* didn't find it in the chain */
    return NULL;
}

/* set an identifier and return the identifier. share if possible */
char* TableSetIdentifier(Picoc* pc, struct Table* Tbl, const char* Ident, int IdentLen) {
    int addAt = 0;
    struct TableEntry* foundEntry = TableSearchIdentifier(Tbl, Ident, IdentLen, &addAt);

    if(foundEntry != NULL) {
        return &foundEntry->p.Key[0];
    }

    /* add it to the table - we economise by not allocating
            the whole structure here */
    struct TableEntry* newEntry =
        HeapAllocMem(pc, sizeof(struct TableEntry) - sizeof(union TableEntryPayload) + IdentLen + 1);
    if(newEntry == NULL) {
        ProgramFailNoParser(pc, "(TableSetIdentifier) out of memory");
    }

    strncpy((char*)&newEntry->p.Key[0], (char*)Ident, IdentLen);
    newEntry->p.Key[IdentLen] = '\0';
    newEntry->Next = Tbl->HashTable[addAt];
    Tbl->HashTable[addAt] = newEntry;
    return &newEntry->p.Key[0];
}

/* register a string in the shared string store */
char* TableStrRegister2(Picoc* pc, const char* Str, int Len) {
    return TableSetIdentifier(pc, &pc->StringTable, Str, Len);
}

char* TableStrRegister(Picoc* pc, const char* Str) { return TableStrRegister2(pc, Str, strlen((char*)Str)); }

/* free all the strings */
void TableStrFree(Picoc* pc) {
    int count = 0;
    struct TableEntry* entry = NULL;
    struct TableEntry* nextEntry = NULL;

    for(count = 0; count < pc->StringTable.Size; count++) {
        for(entry = pc->StringTable.HashTable[count]; entry != NULL; entry = nextEntry) {
            nextEntry = entry->Next;
            HeapFreeMem(pc, entry);
        }
    }
}
