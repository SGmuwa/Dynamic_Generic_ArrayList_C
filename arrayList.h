#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>


typedef struct ArrayList
{
	// Полезная нагрузка.
	void * array;
	// Количество элементов, доступных для взаимодействия.
	size_t length;
	// Количество доступного места в листе.
	size_t capacity;
	// Размер одного элемента листа. Размер хранимого типа.
	size_t sizeType;
} * ArrayList;


/*
Создание в оперативной памяти места для листа.
size_t sizeType - размер одного элемента листа. Размер хранимого типа.
*/
ArrayList ArrayList_malloc(size_t sizeType)
{
	if (sizeType == 0)
		return NULL; // Not support.

	ArrayList arrayList = (ArrayList) malloc(sizeof(struct ArrayList));

	if (arrayList == NULL)
		return NULL;

	arrayList->length = 0;
	arrayList->capacity = 1;
	arrayList->sizeType = sizeType;

	arrayList->array = (void*) malloc(arrayList->capacity * sizeType);

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
Устанавливает значение элемента по индексу.
Возвращает: код ошибки.
			0 - Значение установлено.
			1 - Отправленный лист имеет нулевой указатель.
			2 - Указатель массива листа равен NULL.
			3 - Индекс вне диапазона листа.
			4 - Копирование с области NULL запрещено.
			5 - memcpy_s обнаружил ошибку.
*/
int ArrayList_set(ArrayList this, size_t index, void * element)
{
	if (this == NULL)
		return 1;
	if (this->array == NULL)
		return 2;
	if (index >= this->length)
		return 3;
	if (element == NULL)
		return 4;
#ifdef _MSC_VER
	if (memcpy_s((char*)this->array + index * this->sizeType, (this->capacity - index) * this->sizeType, element, this->sizeType))
		return 5;
#else
	memcpy((char*)this->array + index * this->sizeType, element, this->sizeType);
#endif // _MSC_VER
	return 0;
}

/*
Добавить элемент в лист.
Возвращает: код ошибки.
			1 - Нехватка памяти. Операция отменена.
			2 - Операция присваивания оказалась с ошибкой.
*/
int ArrayList_add(ArrayList this, void * element)
{
	if (this->length == this->capacity - 1)
	{
		void * buffer;
		if ((buffer = realloc(this->array, 2u * this->capacity * this->sizeType)) == NULL)
			return 1;
		this->array = buffer;
		this->capacity *= 2;
	}
	if (ArrayList_set(this, this->length++, element))
	{
		this->length--;
		return 2;
	}
	return 0;
}
