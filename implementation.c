#include "arrayList.h"

int main(int argc, char **argv) {

	//ArrayList *arrayList;

	ArrayList list = ArrayList_malloc(sizeof(int));
	
	int one = 1;
	int two = 2;
	int three = 3;
	int four = 4;

	ArrayList_add(list, &one);
	ArrayList_add(list, &two);
	ArrayList_add(list, &three);
	ArrayList_add(list, &four);

	one = 5;
	two = 6;
	three = 7;
	four = 8;

	for (size_t i = 0; i < list->length; i++)
		printf("%d ", ((int*)list->array)[i]);
	ArrayList_free(list);
}
