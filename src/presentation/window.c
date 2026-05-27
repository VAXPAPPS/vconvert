#include "window.h"
#include "../usecases/converter.h"
#include "../domain/media_types.h"

static GtkWidget *list_box;


static void load_css() {
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_resource(provider, "/org/vaxp/vconvert/presentation/style.css");
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
    g_object_unref(provider);
}

static void on_drag_data_received(GtkWidget *widget, GdkDragContext *context,
                                  gint x, gint y, GtkSelectionData *data,
                                  guint info, guint time, gpointer user_data) {
    gchar **uris = gtk_selection_data_get_uris(data);
    if (uris) {
        for (int i = 0; uris[i] != NULL; i++) {
            gchar *filename = g_filename_from_uri(uris[i], NULL, NULL);
            if (filename) {
                GtkWidget *row = gtk_list_box_row_new();
                gtk_list_box_row_set_activatable(GTK_LIST_BOX_ROW(row), FALSE);
                gtk_list_box_row_set_selectable(GTK_LIST_BOX_ROW(row), FALSE);

                GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
                GtkStyleContext *ctx = gtk_widget_get_style_context(box);
                gtk_style_context_add_class(ctx, "item-row");

                GtkWidget *icon = gtk_image_new_from_icon_name("image-x-generic", GTK_ICON_SIZE_DND);
                
                gchar *basename = g_path_get_basename(filename);
                GtkWidget *label = gtk_label_new(basename);
                gtk_widget_set_halign(label, GTK_ALIGN_START);
                gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END);
                gtk_widget_set_hexpand(label, TRUE);
                g_free(basename);

                // Format selection
                GtkWidget *combo = gtk_combo_box_text_new();
                
                // Determine input type to filter formats
                MediaType input_type = MEDIA_TYPE_UNKNOWN;
                const gchar *ext = g_strrstr(filename, ".");
                if (ext) {
                    input_type = get_media_type_from_format(ext + 1);
                }

                const gchar *image_formats[] = {"JPG", "PNG", "WEBP", "BMP", "TIFF", "ICO", "TGA", "HEIC", "SVG", "PDF", "EPS", "AVIF", "HDR", NULL};
                const gchar *video_formats[] = {"MP4", "MKV", "AVI", "WEBM", "MOV", "FLV", "GIF",NULL};
                
                const gchar **formats_to_use = image_formats; // default
                if (input_type == MEDIA_TYPE_VIDEO) {
                    formats_to_use = video_formats;
                }
                
                for (int j = 0; formats_to_use[j] != NULL; j++) {
                    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), formats_to_use[j]);
                }
                gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
                gtk_widget_set_valign(combo, GTK_ALIGN_CENTER);

                // Progress Bar
                GtkWidget *progress = gtk_progress_bar_new();
                gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(progress), TRUE);
                gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress), "جاهز");
                gtk_widget_set_valign(progress, GTK_ALIGN_CENTER);
                gtk_widget_set_size_request(progress, 150, -1);

                GtkWidget *remove_btn = gtk_button_new_from_icon_name("window-close-symbolic", GTK_ICON_SIZE_BUTTON);
                GtkStyleContext *btn_ctx = gtk_widget_get_style_context(remove_btn);
                gtk_style_context_add_class(btn_ctx, "circular");
                gtk_style_context_add_class(btn_ctx, "destructive-action");
                gtk_widget_set_valign(remove_btn, GTK_ALIGN_CENTER);

                gtk_box_pack_start(GTK_BOX(box), icon, FALSE, FALSE, 0);
                gtk_box_pack_start(GTK_BOX(box), label, TRUE, TRUE, 0);
                gtk_box_pack_start(GTK_BOX(box), combo, FALSE, FALSE, 0);
                gtk_box_pack_start(GTK_BOX(box), progress, FALSE, FALSE, 0);
                gtk_box_pack_start(GTK_BOX(box), remove_btn, FALSE, FALSE, 0);

                gtk_container_add(GTK_CONTAINER(row), box);
                
                // Attach data for conversion logic
                g_object_set_data_full(G_OBJECT(row), "filename", g_strdup(filename), g_free);
                g_object_set_data(G_OBJECT(row), "combo", combo);
                g_object_set_data(G_OBJECT(row), "progress", progress);

                // Signal to remove row
                g_signal_connect_swapped(remove_btn, "clicked", G_CALLBACK(gtk_widget_destroy), row);


                gtk_list_box_insert(GTK_LIST_BOX(list_box), row, -1);
                gtk_widget_show_all(row);

                g_free(filename);
            }
        }
        g_strfreev(uris);
        gtk_drag_finish(context, TRUE, FALSE, time);
    } else {
        gtk_drag_finish(context, FALSE, FALSE, time);
    }
}

static void on_conversion_progress(double fraction, gpointer user_data) {
    GtkWidget *progress = GTK_WIDGET(user_data);
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress), fraction);
    
    gchar *text = g_strdup_printf("%d%%", (int)(fraction * 100));
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress), text);
    g_free(text);
}

static void on_conversion_finished(gboolean success, gpointer user_data) {
    GtkWidget *progress = GTK_WIDGET(user_data);
    if (success) {
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress), 1.0);
        gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress), "مكتمل!");
    } else {
        gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress), "خطأ!");
        // Reset fraction to 0 so it doesn't look done
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress), 0.0);
    }
}

static void on_convert_all_clicked(GtkButton *btn, gpointer user_data) {
    GList *children = gtk_container_get_children(GTK_CONTAINER(list_box));
    
    // For simplicity, we get the home directory and save to a folder there
    const gchar *home = g_get_home_dir();
    gchar *output_dir = g_build_filename(home, "Videos", "VConvert", NULL);
    g_mkdir_with_parents(output_dir, 0755);

    for (GList *iter = children; iter != NULL; iter = iter->next) {
        GtkWidget *row = GTK_WIDGET(iter->data);
        const gchar *input_filename = g_object_get_data(G_OBJECT(row), "filename");
        GtkWidget *combo = g_object_get_data(G_OBJECT(row), "combo");
        GtkWidget *progress = g_object_get_data(G_OBJECT(row), "progress");
        
        gchar *format = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo));
        if (input_filename && format) {
            gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress), "جاري...");
            gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress), 0.1); // Small indicator that it started
            
            // Build new output filename based on selected format
            gchar *basename = g_path_get_basename(input_filename);
            gchar *name_no_ext = g_strndup(basename, g_strrstr(basename, ".") ? (g_strrstr(basename, ".") - basename) : strlen(basename));
            
            gchar *new_basename = g_strdup_printf("%s.%s", name_no_ext, g_ascii_strdown(format, -1));
            gchar *output_filename = g_build_filename(output_dir, new_basename, NULL);
            
            start_conversion(input_filename, output_filename, format, on_conversion_progress, on_conversion_finished, progress);
            
            g_free(new_basename);
            g_free(output_filename);
            g_free(name_no_ext);
            g_free(basename);
        }
        g_free(format);
    }
    g_list_free(children);
    g_free(output_dir);
}

void show_main_window(GtkApplication *app) {
    load_css();

    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "VConvert");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    // Setup drag and drop on the window
    gtk_drag_dest_set(window, GTK_DEST_DEFAULT_ALL, NULL, 0, GDK_ACTION_COPY);
    gtk_drag_dest_add_uri_targets(window);
    g_signal_connect(window, "drag-data-received", G_CALLBACK(on_drag_data_received), NULL);

    // HeaderBar
    GtkWidget *header = gtk_header_bar_new();
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header), TRUE);
    gtk_header_bar_set_title(GTK_HEADER_BAR(header), "VConvert");
    gtk_header_bar_set_subtitle(GTK_HEADER_BAR(header), "محول الوسائط");
    gtk_window_set_titlebar(GTK_WINDOW(window), header);

    // Main Box
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), main_box);

    // Scrolled window for the list
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_hexpand(scrolled, TRUE);
    gtk_widget_set_vexpand(scrolled, TRUE);
    gtk_box_pack_start(GTK_BOX(main_box), scrolled, TRUE, TRUE, 0);

    // List Box
    list_box = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(list_box), GTK_SELECTION_NONE);
    gtk_container_add(GTK_CONTAINER(scrolled), list_box);

    // Drag and Drop Zone (Placeholder for empty list)
    GtkWidget *drop_zone = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_valign(drop_zone, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(drop_zone, GTK_ALIGN_CENTER);
    GtkStyleContext *drop_ctx = gtk_widget_get_style_context(drop_zone);
    gtk_style_context_add_class(drop_ctx, "drop-zone");

    GtkWidget *drop_icon = gtk_image_new_from_icon_name("document-send-symbolic", GTK_ICON_SIZE_DIALOG);
    gtk_image_set_pixel_size(GTK_IMAGE(drop_icon), 64);
    GtkStyleContext *icon_ctx = gtk_widget_get_style_context(drop_icon);
    gtk_style_context_add_class(icon_ctx, "drop-zone-icon");

    GtkWidget *drop_label = gtk_label_new("قم بسحب الملفات وإفلاتها هنا");
    gtk_label_set_justify(GTK_LABEL(drop_label), GTK_JUSTIFY_CENTER);
    GtkStyleContext *label_ctx = gtk_widget_get_style_context(drop_label);
    gtk_style_context_add_class(label_ctx, "drop-zone-label");

    gtk_box_pack_start(GTK_BOX(drop_zone), drop_icon, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(drop_zone), drop_label, FALSE, FALSE, 0);

    gtk_list_box_set_placeholder(GTK_LIST_BOX(list_box), drop_zone);

    // Bottom Bar
    GtkWidget *bottom_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    GtkStyleContext *bottom_ctx = gtk_widget_get_style_context(bottom_bar);
    gtk_style_context_add_class(bottom_ctx, "bottom-bar");

    GtkWidget *output_label = gtk_label_new("مجلد الإخراج:");
    GtkWidget *output_btn = gtk_button_new_with_label("~/Videos/VConvert");
    
    GtkWidget *convert_btn = gtk_button_new_with_label("تحويل الكل");
    GtkStyleContext *btn_ctx = gtk_widget_get_style_context(convert_btn);
    gtk_style_context_add_class(btn_ctx, "convert-btn");
    
    g_signal_connect(convert_btn, "clicked", G_CALLBACK(on_convert_all_clicked), NULL);

    gtk_box_pack_start(GTK_BOX(bottom_bar), output_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(bottom_bar), output_btn, FALSE, FALSE, 0);
    
    // Push the convert button to the right
    GtkWidget *spacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand(spacer, TRUE);
    gtk_box_pack_start(GTK_BOX(bottom_bar), spacer, TRUE, TRUE, 0);
    
    gtk_box_pack_start(GTK_BOX(bottom_bar), convert_btn, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(main_box), bottom_bar, FALSE, FALSE, 0);

    gtk_widget_show_all(window);
}
