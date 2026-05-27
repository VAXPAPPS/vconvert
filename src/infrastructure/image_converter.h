#ifndef IMAGE_CONVERTER_H
#define IMAGE_CONVERTER_H

#include <glib.h>
#include "../usecases/converter.h"

void convert_image_async(const gchar *input_path, const gchar *output_path, 
                         ConversionFinishedCallback finished_cb,
                         gpointer user_data);

#endif // IMAGE_CONVERTER_H
