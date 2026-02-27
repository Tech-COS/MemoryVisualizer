////////////////////////
//
//  Created: Thu Jan 08 2026
//  File: memory.c
//
////////////////////////

#include "window/memory.h"
#include <stdlib.h>
#include <string.h>

void init_memory(Memory *mem) {
    mem->num_pages = 1;
    mem->capacity = 1;
    mem->pages = (Page*)malloc(sizeof(Page) * mem->capacity);
    memset(mem->pages[0].bytes, 0, PAGE_SIZE * sizeof(Byte));
}

void add_page(Memory *mem) {
    if(mem->num_pages >= mem->capacity) {
        mem->capacity *= 2;
        mem->pages = realloc(mem->pages, mem->capacity * sizeof(Page));
    }
    memset(mem->pages[mem->num_pages].bytes, 0, PAGE_SIZE * sizeof(Byte));
    mem->num_pages++;
}

int allocate(AppData *app, int size, int *out_page, int *out_offset)
{
    int remaining = size;
    int start_page = -1, start_offset = -1;

    for(int p = 0; ; p++) {
        if (p >= app->memory.num_pages)
            add_page(&app->memory);

        for (int o = 0; o < PAGE_SIZE; o++) {
            if (remaining == size && app->memory.pages[p].bytes[o].id == 0) {
                start_page = p;
                start_offset = o;
            }
            if (app->memory.pages[p].bytes[o].id == 0) {
                remaining--;

                if (remaining == 0) {
                    int id = app->next_id++;
                    int to_allocate = size;
                    int page = start_page;
                    int offset = start_offset;

                    while (to_allocate > 0) {
                        int alloc_in_page = PAGE_SIZE - offset;

                        if (alloc_in_page > to_allocate)
                            alloc_in_page = to_allocate;

                        for (int i = 0; i < alloc_in_page; i++)
                            app->memory.pages[page].bytes[offset+i].id = id;

                        to_allocate -= alloc_in_page;
                        page++;
                        offset=0;
                    }
                    *out_page = start_page;
                    *out_offset = start_offset;
                    return id;
                }
            } else
                remaining = size;
        }
    }
}

void deallocate(AppData *app, int id) {
    for(int p = 0; p<app->memory.num_pages; p++)
        for(int o = 0; o<PAGE_SIZE; o++)
            if(app->memory.pages[p].bytes[o].id == id)
                app->memory.pages[p].bytes[o].id = 0;
}

void id_to_color(int id, double *r, double *g, double *b) {
    *r = ((id*53) % 256) / 255.0;
    *g = ((id*97) % 256) / 255.0;
    *b = ((id*193) % 256) / 255.0;
}
