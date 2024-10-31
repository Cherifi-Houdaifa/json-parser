#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include "json.h"

struct node* lex(char* jsonstr, size_t size) {
    struct node* tokens = NULL;
    bool indigit = false;
    bool instring = false;
    // this probably should be dynamic (i will keep it static for now)
    char* current = malloc(0x100);
    memset(current, 0x0, 0x100);

    for (size_t i = 0; i < size; i++) {
        if (instring) {
            if (jsonstr[i] == '"') {
                instring = false;
                char* newstring = malloc(strlen(current)+1);
                strncpy(newstring, current, strlen(current));
                addendnode(&tokens, maketoken(string, newstring));
                current[0] = 0;
            } else {
                strncat(current, jsonstr + i, 1);
            }
            continue;
        }
        if (indigit) {
            if (!isdigit(jsonstr[i])) {
                addendnode(&tokens, maketoken(number, (void*)atol(current)));
                current[0] = 0;
                indigit = false;
            } else {
                strncat(current, (jsonstr + i), 1);
                continue;
            }
        }
        if (jsonstr[i] == ' ' || jsonstr[i] == '\t' || jsonstr[i] == '\r'){
            continue;
        }
        if (jsonstr[i] == '{'){
            addendnode(&tokens, maketoken(openbrace, NULL));
        } else if (jsonstr[i] == '}') {
            addendnode(&tokens, maketoken(closebrace, NULL));
        } else if (jsonstr[i] == '['){
            addendnode(&tokens, maketoken(opensqbrace, NULL));
        } else if (jsonstr[i] == ']') {
            addendnode(&tokens, maketoken(closesqbrace, NULL));
        } else if (jsonstr[i] == ':') {
            addendnode(&tokens, maketoken(colon, NULL));
        } else if (jsonstr[i] == ',') {
            addendnode(&tokens, maketoken(comma, NULL));
        } else if (isdigit(jsonstr[i]) || jsonstr[i] == '-') {
            strncat(current, (jsonstr + i), 1);
            indigit = true;
        } else if(jsonstr[i] == '"') {
            current[0] = 0;
            instring = true;
        } else {
            strncat(current, jsonstr+i, 1);
            if (strcmp(current, "null") == 0) {
                addendnode(&tokens, maketoken(jsonnull, NULL));
                current[0] = 0;
            } else if (strcmp(current, "true") == 0) {
                addendnode(&tokens, maketoken(jsontrue, NULL));
                current[0] = 0;
            } else if (strcmp(current, "false") == 0) {
                addendnode(&tokens, maketoken(jsonfalse, NULL));
                current[0] = 0;
            }
        }
        
    } 
    free(current);
    return tokens;
}


void* parse(struct node* tokens) {
    struct Token* firsttoken = tokens->ptr;
    if (firsttoken->type == openbrace) {
        return handlejsonobject(tokens, getjsonobjectlen(tokens));
    } else if (firsttoken->type == opensqbrace) {
        return handlejsonarray(tokens, getjsonarraylen(tokens));
    } else {
        jabort("first token can either be { or [");
    }
    
}

size_t getjsonobjectlen(struct node* tokens) {
    if (cast2token(tokens->ptr)->type != openbrace)
        jabort("objects usually start with an open brace");
    size_t bracestack = 0;
    size_t len = 0;
    struct node* temp = tokens;
    while (temp != NULL) {
        struct Token* current = temp->ptr;
        if (current->type == openbrace)
            bracestack++;
        else if (current->type == closebrace) {
            bracestack--;
            if (bracestack == -1)
                jabort("random closed brace???");
            else if (bracestack == 0) {
                // we are in
                return len + 1;
            }
        }
        len++;
        temp = temp->next;
    }
    jabort("error happened when trying to parse the open and closed braces");
}

size_t getjsonarraylen(struct node* tokens) {
    if (cast2token(tokens->ptr)->type != opensqbrace)
        jabort("arrays usually start with an open square brace");
    size_t bracestack = 0;
    size_t len = 0;
    struct node* temp = tokens;
    while (temp != NULL) {
        struct Token* current = temp->ptr;
        if (current->type == opensqbrace)
            bracestack++;
        else if (current->type == closesqbrace) {
            bracestack--;
            if (bracestack == -1)
                jabort("random closed square brace???");
            else if (bracestack == 0) {
                // we are in
                return len + 1;
            }
        }
        len++;
        temp = temp->next;
    }
    jabort("error happened when trying to parse the open and closed square braces");
}

struct objitem* handlejsonobject(struct node* tokens, size_t len) {
    // this probably should not be hardcoded
    struct objitem* object = malloc(sizeof(struct objitem) * 10);
    for (int i = 0; i < 10; i++) {
        object[i].type = -1;
    }
    size_t objectidx = 0;

    // skip the openbrace
    tokens = tokens->next;
    size_t i = 1;
    while (i < len) {
        struct Token* token = tokens->ptr;
        if (token->type == string) {
            if (cast2token(tokens->next->ptr)->type != colon) {
                jabort("colon must be after key");
            }
            struct Token* valuetoken = cast2token(tokens->next->next->ptr);
            if (valuetoken->type == string) {
                object[objectidx].type = jsontype_string;
                object[objectidx].key = token->value;
                object[objectidx].value = valuetoken->value;
                objectidx++;

                tokens = tokens->next->next->next;
                i += 3;
            } else if (valuetoken->type == number) {
                object[objectidx].type = jsontype_number;
                object[objectidx].key = token->value;
                object[objectidx].value = valuetoken->value;
                objectidx++;
                
                tokens = tokens->next->next->next;
                i += 3;
                printf("end token: %d\n",cast2token(tokens->ptr)->type);
                // printf("i: %ld\nlen: %ld\n", i, len);
            } else if (valuetoken->type == jsonnull) {
                object[objectidx].type = jsontype_null;
                object[objectidx].key = token->value;
                object[objectidx].value = NULL;
                objectidx++;
                
                tokens = tokens->next->next->next;
                i += 3;
            } else if (valuetoken->type == jsontrue) {
                object[objectidx].type = jsontype_boolean;
                object[objectidx].key = token->value;
                object[objectidx].value = (void*)1;
                objectidx++;

                tokens = tokens->next->next->next;
                i += 3;
            } else if (valuetoken->type == jsonfalse) {
                object[objectidx].type = jsontype_boolean;
                object[objectidx].key = token->value;
                object[objectidx].value = (void*)0;
                objectidx++;
                
                tokens = tokens->next->next->next;
                i += 3;
            } else if (valuetoken->type == openbrace) {
                object[objectidx].type = jsontype_object;
                object[objectidx].key = token->value;

                size_t objectlen = getjsonobjectlen(tokens->next->next);
                object[objectidx].value = handlejsonobject(tokens->next->next, objectlen);
                objectidx++;
                i += 2;
                size_t j = i;
                tokens = tokens->next->next;
                while (i < j + objectlen) {
                    tokens = tokens->next;
                    i++;
                }
                // printf("aa: %d\n%p\n", cast2token(tokens->ptr)->type, tokens->next);
            } else if (valuetoken->type == opensqbrace) {
                object[objectidx].type = jsontype_array;
                object[objectidx].key = token->value;
                size_t arraylen = getjsonarraylen(tokens->next->next);
                object[objectidx].value = handlejsonarray(tokens->next->next, arraylen);
                objectidx++;

                i+=2;
                tokens = tokens->next->next;
                size_t j = i;
                while (i < j + arraylen) {
                    tokens = tokens->next;
                    i++;
                }
            }
        } else if (token->type == comma) {
            // nice
            tokens = tokens->next;
            i++;
        } else if (token->type == closebrace) {
            // puts("Hiii");
            return object;
        } else {
            jabort("an object is consisted of <key> : <value> pairs, key is a string");
        }        
    }
    jabort("should not reach end of function");
}

struct arrayitem* handlejsonarray(struct node* tokens, size_t len) {
    struct arrayitem* array = malloc(10 * sizeof(struct arrayitem));
    for (int i = 0; i < 10; i++) {
        array[i].type = -1;
    }
    size_t arrayidx = 0;
    tokens = tokens->next;
    size_t i = 1;
    while (i < len) {
        struct Token* token = tokens->ptr;
        if (token->type == string) {
            array[arrayidx].type = jsontype_string;
            array[arrayidx].value = token->value;
            arrayidx++;
            
            tokens = tokens->next;
            i+=1;
        } else if (token->type == number) {
            array[arrayidx].type = jsontype_number;
            array[arrayidx].value = token->value;
            arrayidx++;
            
            tokens = tokens->next;
            i+=1;
        } else if (token->type == jsonnull) {
            array[arrayidx].type = jsontype_null;
            array[arrayidx].value = NULL;
            arrayidx++;
            
            tokens = tokens->next;
            i+=1;
        } else if (token->type == jsontrue) {
            array[arrayidx].type = jsontype_boolean;
            array[arrayidx].value = (void*)1;
            arrayidx++;
            
            tokens = tokens->next;
            i+=1;
        } else if (token->type == jsonfalse) {
            array[arrayidx].type = jsontype_boolean;
            array[arrayidx].value = (void*)0;
            arrayidx++;
            
            tokens = tokens->next;
            i+=1;
        } else if (token->type == openbrace) {
            array[arrayidx].type = jsontype_object;
            size_t objectlen = getjsonobjectlen(tokens);
            array[arrayidx].value = handlejsonobject(tokens, objectlen);
            arrayidx++;

            size_t j = i;
            while (i < j + objectlen) {
                tokens = tokens->next;
                i++;
            }
        } else if (token->type == opensqbrace) {
            array[arrayidx].type = jsontype_array;
            size_t arraylen = getjsonarraylen(tokens);
            array[arrayidx].value = handlejsonarray(tokens, arraylen);
            arrayidx++;

            size_t j = i;
            while (i < j + arraylen) {
                tokens = tokens->next;
                i++;
            }
        } else if (token->type == comma) {
            tokens = tokens->next;
            i++;
        } else if (token->type == closesqbrace) {
            return array;
        }
        
    }
}

void printobject(struct objitem* object) {
    printf("{");
    for (int i = 0; object[i].type != -1; i++) {
        switch (object[i].type)
        {
        case jsontype_string:
            printf("\"%s\": \"%s\"\n", object[i].key, (char*)object[i].value);
            break;
        case jsontype_object:
            printf("\"%s\": ", object[i].key);
            printobject(object[i].value);
            break;
        case jsontype_array:
            printf("\"%s\": ", object[i].key);
            printarray(object[i].value);
            break;
        case jsontype_number:
            printf("\"%s\": %ld\n", object[i].key, (long)object[i].value);
            break;
        case jsontype_boolean:
            printf("\"%s\": %s\n", object[i].key, object[i].value != 0 ? "true" : "false");
            break;
        case jsontype_null:
            printf("\"%s\": %s\n", object[i].key, "null");
            break;
        default:
            break;
        }
        if (object[i+1].type != -1) {
            putchar(',');
        }
    }
    printf("}");
}

void printarray(struct arrayitem* array) {
    printf("[");
    for (int i = 0; array[i].type != -1; i++) {
        switch (array[i].type)
        {
        case jsontype_string:
            printf("\"%s\"\n", (char*)array[i].value);
            break;
        case jsontype_object:
            printobject(array[i].value);
            break;
        case jsontype_array:
            printarray(array[i].value);
            break;
        case jsontype_number:
            printf("%ld\n", (long)array[i].value);
            break;
        case jsontype_boolean:
            printf("%s\n", array[i].value != 0 ? "true" : "false");
            break;
        case jsontype_null:
            printf("%s\n", "null");
            break;
        default:
            break;
        }
        if (array[i+1].type != -1) {
            putchar(',');
        }
    }
    printf("]");
}

void jabort(char* message) {
    fputs(message, stderr);
    exit(1);
}

void addendnode(struct node** head, void* value) {
    if (*head == NULL) {
        *head = malloc(sizeof(struct node));
        (*head)->ptr = value;
        (*head)->next = NULL;
    } else {
        struct node* temp = *head;
        while (temp->next != NULL)
            temp = temp->next;
        temp->next = malloc(sizeof(struct node));
        temp->next->ptr = value;
        temp->next->next = NULL;
    }
}


struct Token* maketoken(enum tokentype type, void* value) {
    struct Token* rslt = malloc(sizeof(struct Token));
    rslt->type = type;
    rslt->value = value;
    return rslt;
}