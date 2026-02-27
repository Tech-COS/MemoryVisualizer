/////////////////////// /
//
//  Created: Thu Jan 08 2026
//  File: ui.c
//
////////////////////////

#include <stdint.h>
#include <sys/mman.h>
#include "window/ui.h"
#include "../MemoryManagement/include/cos_memory_api.h"
#include "../MemoryManagement/include/cos_memory_management.h"

#define COS_MEMSIZE_PAGE 4096
static AppData *process = NULL;

uintptr_t find_largest_free_area(uint64_t *largest_free_size);

AppData *create_app() {
    process  = malloc(sizeof(AppData));
    process->next_id = 1;
    process->num_blocks  = 0;
    process->total_size = 0;


    uint64_t largest_free_size = 0;
    uint64_t largest_free_area = find_largest_free_area(&largest_free_size);
    process->heap_start = (uint64_t)mmap((void *)largest_free_area, COS_MEMSIZE_PAGE, PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    memset((uint8_t *)process->heap_start, 0, COS_MEMSIZE_PAGE);
    init_cos_malloc(process->heap_start);
    process->heap_end = process->heap_start + COS_MEMSIZE_PAGE;
    process->heap_size = COS_MEMSIZE_PAGE;
    init_memory(&process->memory);
    return process;
}

void add_block_to_list(AppData *app, int id, int size) {
    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(app->list)));
    GtkTreeIter iter;
    gtk_list_store_append(store, &iter);
    char buf[64];
    sprintf(buf, "Block  %d :  %d bytes", id, size);
    gtk_list_store_set(store, &iter, 0, buf,  - 1);
}

gboolean draw_callback(__attribute__((unused)) GtkWidget *widget, cairo_t *cr, gpointer data) {
    AppData *app = (AppData *)data;
    int cell_size = 5;
    int cols = 64;
    int rows = COS_MEMSIZE_PAGE / cols;
    int block = 0;
    int j = 0;

    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    for (int p = 0; p < app->heap_size / COS_MEMSIZE_PAGE; p++) {
        int px_index = p % 2, py_index = p / 2;
        int page_x = 10  +  px_index * (cols * cell_size + 20);
        int page_y = 10  +  py_index * (rows * cell_size + 40);
        int i = 0;

        if (app->num_blocks)
        {
            for (; block < app->num_blocks && i < COS_MEMSIZE_PAGE;) {
                for (; j < app->blocks[block].size && i < COS_MEMSIZE_PAGE; ++j, ++i)
                {
                    int x = page_x  +  (i % cols) * cell_size;
                    int y = page_y  +  (i / cols) * cell_size;

                    if (!i)
                        cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
                    else
                    {
                        double r, g, b;
                        id_to_color(app->blocks[block].id, &r, &g, &b);
                        cairo_set_source_rgb(cr, r, g, b);
                    }
                    cairo_rectangle(cr, x, y, cell_size - 1, cell_size - 1);
                    cairo_fill(cr);
                }
                if (j == app->blocks[block].size)
                {
                    j = 0;
                    ++block;
                }
            }
            for (; i < COS_MEMSIZE_PAGE; ++i)
            {
                int x = page_x  +  (i % cols) * cell_size;
                int y = page_y  +  (i / cols) * cell_size;

                cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
                cairo_rectangle(cr, x, y, cell_size - 1, cell_size - 1);
                cairo_fill(cr);
            }
        }
        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_move_to(cr, page_x, page_y - 2);

        char buf[16] = {};
        sprintf(buf, "Page %d", p);
        cairo_show_text(cr, buf);
    }
    return FALSE;
}

void init_heap_object(malloc_heap_object_t *heap_object)
{
    heap_object->heap_start = process->heap_start;
    heap_object->heap_end = process->heap_end;
}

uint64_t expand_section(uint64_t next_heap_addr,  uint64_t size)
{
    uint64_t nb_of_block_allocated = size >> 12;

    UNUSED(next_heap_addr);
    if (size & 0xFFF)
        ++nb_of_block_allocated;
    process->heap_end = (uint64_t)mmap((void *)process->heap_end, nb_of_block_allocated * COS_MEMSIZE_PAGE, 
                                                PROT_WRITE | PROT_READ, 
                                                MAP_PRIVATE | MAP_ANONYMOUS, 
                                                -1, 0);

    process->heap_size += COS_MEMSIZE_PAGE * nb_of_block_allocated;
    process->heap_end += COS_MEMSIZE_PAGE * nb_of_block_allocated;
    return nb_of_block_allocated;
}

void malloc_clicked(__attribute__((unused)) GtkWidget *widget, gpointer user_data) {
    AppData *app = (AppData * )user_data;
    const char *text = gtk_entry_get_text(GTK_ENTRY(app->size_entry));
    int size = atoi(text);  if (size <= 0) return;

    app->blocks[app->num_blocks].malloced_ptr = (uint8_t *)cos_malloc(size);
    app->blocks[app->num_blocks].size = size;
    app->blocks[app->num_blocks].page_id = app->total_size / COS_MEMSIZE_PAGE;
    app->blocks[app->num_blocks].id = app->num_blocks;
    app->total_size += size;
    add_block_to_list(app, app->blocks[app->num_blocks].page_id, size);
    app->num_blocks++;

    int cols = 64;
    int rows = COS_MEMSIZE_PAGE / cols;
    int num_lines = (app->heap_size / COS_MEMSIZE_PAGE + 1) / 2;
    gtk_widget_set_size_request(app->drawing_area, 600, num_lines * (rows * 5 + 40));
    gtk_widget_queue_draw(app->drawing_area);
}
void free_clicked(__attribute__((unused)) GtkWidget *widget, gpointer user_data) {
    AppData *app = (AppData *)user_data;
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(app->list));
    GtkTreeModel *model;
    GtkTreeIter iter;

    if (!gtk_tree_selection_get_selected(selection, &model, &iter))
        return;

    char *text;
    gtk_tree_model_get(model, &iter, 0, &text,  - 1);
    int id;
    int size;
    sscanf(text, "Block  %d :  %d bytes", &id, &size);
    //deallocate(app, id);

    for (int i = 0; i < app->num_blocks; i++) {
        if (app->blocks[i].id == id && app->blocks[i].size == size) {
            cos_free(app->blocks[i].malloced_ptr);
            for (int j = i; j < app->num_blocks - 1; j++)  {
                app->blocks[j] = app->blocks[j + 1];
                app->num_blocks--;
                break;
            }
        }
    }

    gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
    gtk_widget_queue_draw(app->drawing_area);
}

void init_ui(AppData *app, GtkWidget *window) {
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_container_add(GTK_CONTAINER(window), hbox);

    app->list = gtk_tree_view_new();
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("Malloc Blocks", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(app->list), column);
    GtkListStore *store = gtk_list_store_new(1, G_TYPE_STRING);
    gtk_tree_view_set_model(GTK_TREE_VIEW(app->list), GTK_TREE_MODEL(store));

    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(scroll, 200,  - 1);
    gtk_container_add(GTK_CONTAINER(scroll), app->list);
    gtk_box_pack_start(GTK_BOX(hbox), scroll, FALSE, FALSE, 5);

    app->drawing_area = gtk_drawing_area_new();
    g_signal_connect(G_OBJECT(app->drawing_area), "draw", G_CALLBACK(draw_callback), app);
    int rows = PAGE_SIZE / 32;
    int num_lines = (app->memory.num_pages + 1) / 2;
    gtk_widget_set_size_request(app->drawing_area, 600, num_lines * (rows * 5 + 40));

    GtkWidget *scroll_visu = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_visu), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scroll_visu), app->drawing_area);
    gtk_box_pack_start(GTK_BOX(hbox), scroll_visu, TRUE, TRUE, 5);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 5);

    app->size_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app->size_entry), "Size to malloc");
    gtk_box_pack_start(GTK_BOX(vbox), app->size_entry, FALSE, FALSE, 5);

    GtkWidget *malloc_btn = gtk_button_new_with_label("Malloc");
    g_signal_connect(malloc_btn, "clicked", G_CALLBACK(malloc_clicked), app);
    gtk_box_pack_start(GTK_BOX(vbox), malloc_btn, FALSE, FALSE, 5);

    GtkWidget *free_btn = gtk_button_new_with_label("Free");
    g_signal_connect(free_btn, "clicked", G_CALLBACK(free_clicked), app);
    gtk_box_pack_start(GTK_BOX(vbox), free_btn, FALSE, FALSE, 5);

    gtk_widget_show_all(window);
}
