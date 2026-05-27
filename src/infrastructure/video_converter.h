#ifndef VIDEO_CONVERTER_H
#define VIDEO_CONVERTER_H

#include <glib.h>
#include "../usecases/converter.h"

void convert_video_async(const gchar *input_path, const gchar *output_path,
                         ConversionProgressCallback prog_cb,
                         ConversionFinishedCallback fin_cb,
                         gpointer user_data);

#endif // VIDEO_CONVERTER_H
