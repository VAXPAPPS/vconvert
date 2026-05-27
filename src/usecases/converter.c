#include "converter.h"
#include "../domain/media_types.h"
#include "../infrastructure/image_converter.h"
#include "../infrastructure/video_converter.h"
#include <stdio.h>

void start_conversion(const gchar *input_path, const gchar *output_path, const gchar *format,
                      ConversionProgressCallback prog_cb,
                      ConversionFinishedCallback fin_cb,
                      gpointer user_data) {
    
    MediaType type = get_media_type_from_format(format);
    
    if (type == MEDIA_TYPE_IMAGE) {
        // Images convert very fast, simulate a single 50% jump before finish
        if (prog_cb) prog_cb(0.5, user_data);
        convert_image_async(input_path, output_path, fin_cb, user_data);
    } else if (type == MEDIA_TYPE_VIDEO) {
        convert_video_async(input_path, output_path, prog_cb, fin_cb, user_data);
    } else {
        g_printerr("Unsupported format type for conversion: %s\n", format);
        if (fin_cb) fin_cb(FALSE, user_data);
    }
}
