#ifndef JSON_H
#define JSON_H 1

enum tokentype {
    openbrace,
    closebrace,
    opensqbrace,
    closesqbrace,
    string,
    number,
    jsonnull,
    jsontrue,
    jsonfalse,
    colon,
    comma
};

enum jsontype {
    jsontype_string,
    jsontype_array,
    jsontype_object,
    jsontype_number,
    jsontype_boolean,
    jsontype_null
};

struct objitem {
    char type; // can be object, string, array ,number, boolean  
    char* key;
    void* value;
};

struct arrayitem {
    char type; // can be object, string, array , number, boolean
    void* value;
};

struct Token {
    enum tokentype type;
    void* value; 
};


struct node {
    void* ptr;
    struct node* next;
};

struct object {

};

struct node* lex(char* jsonstr, size_t siz4e);
void addendnode(struct node** head, void* value);
struct Token* maketoken(enum tokentype type, void* value); 
void* parse(struct node* tokens);
struct objitem* handlejsonobject(struct node* tokens, size_t len); 
struct arrayitem* handlejsonarray(struct node* tokens, size_t len);
void jabort(char* message);
size_t getjsonobjectlen(struct node* tokens); 
size_t getjsonarraylen(struct node* tokens);
void printarray(struct arrayitem* array);
void printobject(struct objitem* object); 


#define cast2token(x) ((struct Token*)(x))

#endif