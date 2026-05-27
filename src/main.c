#include <gtk/gtk.h>
#include "presentation/window.h"

static void on_activate(GtkApplication *app, gpointer user_data) {
    show_main_window(app);
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("org.vaxp.vconvert", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    
    return status;
}
