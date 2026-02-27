////////////////////////
//
//  Created: Thu Jan 08 2026
//  File: ui.h
//
////////////////////////

#pragma once

#include "window/memory.h"

#include <gtk/gtk.h>

AppData* create_app();
void init_ui(AppData *app, GtkWidget *window);
gboolean draw_callback(GtkWidget *widget, cairo_t *cr, gpointer data);
void add_block_to_list(AppData *app, int id, int size);
void malloc_clicked(GtkWidget *widget, gpointer user_data);
void free_clicked(GtkWidget *widget, gpointer user_data);
