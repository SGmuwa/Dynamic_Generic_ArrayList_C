#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>


typedef struct ArrayList
{
	// Полезная нагрузка.
	void ** array;
	// Количество элементов, доступных для взаимодействия.
	size_t length;
	// Количество доступного места в листе.
	size_t capacity;
} * ArrayList;


/*
Создание в оперативной памяти места для листа.
*/
ArrayList ArrayList_malloc()
{
	ArrayList arrayList = (ArrayList) malloc(sizeof(struct ArrayList));

	if (arrayList == NULL)
		return NULL;

	arrayList->length = 0;
	arrayList->capacity = 1;

	arrayList->array = (void**) malloc(arrayList->capacity * sizeof(void*));

	if (arrayList->array == NULL)
	{
		free(arrayList);
		return NULL;
	}

	return arrayList;
}

/*
Очистка листа из памяти.
*/
void ArrayList_free(ArrayList arrayList)
{
	if (arrayList != NULL)
	{
		if(arrayList->array != NULL)
			free(arrayList->array);
		free(arrayList);
	}
} 

/*
Добавить элемент в лист.
Возвращает: код ошибки.
			1 - Нехватка памяти. Операция отменена.
*/
int ArrayList_add(ArrayList arrayList, void * element )
{
	if (arrayList->length == arrayList->capacity - 1)
	{
		void ** buffer;
		if ((buffer = realloc(arrayList->array, 2u * arrayList->capacity * sizeof(void*))) == NULL)
			return 1;
		arrayList->array = buffer;
		arrayList->capacity *= 2;
	}
	arrayList->array[arrayList->length++] = element;
	return 0;
}
