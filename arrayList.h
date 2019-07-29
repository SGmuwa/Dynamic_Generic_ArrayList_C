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


// Перечисление возможных ошибок.
enum ArrayList_err
{
	// Ошибка отсутсвует.
	ArrayList_err_ok = 0,
	// Указатель на лист равен NULL.
	ArrayList_err_listNull,
	// Указатель на массив листа равен NULL.
	ArrayList_err_list_arrayNull,
	// Не хватает памяти.
	ArrayList_err_outOfMemory,
	// Для текущей архитектуры такой размер листа невозможен.
	ArrayList_err_lengthIsTooBig,
	// Индекс вне диапазона списка.
	ArrayList_err_outsideList,
	// Копирование с области NULL запрещено.
	ArrayList_err_copyFromNull,
	// Копирование в область NULL не поддерживается.
	ArrayList_err_copyToNull,
	// Неизвестная ошибка при выполнении memcpy_s.
	ArrayList_err_memcpy_s,
	// Ошибка связанная с увеличением размеров списка. Проблема внутри ArrayList_prepareLength.
	ArrayList_err_prepareLength,
	// Ошибка связанная с присваиванием элемента. Проблема внутри ArrayList_set.
	ArrayList_err_set,
	// Неизвестная ошибка при выполнении memmove_s.
	ArrayList_err_memmove_s,
	// Ошибка связанная с добавлением элемента в конец списка.
	ArrayList_err_addLast
};


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
Проверяет, чтобы места хватало в листе под новое количество элементов.
Если в листе не хватает места, то выделяется дополнительная память.
ArrayList this - Лист, который надо подготовить.
size_t needLength - Указывает, сколько элементов нужно листу.
Возвращает: код ошибки ArrayList_err_listNull, ArrayList_err_list_arrayNull, ArrayList_err_outOfMemory, ArrayList_err_ok
*/
enum ArrayList_err ArrayList_prepareCapacity(ArrayList this, size_t needLength)
{
	if (this == NULL)
		return ArrayList_err_listNull;
	if (this->array == NULL)
		return ArrayList_err_list_arrayNull;
	if (needLength < SIZE_MAX / 2) // Интересно, это условие в реальности может быть ложью?
		needLength = needLength * 2u;
	if (needLength > this->capacity)
	{
		void * buffer;
		if ((buffer = realloc(this->array, needLength * this->sizeType)) == NULL)
			return ArrayList_err_outOfMemory;
		this->array = buffer;
		this->capacity = needLength;
	}
	return ArrayList_err_ok;
}

/*
Безопасно изменяет длинну листа.
ArrayList this - Лист, в котором надо прибавить длинну.
size_t addToLength - Слагаемое. Сколько нужно прибавить к текущей длине?
Возвращает: код ошибки ArrayList_err_listNull, ArrayList_err_list_arrayNull, ArrayList_err_lengthIsTooBig, ArrayList_err_outOfMemory, ArrayList_err_ok
*/
enum ArrayList_err ArrayList_prepareLength(ArrayList this, size_t addToLength)
{
	if (this == NULL)
		return ArrayList_err_listNull;
	if (this->array == NULL)
		return ArrayList_err_list_arrayNull;
	const size_t result = this->length + addToLength;
	if (result < this->length)
		return ArrayList_err_lengthIsTooBig;
	if (ArrayList_prepareCapacity(this, result))
		return ArrayList_err_outOfMemory;
	return ArrayList_err_ok;
}

/*
Приравнивает размер вмещения элемента листа к количеству элементов.
Эту функцию необходимо вызывать в случае, если Вы
не собираетесь добавлять новые данные в лист.
ArrayList this - лист, который надо очистить.
Возвращает: код ошибки ArrayList_err_listNull, ArrayList_err_list_arrayNull, ArrayList_err_ok, ArrayList_err_outOfMemory
*/
enum ArrayList_err ArrayList_removeTrash(ArrayList this)
{
	if (this == NULL)
		return ArrayList_err_listNull;
	if (this->array == NULL)
		return ArrayList_err_list_arrayNull;
	if (this->length == this->capacity)
		return ArrayList_err_ok;
	void * buffer;
	if ((buffer = realloc(this->array, this->length * this->sizeType)) == NULL)
		return ArrayList_err_outOfMemory;
	this->array = buffer;
	this->capacity = this->length;
	return ArrayList_err_ok;
}

/*
Устанавливает значение элемента по индексу.
ArrayList this - Список, в который надо установить значение элемента.
size_t index - Номер ячейки, в которой надо заменить элемент.
void * element - Указатель на элемент-источник, откуда должно происходить копирование.
Возвращает: код ошибки ArrayList_err_ok, ArrayList_err_memcpy_s, ArrayList_err_copyFromNull, ArrayList_err_outsideList, ArrayList_err_list_arrayNull, ArrayList_err_listNull
*/
enum ArrayList_err ArrayList_set(ArrayList this, size_t index, void * element)
{
	if (this == NULL)
		return ArrayList_err_listNull;
	if (this->array == NULL)
		return ArrayList_err_list_arrayNull;
	if (index >= this->length)
		return ArrayList_err_outsideList;
	if (element == NULL)
		return ArrayList_err_copyFromNull;
#ifdef _MSC_VER
	if (memcpy_s((char*)this->array + index * this->sizeType, (this->capacity - index) * this->sizeType, element, this->sizeType))
		return ArrayList_err_memcpy_s;
#else
	memcpy((char*)this->array + index * this->sizeType, element, this->sizeType);
#endif // _MSC_VER
	return ArrayList_err_ok;
}

/*
Получает значение элемента по индексу.
Возвращает: код ошибки ArrayList_err_ok, ArrayList_err_memcpy_s, ArrayList_err_copyToNull, ArrayList_err_outsideList, ArrayList_err_list_arrayNull, ArrayList_err_listNull
*/
enum ArrayList_err ArrayList_get(ArrayList this, size_t index, void * output)
{
	if (this == NULL)
		return ArrayList_err_listNull;
	if (this->array == NULL)
		return ArrayList_err_list_arrayNull;
	if (index >= this->length)
		return ArrayList_err_outsideList;
	if (output == NULL)
		return ArrayList_err_copyToNull;
#ifdef _MSC_VER
	if (memcpy_s(output, (this->capacity - index) * this->sizeType, (char*)this->array + index * this->sizeType, this->sizeType))
		return ArrayList_err_memcpy_s;
#else
	memcpy(output, (char*)this->array + index * this->sizeType, this->sizeType);
#endif // _MSC_VER
	return ArrayList_err_ok;
}


/*
Добавить элемент в список в последнюю позицию.
ArrayList this - Список, куда надо добавить элемент.
void * element - Указатель на элемент, который надо поместить в список последним.
Возвращает: код ошибки ArrayList_err_ok, ArrayList_err_set, ArrayList_err_prepareLength, ArrayList_err_copyFromNull, ArrayList_err_list_arrayNull, ArrayList_err_listNull
*/
enum ArrayList_err ArrayList_addLast(ArrayList this, void * element)
{
	if (this == NULL)
		return ArrayList_err_listNull;
	if (this->array == NULL)
		return ArrayList_err_list_arrayNull;
	if (element == NULL)
		return ArrayList_err_copyFromNull;
	if (ArrayList_prepareLength(this, 1))
		return ArrayList_err_prepareLength;
	if (ArrayList_set(this, this->length++, element))
	{
		this->length--;
		return ArrayList_err_set;
	}
	return ArrayList_err_ok;
}

/*
Помещает элемент на указанный индекс. Тот элемент, который раньше был по этому
индексу перемещается вперёд, как и последующие элементы.
ArrayList this - Лист, в который надо добавить элемент.
size_t index - Место, куда надо добавить элемент.
void * element - Указатель на элемент, который надо добавить.
Возвращает: код ошибки ArrayList_err_ok, ArrayList_err_addLast, ArrayList_err_set, ArrayList_err_memmove_s, ArrayList_err_outsideList, ArrayList_err_prepareLength, ArrayList_err_copyFromNull, ArrayList_err_list_arrayNull, ArrayList_err_listNull
*/
enum ArrayList_err ArrayList_add(ArrayList this, size_t index, void * element)
{
	if (this == NULL)
		return ArrayList_err_listNull;
	if (this->array == NULL)
		return ArrayList_err_list_arrayNull;
	if (element == NULL)
		return ArrayList_err_copyFromNull;
	if (ArrayList_prepareLength(this, 1))
		return ArrayList_err_prepareLength;
	if (index > this->length + 1)
		return ArrayList_err_outsideList;
	if (index < this->length)
	{ // Передвигать последние элементы нужно только тогда, когда последующие элементы существуют.
#ifdef _MSC_VER
		if (memmove_s(
			(char*)this->array + (index + 1) * this->sizeType,
			(this->capacity - (index + 1)) * this->sizeType,
			(char*)this->array + index * this->sizeType,
			(this->length - index) * this->sizeType))
			return ArrayList_err_memmove_s; // error.
#else
		memmove(
			(char*)this->array + (index + 1) * this->sizeType,
			(char*)this->array + index * this->sizeType,
			(this->length - index) * this->sizeType);
#endif // _MSC_VER
		if (ArrayList_set(this, index, element))
			return ArrayList_err_set;
		this->length++;
	}
	else if (ArrayList_addLast(this, element))
		return ArrayList_err_addLast;
	return ArrayList_err_ok;
}

/*
Удаляет элемент из листа по его индексу.
ArrayList this - Лист, из которого нужно исключить элемент.
size_t index - Номер элемента, который необходимо освободить.
Возвращает: код ошибки ArrayList_err_ok, ArrayList_err_memmove_s, ArrayList_err_outsideList, ArrayList_err_list_arrayNull, ArrayList_err_listNull
*/
enum ArrayList_err ArrayList_remove(ArrayList this, size_t index)
{
	if (this == NULL)
		return ArrayList_err_listNull;
	if (this->array == NULL)
		return ArrayList_err_list_arrayNull;
	if (index >= this->length)
		return ArrayList_err_outsideList;
	if (index != this->length - 1)
	{ // Передвигать последние элементы нужно только тогда, когда последние элементы существуют.
#ifdef _MSC_VER
		if (memmove_s(
			(char*)this->array + index * this->sizeType,
			(this->capacity - index) * this->sizeType,
			(char*)this->array + (index + 1)*this->sizeType,
			(this->length - (index + 1)) * this->sizeType))
			return ArrayList_err_memmove_s; // error.
#else
		memmove(
			(char*)this->array + index * this->sizeType,
			(char*)this->array + (index + 1)*this->sizeType,
			(this->length - (index + 1)) * this->sizeType);
#endif // _MSC_VER
	}
	this->length--;
	return ArrayList_err_ok;
}
