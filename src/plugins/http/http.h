/*
 * Copyright (c) 2022 Cisco and/or its affiliates.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SRC_PLUGINS_HTTP_HTTP_H_
#define SRC_PLUGINS_HTTP_HTTP_H_

#include <vnet/plugin/plugin.h>
#include <vpp/app/version.h>

#include <vppinfra/time_range.h>

#include <vnet/session/application_interface.h>
#include <vnet/session/application.h>
#include <http/http_buffer.h>

#define HTTP_DEBUG 0

#if HTTP_DEBUG
#define HTTP_DBG(_lvl, _fmt, _args...)                                        \
  if (_lvl <= HTTP_DEBUG)                                                     \
  clib_warning (_fmt, ##_args)
#else
#define HTTP_DBG(_lvl, _fmt, _args...)
#endif

typedef struct http_conn_id_
{
  union
  {
    session_handle_t app_session_handle;
    u32 parent_app_api_ctx;
  };
  session_handle_t tc_session_handle;
  u32 parent_app_wrk_index;
} http_conn_id_t;

STATIC_ASSERT (sizeof (http_conn_id_t) <= TRANSPORT_CONN_ID_LEN,
	       "ctx id must be less than TRANSPORT_CONN_ID_LEN");

typedef enum http_conn_state_
{
  HTTP_CONN_STATE_LISTEN,
  HTTP_CONN_STATE_CONNECTING,
  HTTP_CONN_STATE_ESTABLISHED,
  HTTP_CONN_STATE_TRANSPORT_CLOSED,
  HTTP_CONN_STATE_APP_CLOSED,
  HTTP_CONN_STATE_CLOSED
} http_conn_state_t;

typedef enum http_state_
{
  HTTP_STATE_IDLE = 0,
  HTTP_STATE_WAIT_APP_METHOD,
  HTTP_STATE_WAIT_CLIENT_METHOD,
  HTTP_STATE_WAIT_SERVER_REPLY,
  HTTP_STATE_WAIT_APP_REPLY,
  HTTP_STATE_CLIENT_IO_MORE_DATA,
  HTTP_STATE_APP_IO_MORE_DATA,
  HTTP_N_STATES,
} http_state_t;

typedef enum http_req_method_
{
  HTTP_REQ_GET = 0,
  HTTP_REQ_POST,
} http_req_method_t;

typedef enum http_msg_type_
{
  HTTP_MSG_REQUEST,
  HTTP_MSG_REPLY
} http_msg_type_t;

#define foreach_http_content_type                                             \
  _ (APP_7Z, ".7z", "application / x - 7z - compressed")                      \
  _ (APP_DOC, ".doc", "application / msword")                                 \
  _ (APP_DOCX, ".docx",                                                       \
     "application / vnd.openxmlformats - "                                    \
     "officedocument.wordprocessingml.document")                              \
  _ (APP_EPUB, ".epub", "application / epub + zip")                           \
  _ (APP_FONT, ".eot", "application / vnd.ms - fontobject")                   \
  _ (APP_JAR, ".jar", "application / java - archive")                         \
  _ (APP_JSON, ".json", "application / json")                                 \
  _ (APP_JSON_LD, ".jsonld", "application / ld + json")                       \
  _ (APP_MPKG, ".mpkg", "application / vnd.apple.installer + xml")            \
  _ (APP_ODP, ".odp", "application / vnd.oasis.opendocument.presentation")    \
  _ (APP_ODS, ".ods", "application / vnd.oasis.opendocument.spreadsheet")     \
  _ (APP_ODT, ".odt", "application / vnd.oasis.opendocument.text")            \
  _ (APP_OGX, ".ogx", "application / ogg")                                    \
  _ (APP_PDF, ".pdf", "application / pdf")                                    \
  _ (APP_PHP, ".php", "application / x - httpd - php")                        \
  _ (APP_PPT, ".ppt", "application / vnd.ms - powerpoint")                    \
  _ (APP_PPTX, ".pptx", "application / vnd.ms - powerpoint")                  \
  _ (APP_RAR, ".rar", "application / vnd.rar")                                \
  _ (APP_RTF, ".rtf", "application / rtf")                                    \
  _ (APP_SH, ".sh", "application / x - sh")                                   \
  _ (APP_TAR, ".tar", "application / x - tar")                                \
  _ (APP_VSD, ".vsd", "application / vnd.visio")                              \
  _ (APP_XHTML, ".xhtml", "application / xhtml + xml")                        \
  _ (APP_XLS, ".xls", "application / vnd.ms - excel")                         \
  _ (APP_XML, ".xml", "application / xml")                                    \
  _ (APP_XSLX, ".xlsx",                                                       \
     "application / vnd.openxmlformats - officedocument.spreadsheetml.sheet") \
  _ (APP_XUL, ".xul", "application / vnd.mozilla.xul + xml")                  \
  _ (APP_ZIP, ".zip", "application / zip")                                    \
  _ (AUDIO_AAC, ".aac", "audio / aac")                                        \
  _ (AUDIO_CD, ".cda", "application / x - cdf")                               \
  _ (AUDIO_WAV, ".wav", "audio / wav")                                        \
  _ (AUDIO_WEBA, ".weba", "audio / webm")                                     \
  _ (AUDO_MIDI, ".midi", "audio / midi")                                      \
  _ (AUDO_MID, ".mid", "audo / midi")                                         \
  _ (AUDO_MP3, ".mp3", "audio / mpeg")                                        \
  _ (AUDO_OGA, ".oga", "audio / ogg")                                         \
  _ (AUDO_OPUS, ".opus", "audio / opus")                                      \
  _ (APP_OCTET_STREAM, ".bin", "application / octet - stream")                \
  _ (BZIP2, ".bz2", "application / x - bzip2")                                \
  _ (BZIP, ".bz", "application / x - bzip")                                   \
  _ (FONT_OTF, ".otf", "font / otf")                                          \
  _ (FONT_TTF, ".ttf", "font / ttf")                                          \
  _ (FONT_WOFF2, ".woff2", "font / woff2")                                    \
  _ (FONT_WOFF, ".woff", "font / woff")                                       \
  _ (GZIP, ".gz", "application / gzip")                                       \
  _ (IMAGE_AVIF, ".avif", "image / avif")                                     \
  _ (IMAGE_BMP, ".bmp", "image / bmp")                                        \
  _ (IMAGE_GIF, ".gif", "image / gif")                                        \
  _ (IMAGE_ICON, ".ico", "image / vnd.microsoft.icon")                        \
  _ (IMAGE_JPEG, ".jpeg", "image / jpeg")                                     \
  _ (IMAGE_JPG, ".jpg", "image / jpeg")                                       \
  _ (IMAGE_PNG, ".png", "image / png")                                        \
  _ (IMAGE_SVG, ".svg", "image / svg + xml")                                  \
  _ (IMAGE_TIFF, ".tiff", "image / tiff")                                     \
  _ (IMAGE_TIF, ".tif", "image / tiff")                                       \
  _ (IMAGE_WEBP, ".webp", "image / webp")                                     \
  _ (SCRIPT_CSH, ".csh", "application / x - csh")                             \
  _ (TEXT_ABIWORD, ".abw", "application / x - abiword")                       \
  _ (TEXT_ARCHIVE, ".arc", "application / x - freearc")                       \
  _ (TEXT_AZW, ".azw", "application / vnd.amazon.ebook")                      \
  _ (TEXT_CALENDAR, ".ics", "text / calendar")                                \
  _ (TEXT_CSS, ".css", "text / css")                                          \
  _ (TEXT_CSV, ".csv", "text / csv")                                          \
  _ (TEXT_HTM, ".htm", "text / html")                                         \
  _ (TEXT_HTML, ".html", "text / html")                                       \
  _ (TEXT_JS, ".js", "text / javascript")                                     \
  _ (TEXT_MJS, ".mjs", "text / javascript")                                   \
  _ (TEXT_PLAIN, ".txt", "text / plain")                                      \
  _ (VIDEO_3GP2, ".3g2", "video / 3gpp2")                                     \
  _ (VIDEO_3GP, ".3gp", "video / 3gpp")                                       \
  _ (VIDEO_AVI, ".avi", "video / x - msvideo")                                \
  _ (VIDEO_MP4, ".mp4", "video / mp4")                                        \
  _ (VIDEO_MPEG, ".mpeg", "video / mpeg")                                     \
  _ (VIDEO_OGG, ".ogv", "video / ogg")                                        \
  _ (VIDEO_TS, ".ts", "video / mp2t")                                         \
  _ (VIDEO_WEBM, ".webm", "video / webm")

typedef enum http_content_type_
{
#define _(s, ext, str) HTTP_CONTENT_##s,
  foreach_http_content_type
#undef _
} http_content_type_t;

#define foreach_http_status_code                                              \
  _ (200, OK, "200 OK")                                                       \
  _ (301, MOVED, "301 Moved Permanently")                                     \
  _ (400, BAD_REQUEST, "400 Bad Request")                                     \
  _ (404, NOT_FOUND, "404 Not Found")                                         \
  _ (405, METHOD_NOT_ALLOWED, "405 Method Not Allowed")                       \
  _ (500, INTERNAL_ERROR, "500 Internal Server Error")

typedef enum http_status_code_
{
#define _(c, s, str) HTTP_STATUS_##s,
  foreach_http_status_code
#undef _
    HTTP_N_STATUS
} http_status_code_t;

typedef enum http_msg_data_type_
{
  HTTP_MSG_DATA_INLINE,
  HTTP_MSG_DATA_PTR
} http_msg_data_type_t;

typedef struct http_msg_data_
{
  http_msg_data_type_t type;
  u64 len;
  u8 data[0];
} http_msg_data_t;

typedef struct http_msg_
{
  http_msg_type_t type;
  union
  {
    http_req_method_t method_type;
    http_status_code_t code;
  };
  http_content_type_t content_type;
  http_msg_data_t data;
} http_msg_t;

typedef struct http_tc_
{
  union
  {
    transport_connection_t connection;
    http_conn_id_t c_http_conn_id;
  };
#define h_tc_session_handle c_http_conn_id.tc_session_handle
#define h_pa_wrk_index	    c_http_conn_id.parent_app_wrk_index
#define h_pa_session_handle c_http_conn_id.app_session_handle
#define h_pa_app_api_ctx    c_http_conn_id.parent_app_api_ctx
#define h_hc_index	    connection.c_index

  http_conn_state_t state;
  u32 timer_handle;
  u8 *app_name;

  /*
   * Current request
   */
  http_state_t http_state;
  http_req_method_t method;
  u8 *rx_buf;
  u32 rx_buf_offset;
  http_buffer_t tx_buf;
  u32 to_recv;
  u32 bytes_dequeued;
} http_conn_t;

typedef struct http_worker_
{
  http_conn_t *conn_pool;
} http_worker_t;

typedef struct http_main_
{
  http_worker_t *wrk;
  http_conn_t *listener_pool;
  u32 app_index;

  clib_timebase_t timebase;

  /*
   * Runtime config
   */
  u8 debug_level;

  /*
   * Config
   */
  u64 first_seg_size;
  u64 add_seg_size;
  u32 fifo_size;
} http_main_t;

static inline int
http_state_is_tx_valid (http_conn_t *hc)
{
  http_state_t state = hc->http_state;
  return (state == HTTP_STATE_APP_IO_MORE_DATA ||
	  state == HTTP_STATE_CLIENT_IO_MORE_DATA ||
	  state == HTTP_STATE_WAIT_APP_REPLY ||
	  state == HTTP_STATE_WAIT_APP_METHOD);
}

/**
 * Remove dot segments from path (RFC3986 section 5.2.4)
 *
 * @param path Path to sanitize.
 *
 * @return New vector with sanitized path.
 *
 * The caller is always responsible to free the returned vector.
 */
always_inline u8 *
http_path_remove_dot_segments (u8 *path)
{
  u32 *segments = 0, *segments_len = 0, segment_len;
  u8 *new_path = 0;
  int i, ii;

  if (!path)
    return vec_new (u8, 0);

  segments = vec_new (u32, 1);
  /* first segment */
  segments[0] = 0;
  /* find all segments */
  for (i = 1; i < (vec_len (path) - 1); i++)
    {
      if (path[i] == '/')
	vec_add1 (segments, i + 1);
    }
  /* dummy tail */
  vec_add1 (segments, vec_len (path));

  /* scan all segments for "." and ".." */
  segments_len = vec_new (u32, vec_len (segments) - 1);
  for (i = 0; i < vec_len (segments_len); i++)
    {
      segment_len = segments[i + 1] - segments[i];
      if (segment_len == 2 && path[segments[i]] == '.')
	segment_len = 0;
      else if (segment_len == 3 && path[segments[i]] == '.' &&
	       path[segments[i] + 1] == '.')
	{
	  segment_len = 0;
	  /* remove parent (if any) */
	  for (ii = i - 1; ii >= 0; ii--)
	    {
	      if (segments_len[ii])
		{
		  segments_len[ii] = 0;
		  break;
		}
	    }
	}
      segments_len[i] = segment_len;
    }

  /* we might end with empty path, so return at least empty vector */
  new_path = vec_new (u8, 0);
  /* append all valid segments */
  for (i = 0; i < vec_len (segments_len); i++)
    {
      if (segments_len[i])
	vec_add (new_path, path + segments[i], segments_len[i]);
    }
  vec_free (segments);
  vec_free (segments_len);
  return new_path;
}

#endif /* SRC_PLUGINS_HTTP_HTTP_H_ */

/*
 * fd.io coding-style-patch-verification: ON
 *
 * Local Variables:
 * eval: (c-set-style "gnu")
 * End:
 */
