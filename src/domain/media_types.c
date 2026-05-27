#include "media_types.h"

MediaType get_media_type_from_format(const gchar *format) {
    if (!format) return MEDIA_TYPE_UNKNOWN;
    
    gchar *fmt = g_ascii_strdown(format, -1);
    MediaType type = MEDIA_TYPE_UNKNOWN;
    
    if (g_strcmp0(fmt, "jpg") == 0 || g_strcmp0(fmt, "jpeg") == 0 ||
        g_strcmp0(fmt, "png") == 0 || g_strcmp0(fmt, "webp") == 0 ||
        g_strcmp0(fmt, "bmp") == 0 || g_strcmp0(fmt, "tiff") == 0 ||
        g_strcmp0(fmt, "gif") == 0 || g_strcmp0(fmt, "ico") == 0 ||
        g_strcmp0(fmt, "tga") == 0 || g_strcmp0(fmt, "heic") == 0 ||
        g_strcmp0(fmt, "svg") == 0 || g_strcmp0(fmt, "pdf") == 0 ||
        g_strcmp0(fmt, "eps") == 0 || g_strcmp0(fmt, "avif") == 0 ||
        g_strcmp0(fmt, "hdr") == 0) {
        type = MEDIA_TYPE_IMAGE;
    } 
    else if (g_strcmp0(fmt, "mp4") == 0 || g_strcmp0(fmt, "mkv") == 0 ||
             g_strcmp0(fmt, "avi") == 0 || g_strcmp0(fmt, "webm") == 0 ||
             g_strcmp0(fmt, "mov") == 0 || g_strcmp0(fmt, "flv") == 0) {
        type = MEDIA_TYPE_VIDEO;
    }
    else if (g_strcmp0(fmt, "mp3") == 0 || g_strcmp0(fmt, "wav") == 0 ||
             g_strcmp0(fmt, "flac") == 0 || g_strcmp0(fmt, "ogg") == 0) {
        type = MEDIA_TYPE_AUDIO;
    }
    
    g_free(fmt);
    return type;
}
