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

int IMAGE_COUNT = 0;
int ALT_COUNT = 0;

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
// :input node (xmlNode*) - a parsed node of the DOM tree
// :output none - increments ALT_COUNT if tag has alt text
void check_alt_text(xmlNode *node) {
    xmlAttr *property = node->properties;
    while (property != NULL) {
        const xmlChar *name = property->name;
        if (strcmp((const char*)name, "alt") == 0) {
            xmlChar *value = xmlGetProp(node, name);
            if (strcmp((const char*)value, "") != 0) {
                ALT_COUNT++;
            }
        }
        property = property->next;
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
    xmlNode *cur_node = NULL;

    if(NULL == node)
    {
        return;
    }

    for (cur_node = node; cur_node; cur_node = cur_node->next) 
    {
        if (cur_node->type == XML_ELEMENT_NODE) 
        {
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

int main(int argc, char **argv)  {
    htmlDocPtr doc;
    xmlNode *root_element = NULL;

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

    root_element = xmlDocGetRootElement(doc);

    if (root_element == NULL) 
    {
        fprintf(stderr, "empty document\n");
        xmlFreeDoc(doc);
        return 0;
    }

    printf("Root Node is %s\n", root_element->name);
    traverse_dom_tree(root_element, 0);
    printf("Your accessibility score: %d/%d\n", ALT_COUNT, IMAGE_COUNT);

    xmlFreeDoc(doc);       // free document
    xmlCleanupParser();    // Free globals
    return 0;
}
