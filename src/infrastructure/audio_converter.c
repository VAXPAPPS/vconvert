#include "audio_converter.h"
#include <gio/gio.h>
#include <stdio.h>

typedef struct {
    ConversionProgressCallback prog_cb;
    ConversionFinishedCallback fin_cb;
    gpointer user_data;
    double duration;
} AudioTask;

typedef struct {
    AudioTask *task;
    double fraction;
} AudioProgressUpdate;

static gboolean dispatch_audio_progress(gpointer data) {
    AudioProgressUpdate *pu = (AudioProgressUpdate *)data;
    if (pu->task->prog_cb) {
        pu->task->prog_cb(pu->fraction, pu->task->user_data);
    }
    g_free(pu);
    return G_SOURCE_REMOVE;
}

static void on_audio_stderr_read(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GDataInputStream *stream = G_DATA_INPUT_STREAM(source_object);
    AudioTask *task = (AudioTask *)user_data;
    GError *error = NULL;
    gsize length;
    
    gchar *line = g_data_input_stream_read_line_finish(stream, res, &length, &error);
    if (line != NULL) {
        if (task->duration == 0.0) {
            gchar *dur_str = g_strstr_len(line, -1, "Duration: ");
            if (dur_str) {
                int h = 0, m = 0; double s = 0.0;
                if (sscanf(dur_str, "Duration: %d:%d:%lf", &h, &m, &s) == 3) {
                    task->duration = h * 3600 + m * 60 + s;
                }
            }
        }
        
        gchar *time_str = g_strstr_len(line, -1, "time=");
        if (time_str && task->duration > 0.0) {
            int h = 0, m = 0; double s = 0.0;
            if (sscanf(time_str, "time=%d:%d:%lf", &h, &m, &s) == 3) {
                double current = h * 3600 + m * 60 + s;
                double fraction = current / task->duration;
                if (fraction > 1.0) fraction = 1.0;
                if (fraction < 0.0) fraction = 0.0;
                
                AudioProgressUpdate *pu = g_new(AudioProgressUpdate, 1);
                pu->task = task;
                pu->fraction = fraction;
                g_idle_add(dispatch_audio_progress, pu);
            }
        }
        
        g_free(line);
        g_data_input_stream_read_line_async(stream, G_PRIORITY_DEFAULT, NULL, on_audio_stderr_read, task);
    } else {
        if (error) g_error_free(error);
        g_object_unref(stream);
    }
}

static void on_audio_subprocess_finished(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GSubprocess *subprocess = G_SUBPROCESS(source_object);
    AudioTask *task = (AudioTask *)user_data;
    GError *error = NULL;
    gboolean success = FALSE;
    
    if (g_subprocess_wait_finish(subprocess, res, &error)) {
        if (g_subprocess_get_if_exited(subprocess) && g_subprocess_get_exit_status(subprocess) == 0) {
            success = TRUE;
        }
    } else {
        g_printerr("Audio conversion failed: %s\n", error->message);
        g_error_free(error);
    }
    
    if (task->fin_cb) {
        task->fin_cb(success, task->user_data);
    }
    
    g_free(task);
    g_object_unref(subprocess);
}

void convert_audio_async(const gchar *input_path, const gchar *output_path,
                         ConversionProgressCallback prog_cb,
                         ConversionFinishedCallback fin_cb,
                         gpointer user_data) {
    GError *error = NULL;
    
    AudioTask *task = g_new0(AudioTask, 1);
    task->prog_cb = prog_cb;
    task->fin_cb = fin_cb;
    task->user_data = user_data;
    task->duration = 0.0;
    
    g_print("Starting audio conversion: %s -> %s\n", input_path, output_path);
    
    // We add -vn to drop video streams (useful if user throws a video to extract audio)
    GSubprocess *subprocess = g_subprocess_new(
        G_SUBPROCESS_FLAGS_STDOUT_PIPE | G_SUBPROCESS_FLAGS_STDERR_PIPE,
        &error,
        "ffmpeg", "-y", "-i", input_path, "-vn", output_path, NULL
    );
    
    if (error != NULL) {
        g_printerr("Failed to spawn ffmpeg for audio: %s\n", error->message);
        g_error_free(error);
        if (fin_cb) fin_cb(FALSE, user_data);
        g_free(task);
        return;
    }
    
    GInputStream *stderr_stream = g_subprocess_get_stderr_pipe(subprocess);
    GDataInputStream *data_stream = g_data_input_stream_new(stderr_stream);
    
    g_data_input_stream_read_line_async(data_stream, G_PRIORITY_DEFAULT, NULL, on_audio_stderr_read, task);
    g_subprocess_wait_async(subprocess, NULL, on_audio_subprocess_finished, task);
}
