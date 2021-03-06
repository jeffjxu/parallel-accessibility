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
xmlNode **STARTING_NODES = NULL;

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
void check_alt_text(xmlNode *node, int* alt) {
    xmlAttr *property = node->properties;
    while (property != NULL) {
        const xmlChar *name = property->name;

        if (strcmp((const char*)name, "alt") == 0) {
            xmlChar *value = xmlGetProp(node, name);
            if (strcmp((const char*)value, "") != 0) {
                *alt = *alt + 1;
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

void traverse_dom_tree(xmlNode *node, int depth, int *image, int *alt) {
    if (node == NULL) {
        return;
    }

    if (depth == 2) {
        if (node->type == XML_ELEMENT_NODE) {
            if (check_if_alt_needed(node)) {
                *image = *image + 1;
                check_alt_text(node, alt);
            }
        }
        depth = depth + 1;
        traverse_dom_tree(node->children, depth++, image, alt);
    } else {
        for (xmlNode *curNode = node; curNode != NULL; curNode = curNode->next) {
            if (curNode->type == XML_ELEMENT_NODE) {
                if (check_if_alt_needed(curNode)) {
                    *image = *image + 1;
                    check_alt_text(curNode, alt);
                }
            }
            depth = depth + 1;
            traverse_dom_tree(curNode->children, depth++, image, alt);
        }
    }
}

// go to depth 2 and accumulate all those nodes in an array
int starting_nodes(xmlNode *node) {
    // printf("Starting element is: %s\n", node->name);
    node = node->children;

    if (node == NULL) {
        return 0;
    }

    int numNodes = 0;
    for (xmlNode *currNode = node; currNode != NULL; currNode = currNode->next) {
        for (xmlNode *currNodeChildren = currNode->children; currNodeChildren != NULL; currNodeChildren = currNodeChildren->next) {
            numNodes++;
        }
    }

    STARTING_NODES = (xmlNode**)calloc(numNodes, sizeof(xmlNode*));

    int counter = 0;
    for (xmlNode *currNode = node; currNode != NULL; currNode = currNode->next) {
        for (xmlNode *currNodeChildren = currNode->children; currNodeChildren != NULL; currNodeChildren = currNodeChildren->next) {
            STARTING_NODES[counter] = currNodeChildren;
            counter++;
        }
    }

    return numNodes;
}

void traverse_dom_tree_wrapper(int procID, int nproc, xmlNode* startingNode) {
    const int root = 0;
    int tag = 0;
    MPI_Status status;
    MPI_Request request;

    int numNodes = starting_nodes(startingNode);

    if (procID == root) {
        printf("%d\n", nproc);
        for (int a = 0; a < numNodes; a++) {
            printf("Element: %s\n", STARTING_NODES[a]->name);
        }
    }

    int *imageCount = (int*)calloc(numNodes, sizeof(int));
    int *altNeeded = (int*)calloc(numNodes, sizeof(int));

    for (int ind = procID; ind < numNodes; ind += nproc) {
        int *image = (int*)calloc(1, sizeof(int));
        int *alt = (int*)calloc(1, sizeof(int));

        traverse_dom_tree(STARTING_NODES[ind], 2, image, alt);
        //printf("proc: %d, image: %d, alt %d\n", procID, *image, *alt);

        imageCount[ind] = *image;
        altNeeded[ind] = *alt;

        if (procID != root) {
            MPI_Isend(&imageCount[ind], 1, MPI_INT, root, tag, MPI_COMM_WORLD, &request);
            MPI_Isend(&altNeeded[ind], 1, MPI_INT, root, tag, MPI_COMM_WORLD, &request);
        }
    }

    // for (int i = 0; i < numNodes; i++) {
    //     printf("%d: img: %d, alt: %d \n", i, imageCount[i], altNeeded[i]);
    // }

    if (procID == root) {
        int proc = 0;

        for (int i = 0; i < numNodes; i++) {
            proc = i % nproc;
            if (proc == 0) {
                continue;
            }

            MPI_Recv(&imageCount[i], 1, MPI_INT, proc, tag, MPI_COMM_WORLD, &status);
            MPI_Recv(&altNeeded[i], 1, MPI_INT, proc, tag, MPI_COMM_WORLD, &status);
        }

        for (int j = 0; j < numNodes; j++) {
            ALT_COUNT += altNeeded[j];
            IMAGE_COUNT += imageCount[j];
        }
    }
}

int main(int argc, char **argv)  {
    htmlDocPtr doc;
    xmlNode *root_element = NULL;

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

    printf("Root element is: %s\n", root_element->name);
    
    int procID;
    int nproc;
    //double startTime;
    //double endTime;

    // took timing code from wsp.c from assignment 3
    struct timespec before, after;
    if (procID == 0) {
        clock_gettime(0, &before); 
    }

    // Initialize MPI
    MPI_Init(&argc, &argv);

    // Get process rank
    MPI_Comm_rank(MPI_COMM_WORLD, &procID);

    // Get total number of processes specificed at start of run
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    // Run computation
    //startTime = MPI_Wtime();
    traverse_dom_tree_wrapper(procID, nproc, root_element);
    // int numNodes = starting_nodes(root_element);
    // int *imageCount = (int*)calloc(numNodes, sizeof(int));
    // int *altNeeded = (int*)calloc(numNodes, sizeof(int));
    // traverse_dom_tree_wrapper(root_element, 0, imageCount, altNeeded);

    //endTime = MPI_Wtime();

    MPI_Finalize();
    //printf("Your accessibility score: %d/%d\n", ALT_COUNT, IMAGE_COUNT);
    //printf("elapsed time for proc %d: %f\n", procID, endTime - startTime);
    if (procID == 0) {
        clock_gettime(0, &after); // same here
        double delta_ms = (double)(after.tv_sec - before.tv_sec) * 1000.0 + (after.tv_nsec - before.tv_nsec) / 1000000.0;
        putchar('\n');
        printf("============ Time ============\n");
        printf("Time: %.3f ms (%.3f s)\n", delta_ms, delta_ms / 1000.0);
        printf("Your accessibility score: %d/%d\n", ALT_COUNT, IMAGE_COUNT);
    }

    xmlFreeDoc(doc);       // free document
    xmlCleanupParser();    // Free globals
    return 0;
}

// run using
// mpirun -np [processor count] ./accessibility [input file]