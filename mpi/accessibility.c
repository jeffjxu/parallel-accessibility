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
#include "mpi.h"

int IMAGE_COUNT = 0;
int ALT_COUNT = 0;
int NCORES = -1;
xmlNode *STARTING_NODES = NULL;

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

void traverse_dom_tree(xmlNode *node, int depth) {
    if (node == NULL) {
        return;
    }

    for (xmlNode *cur_node = node; cur_node != NULL; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE) {
            if (check_if_alt_needed(cur_node)) {
                IMAGE_COUNT++;
                check_alt_text(cur_node);
            }
        }
        traverse_dom_tree(cur_node->children, depth++);
    }
}

// go to depth 1 and accumulate all those nodes in an array
void starting_nodes(xmlNode *node) {

}

void traverse_dom_tree_wrapper(int procID, int nproc) {
    const int root = 0;
    int tag = 0;
    MPI_Status status;
    MPI_Request request;

    int *imageCount = (int*)calloc(nproc, sizeof(int));
    int *altNeeded = (int*)calloc(nproc, sizeof(int));

    // TODO:
    // Generate a list of starting nodes and broadcast it
    // each process takes starting nodes using interleaved assignment
    // call traverse_dom_tree on the starting node
    // accumulate imageCount and altNeeded and send that back to root
    // print out result
    
}

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
    
    int procID;
    int nproc;
    double startTime;
    double endTime;

    // Initialize MPI
    MPI_Init(&argc, &argv);

    // Get process rank
    MPI_Comm_rank(MPI_COMM_WORLD, &procID);

    // Get total number of processes specificed at start of run
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    // Run computation
    startTime = MPI_Wtime();
    traverse_dom_tree(root_element, 0);
    endTime = MPI_Wtime();

    MPI_Finalize();
    printf("Your accessibility score: %d/%d\n", ALT_COUNT, IMAGE_COUNT);
    printf("elapsed time for proc %d: %f\n", procID, endTime - startTime);

    xmlFreeDoc(doc);       // free document
    xmlCleanupParser();    // Free globals
    return 0;
}
