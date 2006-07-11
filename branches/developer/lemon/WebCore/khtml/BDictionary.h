/******************************************************************************
/
/	File:			BDictionary.h
/
/	Description:	BDictionary class provides Hashtable.
/					Not thread safe.
/
/	Copyright 2005 (c) SYNRC Research Center.
/
******************************************************************************/

#ifndef _BE_DICTIONARY_H_
#define _BE_DICTIONARY_H_

#include <BeBuild.h>
#include <SupportDefs.h>
#include <OS.h>
#include <iostream.h>

class BDictionary;

class BDictionaryEntry {

public:
	BDictionaryEntry();
	~BDictionaryEntry();
	char  *Key;
	void  *Value;

private:
	
	BDictionaryEntry *Next;
	BDictionary *Parent;

	friend ostream &operator<<(ostream &stream, BDictionaryEntry &bkt);
	friend ostream &operator<<(ostream &stream, BDictionary &htable);
        friend class BDictionary;
};

class BDictionary {

public:

	enum {
		default_size = 1027 // default size
	};

	BDictionary(uint16 sz = BDictionary::default_size);
	~BDictionary();

	void AddItem(const char *key, void *value);
	void RemoveItem(const char *value);
	int Lookup(const char *value); // 0 if value not in table
	void *ItemAt(const char *value);
	bool IsStrings();
	void SetStrings(bool new_val);
	//bool IsDebug();
	//void SetDebug(bool new_state);

	unsigned short Size; // number of slots

private:

	BDictionaryEntry **Array;
	bool is_strings;
	bool show_debug;
	unsigned long hash(const char *value);
	BDictionaryEntry *lookup_bucket(const char *value);

	friend ostream &operator<<(ostream &stream, BDictionary &htable);
};

#endif /* */

