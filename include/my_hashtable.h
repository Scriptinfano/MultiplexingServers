
#pragma once
#define TABLESIZE 1024
struct Connection;
typedef struct Entry
{
    char *key;
    struct Connection *value;
    struct Entry *next; // 链表法处理冲突
} Entry;
typedef struct HashTable
{
    Entry **table;
} HashTable;
HashTable *createHashTable();
unsigned int hash(const char *key);
void insertHashTable(HashTable *ht, const char *key, struct Connection *value);
struct Connection *searchHashTable(HashTable *ht, const char *key);
void freeHashTable(HashTable *ht);
