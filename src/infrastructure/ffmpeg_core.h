#ifndef FFMPEG_CORE_H
#define FFMPEG_CORE_H

#include <glib.h>

typedef void (*ConversionFinishedCallback)(gboolean success, gpointer user_data);

void convert_media_async(const gchar *input_path, const gchar *output_path, 
                         ConversionFinishedCallback finished_cb,
                         gpointer user_data);

#endif // FFMPEG_CORE_H
