////////////////////////
//
//  Created: Thu Jan 08 2026
//  File: main.c
//
////////////////////////

#include <gtk/gtk.h>

#include "window/ui.h"

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window),900,600);
    g_signal_connect(window,"destroy",G_CALLBACK(gtk_main_quit),NULL);

    AppData *app = create_app();
    init_ui(app,window);

    gtk_main();
    free(app->memory.pages);
    free(app);
    return 0;
}
