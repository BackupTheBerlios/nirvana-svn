#include <OS.h>
#include <SupportKit.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "BDictionary.h"

int main(void) {
	char *one = "1";
	char *two = "2";
	BDictionary *dict = new BDictionary();
	BDictionary *dict2 = new BDictionary();
	printf("BDictionary Test.\n");
	dict->AddItem("one", one);
	dict->AddItem("two", two);
	char *list = (char *)dict->ItemAt("one");
	printf("%s\n",list);
	list = (char *)dict->ItemAt("two");
	if (dict == dict2) printf("equals");
	printf("%s\n",list);
	printf("%s\n",BDictionaryEntry::ClassName);
	return 0;
}
