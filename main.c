#include <stdio.h>
#include <stdlib.h>
#include "json.h"
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void readfile(const char* path, char* buf, size_t size) {
    int fd = open(path, 0x0);
    read(fd, buf, size);
    close(fd);
}


int main() {
    char* jsondata = malloc(0x1000);
    readfile("./example.json", jsondata, 0x1000);
    puts(jsondata);
    puts("==========================");
    struct node* tokens = lex(jsondata, strlen(jsondata));
    struct arrayitem* object = parse(tokens);
    printf("addr: %p\n", object);
    puts("==========================");
    printarray(object);
    
    exit(1);
    
    
    struct node* temp = tokens;
    
    
    puts("==========================");
    size_t len = getjsonobjectlen(temp);
    printf("len: %ld\n", len);
    size_t i = 0;
    while (i < len) {
        struct Token* token = tokens->ptr;
        switch (token->type)
        {
            case openbrace:
                puts("openbrace");
                break;
            case closebrace:
                puts("closebrace");
                break;
            case opensqbrace:
                puts("opensqbrace");
                break;
            case closesqbrace:
                puts("closesqbrace");
                break;
            case string:
                printf("string: %s\n", (char*)(token->value));
                break;
            case number:
                printf("number: %ld\n", (long)(token->value));
                break;
            case jsonnull:
                puts("jsonnull");
                break;
            case jsontrue:
                puts("jsontrue");
                break;
            case jsonfalse:
                puts("jsonfalse");
                break;
            case colon:
                puts("colon");
                break;
            case comma:
                puts("comma");
                break;
        default:
            printf("Token does not exist\n");
            exit(1);
            break;
        }
        i++;
        tokens = tokens->next;
    }
    
}


