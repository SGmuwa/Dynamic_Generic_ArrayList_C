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
Проверяет, чтобы места хватало в листе под новое количество элементов.
Если в листе не хватает места, то выделяется дополнительная память.
ArrayList this - Лист, который надо подготовить.
size_t needLength - Указывает, сколько элементов нужно листу.
Возвращает: код ошибки
						0 - Место подготовлено.
						1 - Нехватка памяти.
						2 - Указатель на лист равен NULL.
						3 - Указатель на массив листа равен NULL.
*/
int ArrayList_prepareCapacity(ArrayList this, size_t needLength)
{
	if (this == NULL)
		return 2;
	if (this->array == NULL)
		return 3;
	if (needLength < SIZE_MAX / 2) // Интересно, это условие в реальности может быть ложью?
		needLength = needLength * 2u;
	if (needLength > this->capacity)
	{
		void * buffer;
		if ((buffer = realloc(this->array, needLength * this->sizeType)) == NULL)
			return 1;
		this->array = buffer;
		this->capacity = needLength;
	}
	return 0;
}

/*
Безопасно изменяет длинну листа.
ArrayList this - Лист, в котором надо прибавить длинну.
size_t addToLength - Слагаемое. Сколько нужно прибавить к текущей длине?
Возвращает: код ошибки
						0 - Размер листа подготовлен. Вы можете выполнить list->length += addToLength.
						1 - Для текущей архитектуры такой размер листа невозможен.
						2 - Нехватка памяти.
						3 - Указатель на лист равен NULL.
						4 - Указатель на массив листа равен NULL.

*/
int ArrayList_prepareLength(ArrayList this, size_t addToLength)
{
	if (this == NULL)
		return 3;
	if (this->array == NULL)
		return 4;
	const size_t result = this->length + addToLength;
	if (result < this->length)
		return 1;
	if (ArrayList_prepareCapacity(this, result))
		return 2;
	return 0;
}

/*
Приравнивает размер вмещения элемента листа к количеству элементов.
Эту функцию необходимо вызывать в случае, если Вы
не собираетесь добавлять новые данные в лист.
ArrayList this - лист, который надо очистить.
Возвращает: код ошибки
						0 - capacity приравнен к length.
						1 - Нехватка памяти. realloc вернул NULL.
						2 - Указатель на лист равен NULL.
						3 - Указатель на массив листа равен NULL.
*/
int ArrayList_removeTrash(ArrayList this)
{
	if (this == NULL)
		return 2;
	if (this->array == NULL)
		return 3;
	if (this->length == this->capacity)
		return 0;
	void * buffer;
	if ((buffer = realloc(this->array, this->length * this->sizeType)) == NULL)
		return 1;
	this->array = buffer;
	this->capacity = this->length;
	return 0;
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
Получает значение элемента по индексу.
Возвращает: код ошибки.
			0 - Значение получено.
			1 - Отправленный лист имеет нулевой указатель.
			2 - Указатель массива листа равен NULL.
			3 - Индекс вне диапазона листа.
			4 - Копирование в область NULL запрещено.
			5 - memcpy_s обнаружил ошибку.
*/
int ArrayList_get(ArrayList this, size_t index, void * output)
{
	if (this == NULL)
		return 1;
	if (this->array == NULL)
		return 2;
	if (index >= this->length)
		return 3;
	if (output == NULL)
		return 4;
#ifdef _MSC_VER
	if (memcpy_s(output, (this->capacity - index) * this->sizeType, (char*)this->array + index * this->sizeType, this->sizeType))
		return 5;
#else
	memcpy(output, (char*)this->array + index * this->sizeType, this->sizeType);
#endif // _MSC_VER
	return 0;
}


/*
Добавить элемент в лист.
Возвращает: код ошибки.
			1 - Нехватка памяти. Операция отменена.
			2 - Операция присваивания оказалась с ошибкой.
			3 - Указатель на лист равен NULL.
			4 - Указатель на массив в листе равен NULL.
			5 - element == NULL не поддерживается.
*/
int ArrayList_addLast(ArrayList this, void * element)
{
	if (this == NULL)
		return 3;
	if (this->array == NULL)
		return 4;
	if (element == NULL)
		return 5;
	if (ArrayList_prepareLength(this, 1))
		return 1;
	if (ArrayList_set(this, this->length++, element))
	{
		this->length--;
		return 2;
	}
	return 0;
}

/*
Помещает элемент на указанный индекс. Тот элемент, который раньше был по этому
индексу перемещается вперёд, как и последующие элементы.
ArrayList this - Лист, в который надо добавить элемент.
size_t index - Место, куда надо добавить элемент.
void * element - Указатель на элемент, который надо добавить.
Возвращает: код ошибки
						0 - Элемент добавлен.
						1 - Не удалось выделить или подготовить память под новый элемент.
						2 - Не удалось переместить элемент (memmove_s != 0).
						3 - Ошибка при добавлении элемента в последнюю позицию.
						4 - Указатель на лист равен NULL.
						5 - Указатель в листе на массив равен NULL.
						6 - Индекс слишком велик.
						7 - element == NULL не поддерживается.
*/
int ArrayList_add(ArrayList this, size_t index, void * element)
{
	if (this == NULL)
		return 4;
	if (this->array == NULL)
		return 5;
	if (element == NULL)
		return 7;
	if (ArrayList_prepareLength(this, 1))
		return 1;
	if (index > this->length + 1)
		return 6;
	if (index < this->length)
	{ // Передвигать последние элементы нужно только тогда, когда последующие элементы существуют.
#ifdef _MSC_VER
		if (memmove_s(
			(char*)this->array + (index + 1) * this->sizeType,
			(this->capacity - (index + 1)) * this->sizeType,
			(char*)this->array + index * this->sizeType,
			(this->length - index) * this->sizeType))
			return 2; // error.
#else
		memmove(
			(char*)this->array + (index + 1) * this->sizeType,
			(char*)this->array + index * this->sizeType,
			(this->length - index) * this->sizeType);
#endif // _MSC_VER
		ArrayList_set(this, 2, element);
		this->length++;
	}
	else if (ArrayList_addLast(this, element))
		return 3;
	return 0;
}

/*
Удаляет элемент из листа по его индексу.
ArrayList this - Лист, из которого нужно исключить элемент.
size_t index - Номер элемента, который необходимо освободить.
Возвращает: код ошибки
						0 - Элемент успешно удалён.
						1 - Указатель на лист равен NULL.
						2 - Указатель на массив в листе равен NULL.
						3 - index указывает на несущетсвующий элемент.
						4 - Ошибка от memmove_s.
*/
int ArrayList_remove(ArrayList this, size_t index)
{
	if (this == NULL)
		return 1;
	if (this->array == NULL)
		return 2;
	if (index >= this->length)
		return 3;
	if (index != this->length - 1)
	{ // Передвигать последние элементы нужно только тогда, когда последние элементы существуют.
#ifdef _MSC_VER
		if (memmove_s(
			(char*)this->array + index * this->sizeType,
			(this->capacity - index) * this->sizeType,
			(char*)this->array + (index + 1)*this->sizeType,
			(this->length - (index + 1)) * this->sizeType))
			return 4; // error.
#else
		memmove(
			(char*)this->array + index * this->sizeType,
			(char*)this->array + (index + 1)*this->sizeType,
			(this->length - (index + 1)) * this->sizeType);
#endif // _MSC_VER
	}
	this->length--;
	return 0;
}
