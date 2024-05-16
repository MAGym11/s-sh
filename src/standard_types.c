#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    char* ptr;
    size_t length;
    size_t capacity;
} String;

void String_increase_capacity(String* string) {
    if (string->capacity == 0) string->capacity++;
    string->capacity *= 2;
    void* tempPtr = malloc(string->capacity);
    memcpy(tempPtr, string->ptr, string->length);
    free(string->ptr);
    string->ptr = tempPtr;
}

void String_push(String* string, char value) {
    if (string->length == string->capacity)
        String_increase_capacity(string);
    string->ptr[string->length] = value;
    string->length++;
}

void to_String(const char* string, String* result) {
    result->length = strlen(string);
    result->capacity = result->length;
    result->ptr = malloc(result->capacity);
    strcpy(result->ptr, string);
}

void drop_String(String string) {
    free(string.ptr);
}

void print_String(String string) {
    for (int i = 0; i < string.length; i++) {
        if (string.ptr[i] == '\n')
            printf("\\n");
        else if (string.ptr[i] == '\0')
            printf("\\0");
        else
            printf("%c", string.ptr[i]);
    }
}

typedef struct {
    void* ptr;
    size_t length;
    size_t capacity;
} Array;
