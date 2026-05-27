#ifndef AUDIO_CONVERTER_H
#define AUDIO_CONVERTER_H

#include <glib.h>
#include "../usecases/converter.h"

void convert_audio_async(const gchar *input_path, const gchar *output_path,
                         ConversionProgressCallback prog_cb,
                         ConversionFinishedCallback fin_cb,
                         gpointer user_data);

#endif // AUDIO_CONVERTER_H
