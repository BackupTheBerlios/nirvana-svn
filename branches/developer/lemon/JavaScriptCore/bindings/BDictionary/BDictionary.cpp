/******************************************************************************
/
/	File:			BDictionary.cpp
/
/	Description:	BDictionary class provides classical Hashtable storage for object.
/					Not thread safe.
/
/	Copyright 2005 (c) SYNRC Research Center.
/
******************************************************************************/

#include <iostream.h>
#include <fstream.h> 
#include <string.h>
#include <assert.h>
#include "BDictionary.h"

BDictionaryEntry::BDictionaryEntry() :
    Key(NULL),
    Value(NULL),
    Next(NULL),
    Parent(NULL)
{
}

BDictionaryEntry::~BDictionaryEntry()
{
    delete [] Value;
}

ostream & operator<<(ostream &stream, BDictionaryEntry &bucket)
{
	stream << "[" << bucket.Key << ",";

	if (bucket.Parent == NULL || !bucket.Parent->IsStrings()) stream << (void *) bucket.Value;
	else stream << (char *) bucket.Value;

	stream << "]";

	return stream;
}

BDictionary::BDictionary(uint16 sz) : Size(sz)
{
	uint16 i;
	assert(sz != 0);
	is_strings = false;
	show_debug = false;
	Array = new BDictionaryEntry*[Size + 1];
	assert(Array != NULL);
	for (i = 0; i < Size; i++) Array[i] = NULL;
}

BDictionary::~BDictionary()
{
	uint16 i;

	if (show_debug) cerr << "# BDictionary::~BDictionary(Size = " << Size << ")\n";

	for (i = 0; i < Size; i++) {
		BDictionaryEntry *entry = Array[i];

		while (entry) {
			BDictionaryEntry *freeme;
			freeme = entry;
			entry = entry->Next;
			delete freeme;
		}
	}

	delete [] Array;
}

void BDictionary::AddItem(const char *key, void *value)
{
	assert(key != NULL);

	long chain = hash(key);

	BDictionaryEntry *entry = lookup_bucket(key);

	if (show_debug) {

		cerr << "# BDictionary::AddItem(\"" << key << "\", ";

		if (is_strings) cerr << "\"" << (char *) value << "\"";
		else cerr << "(void *)" << (void *) value;

		cerr << ") into chain [" << chain << "]";
	}

	if (entry != NULL) {                               // update existing

		if (show_debug) cerr << " <updating>";

		entry->Value = value;

	} else {

		if (show_debug) cerr << " <adding>";

		BDictionaryEntry *new_entry = new BDictionaryEntry();
		assert(new_entry != NULL);
		new_entry->Key = new char[strlen(key) + 1];
		assert(new_entry->Key != NULL);
		strcpy(new_entry->Key, key);
		new_entry->Value = value;
		new_entry->Parent = this;
		new_entry->Next = Array[chain];
		Array[chain] = new_entry;
	}

	if (show_debug) cerr << "\n";

}

int BDictionary::Lookup(const char *key)
{
    assert(key != NULL);

    if (show_debug) cerr << "# BDictionary::Lookup(" << key << ")\n";

    return (lookup_bucket(key) != NULL);
}

void *BDictionary::ItemAt(const char *key)
{
	BDictionaryEntry *entry;
	void *retval;

	assert(key != NULL);

	if (show_debug) cerr << "# BDictionary::ItemAt(" << key << ") => ";

	entry = lookup_bucket(key);

	if (entry != NULL) retval = (void *) entry->Value;
	else retval = NULL;

	if (show_debug) {

		if (retval == NULL) cerr << "<NULL>";
		else if (is_strings) cerr << "\"" << (char *) retval << "\"";
		else cerr << "(void *)" << (void *) retval;
		cerr << "\n";
	}

	return retval;
}

void BDictionary::RemoveItem(const char *key)
{
	BDictionaryEntry *entry;
	unsigned long chain;

	assert(key != NULL);
	chain = hash(key);
	entry = lookup_bucket(key);
	assert(entry != NULL);

	if (show_debug) cerr << "# BDictionary::RemoveItem(" << key << ")\n";

	if (Array[chain] == entry) {
		Array[chain] = entry->Next;
		entry->Next = NULL;
		delete entry;
    } else {
        BDictionaryEntry *tmp = Array[chain];
		while (tmp->Next != NULL && tmp->Next != entry) tmp = tmp->Next;
        assert(tmp != NULL);
        tmp->Next = tmp->Next->Next;
        delete entry;
    }
}

bool BDictionary::IsStrings()
{
	return is_strings;
}

void BDictionary::SetStrings(bool value_is_strings)
{
	is_strings = value_is_strings;
}

ostream &operator<<(ostream &stream, BDictionary &htable)
{
	int                 how_many;
	unsigned short      i;

	stream << "BDictionary Size = " << htable.Size;

	if (htable.IsStrings()) stream << ", value interpreted as a (char *)";

    stream << "\n";

    how_many = 0;

    for (i = 0; i < htable.Size; i++) { // for each chain
        BDictionaryEntry *entry = htable.Array[i];

        if (entry == NULL) continue;
        else {

			stream << "[";

			if (i < 10) stream << "  ";
			else if (i < 100) stream << " ";

			stream << i << "]";
		}

        while (entry != NULL) {
			how_many = 0;
			stream << "=>" << *entry;
			entry = entry->Next;
		}

		stream << "\n";
	}

	if (!how_many) stream << "HashTable is empty";
    else stream << "HashTable contains " << how_many << " items.";

    stream << "\n";

    return stream;
}

BDictionaryEntry *BDictionary::lookup_bucket(const char *key)
{
	assert(key != NULL);
	BDictionaryEntry *bp;
	for (bp = Array[hash(key)]; bp != NULL; bp = bp->Next) {
		if (strcmp(bp->Key, key) == 0)
			return bp;
    }
    return NULL;
}

unsigned long BDictionary::hash(const char *key)
{
	const char *p; // Steps thru the token
	unsigned long hval; // hash value
	unsigned long g; // intermediate hash value

	assert(key != NULL);

	hval = 0;

	for (p = key; *p != '\0'; p++) {
		hval = (hval << 4) + (*p);
		if ((g = hval & 0xf0000000L) != 0) {
			hval = hval ^ (g >> 24);
			hval = hval ^ g;
		}
	}
    return (hval % Size);
}
