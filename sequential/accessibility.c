#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdint.h>
#include <error.h>
#include <limits.h>
#include <time.h>
#include <pthread.h>
#include "libxml/parser.h"
#include "libxml/tree.h"
#include "libxml/HTMLparser.h"
#include "queue.c"
#include <omp.h>

int IMAGE_COUNT = 0;
int ALT_COUNT = 0;
int NCORES = -1;
q_t *Q = createQueue(1000000);


// print the DOM tree
void print_properties(xmlNode *node) {
    xmlAttr *property = node->properties;
    while (property != NULL) {
        const xmlChar *name = property->name;
        xmlChar *value = xmlGetProp(node, name);
        printf("PropName: %s %s\n", name, value); 
        property = property->next;
    }
}

// check if a tag has alt text
// Parallelized this while loop like this after viewing http://web.engr.oregonstate.edu/~mjb/cs575/Handouts/tasks.1pp.pdf slide 9 but this causes slowdown
// :input node (xmlNode*) - a parsed node of the DOM tree
// :output none - increments ALT_COUNT if tag has alt text
void check_alt_text(xmlNode *node) {
    // #pragma omp parallel
    // {
    //     #pragma omp single 
    //     {
            xmlAttr *property = node->properties;
            while (property != NULL) {
                const xmlChar *name = property->name;
                // #pragma omp task
                if (strcmp((const char*)name, "alt") == 0) {
                    xmlChar *value = xmlGetProp(node, name);
                    if (strcmp((const char*)value, "") != 0) {
                        ALT_COUNT++;
                    }
                }
                property = property->next;
        //     }
        // }
        //#pragma omp taskwait // may not be needed for this funciton
    }
}

// check if alt text is needed for a tag
// :input node (xmlNode*) - a parsed node of the DOM tree
// :output bool - true if alt text is needed, false otherwise
bool check_if_alt_needed(xmlNode *node) {
    const char *name = (const char*)node->name;
    return strcmp(name, "img") == 0 ||
           strcmp(name, "area") == 0 ||
           strcmp(name, "input") == 0;
}

// traverse the DOM tree
// :input node (xmlNode*) - a parsed node of the DOM tree
// :input depth (int) - recursion depth
// :output none - calculate accessibility score
void traverse_dom_tree(xmlNode *node, int depth) {
    printf("depth: %d\n", depth);
    xmlNode *cur_node = NULL;

    if(NULL == node)
    {
        return;
    }
    #pragma omp parallel
    for (cur_node = node; cur_node; cur_node = cur_node->next) {
        #pragma omp task 
        {
            if (cur_node->type == XML_ELEMENT_NODE) {
                // printf("Node type: Text, name: %s\n", (const char*)cur_node->name);
                // print_properties(cur_node);
                if (check_if_alt_needed(cur_node)) {
                    IMAGE_COUNT++;
                    check_alt_text(cur_node);
                }
            }
            traverse_dom_tree(cur_node->children, depth++);
        }
    }
}

// trying to make this function parallizeable by using BFS
// void traverse_dom_tree(xmlNode *node, int depth) {
//     printf("depth: %d\n", depth);
//     // #pragma omp parallel
//     // {
//     //     #pragma omp single
//     //     {
//             enqueue(Q, node);

//             xmlNode *cur_node;
//             #pragma omp parallel
//             while (!isEmpty(Q)) 
//             {
//                 // #pragma omp task
//                 // {
//                     #pragma omp critical
//                     cur_node = dequeue(Q);
//                     while (cur_node != NULL)
//                     { 
//                         if (cur_node->type == XML_ELEMENT_NODE) 
//                         {
//                             // printf("Node type: Text, name: %s\n", (const char*)cur_node->name);
//                             // print_properties(cur_node);
//                             if (check_if_alt_needed(cur_node)) {
//                                 IMAGE_COUNT++;
//                                 check_alt_text(cur_node);
//                             }
//                         }
//                         if (cur_node->children != NULL){
//                             #pragma omp critical
//                             enqueue(Q, cur_node->children);
//                         }
//                         cur_node = cur_node->next;
//                     }
//                 // }
//             }
//     //     }
//     // }
// }

int main(int argc, char **argv)  {
    htmlDocPtr doc;
    xmlNode *root_element = NULL;

    if (argc != 3)  
    {
        fprintf(stderr,"Expecting two arguments: [file name] [processor count]\n");
        return 0;
    }

    NCORES = atoi(argv[2]);
    if(NCORES < 1) {
        fprintf(stderr, "Illegal core count: %d\n", NCORES);
        return 0;
    }

    /* Macro to check API for match with the DLL we are using */
    LIBXML_TEST_VERSION

    doc = htmlReadFile(argv[1], NULL, HTML_PARSE_NOBLANKS | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING | HTML_PARSE_NONET);
    if (doc == NULL) 
    {
        fprintf(stderr, "Document not parsed successfully.\n");
        return 0;
    }

    root_element = xmlDocGetRootElement(doc);

    if (root_element == NULL) 
    {
        fprintf(stderr, "empty document\n");
        xmlFreeDoc(doc);
        return 0;
    }
    
    // took timing code from wsp.c from assignment 3
    struct timespec before, after;
    printf("Root Node is %s\n", root_element->name);
    clock_gettime(0, &before); // "0" should be CLOCK_REALTIME but vscode thinks it undefined for some reason
    traverse_dom_tree(root_element, 0);
    clock_gettime(0, &after); // same here
    double delta_ms = (double)(after.tv_sec - before.tv_sec) * 1000.0 + (after.tv_nsec - before.tv_nsec) / 1000000.0;
    putchar('\n');
    printf("============ Time ============\n");
    printf("Time: %.3f ms (%.3f s)\n", delta_ms, delta_ms / 1000.0);
    printf("Your accessibility score: %d/%d\n", ALT_COUNT, IMAGE_COUNT);

    xmlFreeDoc(doc);       // free document
    xmlCleanupParser();    // Free globals
    return 0;
}

