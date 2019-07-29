#include "arrayList.h"

size_t printList(ArrayList this)
{
	for (size_t i = 0; i < this->length; i++)
	{
		int buffer;
		if (ArrayList_get(this, i, &buffer))
			return i + 1;
		printf("%d ", buffer);
	}
	printf("\n");
	return 0;
}

int main(int argc, char **argv) {

	//ArrayList *arrayList;

	ArrayList list = ArrayList_malloc(sizeof(int));
	
	int one = 1;
	int two = 2;
	int three = 3;
	int four = 4;

	ArrayList_addLast(list, &one);
	ArrayList_addLast(list, &two);
	ArrayList_addLast(list, &three);
	ArrayList_addLast(list, &four);

	one = 5;
	two = 6;
	three = 7;
	four = 8;

	printf("1...4: ");
	printList(list);

	ArrayList_remove(list, 2);

	printf("remove index 2: ");
	printList(list);

	ArrayList_add(list, 2, &three);

	printf("put 7 to index 2: ");
	printList(list);

	ArrayList_add(list, 2, &three);

	printf("put 7 to index 2: ");
	printList(list);

	ArrayList_addLast(list, &four);

	printf("put 8 to index last: ");
	printList(list);

	ArrayList_remove(list, 5);

	printf("remove index 5: ");
	printList(list);

	ArrayList_remove(list, 0);

	printf("remove index 0: ");
	printList(list);

	printf("before clear capacity = %zu\n", list->capacity);
	ArrayList_removeTrash(list);
	printf("after  clear capacity = %zu\n", list->capacity);

	ArrayList_free(list);
	return 0;
}
