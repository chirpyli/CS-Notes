/*
 * C pointer 
 */

#include<stdio.h>
#include<stdlib.h>
#include<assert.h>

typedef enum NodeType {
    INT,
    CHAR
} NodeType;

typedef struct Node {
    NodeType type;
} Node;

typedef struct IntNode {
    NodeType type;
    int value;
} IntNode;

typedef struct CharNode {
    NodeType type;
    char value;
} CharNode;

char* printNodeType(NodeType type) {

    switch (type)
    {
    case INT:
        return "INT";    
    case CHAR:
        return "CHAR";
    default:
        return "UNKNOW";
    }

    return NULL;
}

void printNode(Node* n) {
    assert(n != NULL);

    switch (n->type)
    {
    case INT:
        {
            IntNode* in = (IntNode*)n;
            printf("type:%s value:%d \n", printNodeType(in->type), in->value);
        }
        break;
    case CHAR:
        {
            CharNode* cn = (CharNode*)n;
            printf("type:%s value:%d \n", printNodeType(cn->type), cn->value);
        }
        break;
    default:
        printf("unknow node \n");
        break;
    }
    
}

int main() {
    IntNode *a = (IntNode*)malloc(sizeof(IntNode));
    a->type = INT;
    a->value = 1024;
    printNode(a);

    CharNode *b = (CharNode*)malloc(sizeof(CharNode));
    b->type = CHAR;
    b->value = 'a';
    printNode(b);

    return 0;
}
