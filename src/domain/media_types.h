#ifndef MEDIA_TYPES_H
#define MEDIA_TYPES_H

#include <glib.h>

typedef enum {
    MEDIA_TYPE_IMAGE,
    MEDIA_TYPE_VIDEO,
    MEDIA_TYPE_AUDIO,
    MEDIA_TYPE_UNKNOWN
} MediaType;

MediaType get_media_type_from_format(const gchar *format);

#endif // MEDIA_TYPES_H
