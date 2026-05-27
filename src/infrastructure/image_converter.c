#include "image_converter.h"
#include <gio/gio.h>
#include <cairo.h>
#include <cairo-pdf.h>
#include <cairo-svg.h>
#include <cairo-ps.h>
#include <gdk/gdk.h>

typedef struct {
    ConversionFinishedCallback finished_cb;
    gpointer user_data;
} ConversionTask;

static void on_subprocess_finished(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GSubprocess *subprocess = G_SUBPROCESS(source_object);
    ConversionTask *task = (ConversionTask *)user_data;
    GError *error = NULL;
    gboolean success = FALSE;
    
    if (g_subprocess_wait_finish(subprocess, res, &error)) {
        if (g_subprocess_get_if_exited(subprocess) && g_subprocess_get_exit_status(subprocess) == 0) {
            success = TRUE;
        }
    } else {
        g_printerr("Conversion failed: %s\n", error->message);
        g_error_free(error);
    }
    
    if (task->finished_cb) {
        task->finished_cb(success, task->user_data);
    }
    
    g_free(task);
    g_object_unref(subprocess);
}

static gboolean convert_with_cairo(const gchar *input, const gchar *output, const gchar *ext) {
    GError *err = NULL;
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(input, &err);
    if (!pixbuf) {
        g_printerr("Failed to load image for cairo conversion: %s\n", err->message);
        g_error_free(err);
        return FALSE;
    }
    
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    cairo_surface_t *surface = NULL;
    
    if (g_ascii_strcasecmp(ext, "pdf") == 0) {
        surface = cairo_pdf_surface_create(output, width, height);
    } else if (g_ascii_strcasecmp(ext, "svg") == 0) {
        surface = cairo_svg_surface_create(output, width, height);
    } else if (g_ascii_strcasecmp(ext, "eps") == 0) {
        surface = cairo_ps_surface_create(output, width, height);
        cairo_ps_surface_set_eps(surface, TRUE);
    }
    
    if (surface) {
        cairo_t *cr = cairo_create(surface);
        gdk_cairo_set_source_pixbuf(cr, pixbuf, 0, 0);
        cairo_paint(cr);
        cairo_show_page(cr);
        cairo_destroy(cr);
        cairo_surface_finish(surface);
        cairo_surface_destroy(surface);
        g_object_unref(pixbuf);
        return TRUE;
    }
    
    g_object_unref(pixbuf);
    return FALSE;
}

void convert_image_async(const gchar *input_path, const gchar *output_path,
                         ConversionFinishedCallback finished_cb,
                         gpointer user_data) {
    GError *error = NULL;
    
    ConversionTask *task = g_new(ConversionTask, 1);
    task->finished_cb = finished_cb;
    task->user_data = user_data;
    
    g_print("Starting conversion: %s -> %s\n", input_path, output_path);
    
    // Intercept PDF, SVG, and EPS to use Cairo natively instead of FFmpeg
    const gchar *ext = g_strrstr(output_path, ".");
    if (ext) {
        ext++; // skip the dot
        if (g_ascii_strcasecmp(ext, "pdf") == 0 || 
            g_ascii_strcasecmp(ext, "svg") == 0 || 
            g_ascii_strcasecmp(ext, "eps") == 0) {
            
            gboolean success = convert_with_cairo(input_path, output_path, ext);
            if (finished_cb) {
                finished_cb(success, user_data);
            }
            g_free(task);
            return;
        }
    }
    
    GSubprocess *subprocess = g_subprocess_new(
        G_SUBPROCESS_FLAGS_STDOUT_PIPE | G_SUBPROCESS_FLAGS_STDERR_PIPE,
        &error,
        "ffmpeg", "-y", "-hide_banner", "-i", input_path, output_path, NULL
    );
    
    if (error != NULL) {
        g_printerr("Failed to spawn ffmpeg: %s\n", error->message);
        g_error_free(error);
        if (finished_cb) finished_cb(FALSE, user_data);
        g_free(task);
        return;
    }
    
    g_subprocess_wait_async(subprocess, NULL, on_subprocess_finished, task);
}
