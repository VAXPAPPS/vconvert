#include "ffmpeg_core.h"
#include <gio/gio.h>

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

void convert_media_async(const gchar *input_path, const gchar *output_path,
                         ConversionFinishedCallback finished_cb,
                         gpointer user_data) {
    GError *error = NULL;
    
    ConversionTask *task = g_new(ConversionTask, 1);
    task->finished_cb = finished_cb;
    task->user_data = user_data;
    
    g_print("Starting conversion: %s -> %s\n", input_path, output_path);
    
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
