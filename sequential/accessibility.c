#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <error.h>
#include <limits.h>
#include "libxml/parser.h"
#include "libxml/tree.h"
#include "libxml/HTMLparser.h"

void check_properties(xmlNode *node) {
    xmlAttr *property = node->properties;
    while (property != NULL) {
        const xmlChar *name = property->name;
        xmlChar *value = xmlGetProp(node, name);
        printf("PropName: %s %s\n", name, value); 
        property = property->next;
    }
}

void traverse_dom_trees(xmlNode * a_node) {
    xmlNode *cur_node = NULL;

    if(NULL == a_node)
    {
        //printf("Invalid argument a_node %p\n", a_node);
        return;
    }

    for (cur_node = a_node; cur_node; cur_node = cur_node->next) 
    {
        if (cur_node->type == XML_ELEMENT_NODE) 
        {
            /* Check for if current node should be exclude or not */
            printf("Node type: Text, name: %s\n", cur_node->name);
            check_properties(cur_node);
        }
        else if(cur_node->type == XML_TEXT_NODE)
        {
            /* Process here text node, It is available in cpStr :TODO: */
            printf("node type: Text, node content: %s,  content length %d\n", (char *)cur_node->content, (int)strlen((char *)cur_node->content));
        }
        traverse_dom_trees(cur_node->children);
    }
}

int main(int argc, char **argv)  {
    htmlDocPtr doc;
    xmlNode *roo_element = NULL;

    if (argc != 2)  
    {
        printf("\nInvalid argument\n");
        return(1);
    }

    /* Macro to check API for match with the DLL we are using */
    LIBXML_TEST_VERSION    

    doc = htmlReadFile(argv[1], NULL, HTML_PARSE_NOBLANKS | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING | HTML_PARSE_NONET);
    if (doc == NULL) 
    {
        fprintf(stderr, "Document not parsed successfully.\n");
        return 0;
    }

    roo_element = xmlDocGetRootElement(doc);

    if (roo_element == NULL) 
    {
        fprintf(stderr, "empty document\n");
        xmlFreeDoc(doc);
        return 0;
    }

    printf("Root Node is %s\n", roo_element->name);
    traverse_dom_trees(roo_element);

    xmlFreeDoc(doc);       // free document
    xmlCleanupParser();    // Free globals
    return 0;
}


// // step 1: figure out how to integrate Libxml2
// /**
//  * print_element_names:
//  * @a_node: the initial xml node to consider.
//  *
//  * Prints the names of the all the xml elements
//  * that are siblings or children of a given xml node.
//  */
// static void
// print_element_names(xmlNode * a_node)
// {
//     xmlNode *cur_node = NULL;

//     for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
//         if (cur_node->type == XML_ELEMENT_NODE) {
//             printf("node type: Element, name: %s\n", cur_node->name);
//         }

//         print_element_names(cur_node->children);
//     }
// }

// /**
//  * Simple example to parse a file called "file.xml", 
//  * walk down the DOM, and print the name of the 
//  * xml elements nodes.
//  */
// int
// main(int argc, char **argv)
// {
//     printf("start\n");
//     xmlDoc *doc = NULL;
//     xmlNode *root_element = NULL;

//     if (argc != 2)
//         return(1);

//     /*
//      * this initialize the library and check potential ABI mismatches
//      * between the version it was compiled for and the actual shared
//      * library used.
//      */
//     LIBXML_TEST_VERSION

//     /*parse the file and get the DOM */
//     doc = xmlReadFile(argv[1], NULL, 0);

//     if (doc == NULL) {
//         printf("error: could not parse file %s\n", argv[1]);
//     }

//     /*Get the root element node */
//     root_element = xmlDocGetRootElement(doc);

//     print_element_names(root_element);

//     /*free the document */
//     xmlFreeDoc(doc);

//     /*
//      *Free the global variables that may
//      *have been allocated by the parser.
//      */
//     xmlCleanupParser();
//     fprintf(stderr, "here\n");
//     putchar('\n');

//     return 0;
// }
