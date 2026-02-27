////////////////////////
//
//  Created: Thu Jan 08 2026
//  File: memory.h
//
////////////////////////

#pragma once

#include <gtk/gtk.h>

#define PAGE_SIZE 4096

typedef struct {
    int id;
} Byte;

typedef struct {
    Byte bytes[PAGE_SIZE];
} Page;

typedef struct {
    Page* pages;
    int num_pages;
    int capacity;
} Memory;

typedef struct {
    int page_id;
    int id;
    int size;
    void *malloced_ptr;
} MallocBlock;

typedef struct {
    Memory memory;
    MallocBlock blocks[100];
    uint64_t heap_start;
    uint64_t heap_end;
    int num_blocks;
    int next_id;
    int total_size;
    int heap_size;

    GtkWidget *list;
    GtkWidget *drawing_area;
    GtkWidget *size_entry;
} AppData;

void init_memory(Memory *mem);
void add_page(Memory *mem);
int allocate(AppData *app, int size, int *out_page, int *out_offset);
void deallocate(AppData *app, int id);
void id_to_color(int id, double *r, double *g, double *b);
