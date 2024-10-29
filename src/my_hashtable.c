#include "my_hashtable.h"
#include <stddef.h>
HashTable *createHashTable()
{
    HashTable *ht = malloc(sizeof(HashTable));
    ht->table = malloc(sizeof(Entry *) * TABLESIZE);
    for (int i = 0; i < TABLESIZE; i++)
    {
        ht->table[i] = NULL;
    }
    return ht;
}
unsigned int hash(const char *key)
{
    unsigned int hashValue = 0;
    while (*key)
    {
        hashValue = (hashValue << 5) + *key++;
    }
    return hashValue % TABLESIZE;
}
void insertHashTable(HashTable *ht, const char *key, struct Connection *value)
{
    unsigned int index = hash(key);
    Entry *newEntry = malloc(sizeof(Entry));
    newEntry->key = strdup(key);
    newEntry->value = value;
    newEntry->next = NULL;

    Entry *entry = ht->table[index];
    while (entry)
    {
        entry = entry->next;
        if (entry != NULL && entry->next == NULL)
            break;
    }
    if (entry != NULL)
        entry->next = newEntry;
    else
        ht->table[index] = newEntry;
}
struct Connection *searchHashTable(HashTable *ht, const char *key)
{
    unsigned int index = hash(key);
    Entry *entry = ht->table[index];
    while (entry)
    {
        if (strcmp(entry->key, key) == 0)
        {
            return entry->value;
        }
        entry = entry->next;
    }
    return NULL; // 未找到
}
void freeHashTable(HashTable *ht)
{
    for (int i = 0; i < TABLESIZE; i++)
    {
        Entry *entry = ht->table[i];
        while (entry)
        {
            Entry *temp = entry;
            entry = entry->next;
            free(temp->key);
            free(temp->value);
            free(temp);
        }
    }
    free(ht->table);
    free(ht);
}
