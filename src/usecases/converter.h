#ifndef CONVERTER_H
#define CONVERTER_H

#include <glib.h>

typedef void (*ConversionProgressCallback)(double fraction, gpointer user_data);
typedef void (*ConversionFinishedCallback)(gboolean success, gpointer user_data);

void start_conversion(const gchar *input_path, const gchar *output_path, const gchar *format,
                      ConversionProgressCallback prog_cb,
                      ConversionFinishedCallback fin_cb,
                      gpointer user_data);

#endif
