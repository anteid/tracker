/**
 * @file libpomp.h
 *
 * @brief Printf Oriented Message Protocol.
 *
 * @author yves-marie.morgan@parrot.com
 *
 * Copyright (c) 2014 Parrot S.A.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the <organization> nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef _LIBPOMP_H_
#define _LIBPOMP_H_

#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** Wrapper for gcc printf attribute */
#ifndef POMP_ATTRIBUTE_FORMAT_PRINTF
#  ifndef _MSC_VER
#    define POMP_ATTRIBUTE_FORMAT_PRINTF(_x, _y) \
			__attribute__((__format__(__printf__, _x, _y)))
#  else /* _MSC_VER */
#    define POMP_ATTRIBUTE_FORMAT_PRINTF(_x, _y)
#  endif /* _MSC_VER */
#endif /* !POMP_ATTRIBUTE_FORMAT_PRINTF */

/** Wrapper for gcc scanf attribute */
#ifndef POMP_ATTRIBUTE_FORMAT_SCANF
#  ifndef _MSC_VER
#    define POMP_ATTRIBUTE_FORMAT_SCANF(_x, _y) \
			__attribute__((__format__(__scanf__, _x, _y)))
#  else /* _MSC_VER */
#    define POMP_ATTRIBUTE_FORMAT_SCANF(_x, _y)
#  endif /* _MSC_VER */
#endif /* !POMP_ATTRIBUTE_FORMAT_SCANF */

/** To be used for all public API */
#ifdef POMP_API_EXPORTS
#  ifdef _WIN32
#    define POMP_API	__declspec(dllexport)
#  else /* !_WIN32 */
#    define POMP_API	__attribute__((visibility("default")))
#  endif /* !_WIN32 */
#else /* !POMP_API_EXPORTS */
#  define POMP_API
#endif /* !POMP_API_EXPORTS */

struct pomp_buffer;
struct pomp_msg;

 /* Buffer API */

/**
 * Allocate a new buffer.
 * @param capacity : initial capacity of the buffer.
 * @return new buffer with initial ref count at 1 or NULL in case of error.
 */
POMP_API struct pomp_buffer *pomp_buffer_new(size_t capacity);

/**
 * Create a new buffer with content copied from another buffer.
 * @param buf : buffer to copy.
 * @return new buffer with initial ref count at 1 and internal data copied from
 * given buffer or NULL in case of error.
 */
POMP_API struct pomp_buffer *pomp_buffer_new_copy(
		const struct pomp_buffer *buf);

/**
 * Create a new buffer with content copied from given data. Internal length and
 * capacity will be set to given data length.
 * @param data : data to copy.
 * @param len : lebth to copy.
 * @return new buffer with initial ref count at 1 and internal data copied from
 * given buffer or NULL in case of error.
 */
POMP_API struct pomp_buffer *pomp_buffer_new_with_data(
		const void *data, size_t len);

/**
 * Same as pomp_buffer_new but retreived internal data pointer at the same time.
 * @param capacity : initial capacity of the buffer.
 * @param data : data of buffer (optional can be NULL).
 * @return new buffer with initial ref count at 1 or NULL in case of error.
 */
POMP_API struct pomp_buffer *pomp_buffer_new_get_data(
		size_t capacity, void **data);

/**
 * Increase ref count of buffer.
 * @param buf : buffer.
 */
POMP_API void pomp_buffer_ref(struct pomp_buffer *buf);

/**
 * Decrease ref count of buffer. When it reaches 0, internal data as well as
 * buffer structure itself is freed.
 * @param buf : buffer.
 */
POMP_API void pomp_buffer_unref(struct pomp_buffer *buf);

/**
 * Set the capacity of the buffer.
 * @param buf : buffer.
 * @param capacity : new capacity of buffer (shall be greater than used length).
 * @return 0 in case of success, negative errno value in case of error.
 * -EPERM is returned if the buffer is shared (ref count is greater than 1).
 */
POMP_API int pomp_buffer_set_capacity(struct pomp_buffer *buf, size_t capacity);

/**
 * Set the used length of the buffer.
 * @param buf : buffer.
 * @param len : used length of data (shall be lesser than allocated size).
 * @return 0 in case of success, negative errno value in case of error.
 * -EPERM is returned if the buffer is shared (ref count is greater than 1) or
 * readonly.
 */
POMP_API int pomp_buffer_set_len(struct pomp_buffer *buf, size_t len);

/**
 * Get internal buffer data for read/write.
 * @param buf : buffer.
 * @param data : data of buffer (optional can be NULL).
 * @param len : used length of buffer (optional can be NULL).
 * @param capacity : capacity of buffer (optional can be NULL).
 * @return 0 in case of success, negative errno value in case of error.
 * -EPERM is returned if the buffer is shared.
 */
POMP_API int pomp_buffer_get_data(struct pomp_buffer *buf,
		void **data, size_t *len, size_t *capacity);

/**
 * Get internal buffer data for read-only.
 * @param buf : buffer
 * @param cdata : data of buffer (optional can be NULL).
 * @param len : used length of buffer (optional can be NULL).
 * @param capacity : capacity of buffer (optional can be NULL).
 * @return 0 in case of success, negative errno value in case of error.
 */
POMP_API int pomp_buffer_get_cdata(struct pomp_buffer *buf,
		const void **cdata, size_t *len, size_t *size);

/*
 * Message API.
 */

/**
 * Create a new message structure.
 * Message is initially empty.
 * @return new message structure or NULL in case of error.
 */
POMP_API struct pomp_msg *pomp_msg_new(void);

/**
 * Create a new message structure.
 * Message will be a copy of given message (with internal buffer copied as
 * well, not just shared).
 * @param msg : message to copy.
 * @return new message structure or NULL in case of error.
 *
 * @remarks : if the message contains file descriptors, they are duplicated in
 * the new message.
 */
POMP_API struct pomp_msg *pomp_msg_new_copy(const struct pomp_msg *msg);

/**
 * Create a new message structure from a buufer with data.
 * @param buf : buffer with message content (header + playload).
 * @return new message structure or NULL in case of error.
 */
POMP_API struct pomp_msg *pomp_msg_new_with_buffer(struct pomp_buffer *buf);

/**
 * Destroy a message.
 * @param msg : message.
 * @return 0 in case of success, negative errno value in case of error.
 */
POMP_API int pomp_msg_destroy(struct pomp_msg *msg);

/**
 * Get the id of the message.
 * @param msg : message.
 * @return id of the message or 0 in case of error.
 */
POMP_API uint32_t pomp_msg_get_id(const struct pomp_msg *msg);

/**
 * Get the internal buffer of the message.
 * @param msg : message.
 * @return internal buffer of the message or NULL in case of error.
 *
 * @remarks this function is useful when only the serialization of the library
 * is needed, not the transport part. This way one can write the data to another
 * transport layer.
 */
POMP_API struct pomp_buffer *pomp_msg_get_buffer(const struct pomp_msg *msg);

/**
 * Write and encode a message.
 * @param msg : message.
 * @param msgid : message id.
 * @param fmt : format string. Can be NULL if no arguments given.
 * @param ... : message arguments.
 * @return 0 in case of success, negative errno value in case of error.
 */
POMP_API int pomp_msg_write(struct pomp_msg *msg, uint32_t msgid,
		const char *fmt, ...) POMP_ATTRIBUTE_FORMAT_PRINTF(3, 4);

/**
 * Write and encode a message.
 * @param msg : message.
 * @param msgid : message id.
 * @param fmt : format string. Can be NULL if no arguments given.
 * @param args : message arguments.
 * @return 0 in case of success, negative errno value in case of error.
 */
POMP_API int pomp_msg_writev(struct pomp_msg *msg, uint32_t msgid,
		const char *fmt, va_list args);

/**
 * Write and encode a message.
 * @param msg : message.
 * @param msgid : message id.
 * @param fmt : format string. Can be NULL if no arguments given.
 * @param argc : number of arguments.
 * @param argv : array of arguments as strings. Each string will be converted
 *               according to its real type specified in format.
 * @return 0 in case of success, negative errno value in case of error.
 *
 * @remarks this is mainly used by the command line tool.
 */
POMP_API int pomp_msg_write_argv(struct pomp_msg *msg, uint32_t msgid,
		const char *fmt, int argc, const char * const *argv);

/**
 * Read and decode a message.
 * @param msg : message.
 * @param fmt : format string. Can be NULL if no arguments given.
 * @param ... : message arguments.
 * @return 0 in case of success, negative errno value in case of error.
 */
POMP_API int pomp_msg_read(const struct pomp_msg *msg,
		const char *fmt, ...) POMP_ATTRIBUTE_FORMAT_SCANF(2, 3);

/**
 * Read and decode a message.
 * @param msg : message.
 * @param fmt : format string. Can be NULL if no arguments given.
 * @param args : message arguments.
 * @return 0 in case of success, negative errno value in case of error.
 */
POMP_API int pomp_msg_readv(const struct pomp_msg *msg,
		const char *fmt, va_list args);

/**
 * Dump a message in a human readable form.
 * @param msg : message.
 * @param dst : destination buffer.
 * @param maxdst : max length of destination buffer.
 * @return 0 in case of success, negative errno value in case of error.

 * @remarks if the buffer is too small, it will be ended with ellipsis '...' if
 * possible. It will also always be null terminated (unless maxlen is 0)
 */
POMP_API int pomp_msg_dump(const struct pomp_msg *msg,
		char *dst, uint32_t maxdst);

/**
 * Dump a message in a human readable form. Similar to pomp_msg_dump() but
 * allocates the output buffer dynamically.
 * Dump a message in a human readable form.
 * @param msg : message.
 * @param dst : destination buffer, which will be allocated to a suitable size
 * dynamically. Must be freed with free(), after usage.
 * @return 0 in case of success, negative errno value in case of error.
 */
POMP_API int pomp_msg_adump(const struct pomp_msg *msg, char **dst);


/* Forward declarations */
struct pomp_encoder;
struct pomp_decoder;

/*
 * Message API (Advanced).
 */

/**
 * Initialize a message object before starting to encode it.
 * @param msg : message.
 * @param msgid : message id.
 * @return 0 in case of success, negative errno value in case of error.
 */
POMP_API int pomp_msg_init(struct pomp_msg *msg, uint32_t msgid);

/**
 * Finish message encoding by writing the header. It shall be called after
 * encoding is done and before sending it. Any write operation on the message
 * will return -EPERM after this function is called.
 * @param msg : message.
 * @return 0 in case of success, negative errno value in case of error.
 */
POMP_API int pomp_msg_finish(struct pomp_msg *msg);

/**
 * Clear message object. It only free the internal data, not the message itself.
 * It shall be called before pomp_msg_init can be called again after a previous
 * encoding.
 * @param msg : message.
 * @return 0 in case of success, negative errno value in case of error.
 */
POMP_API int pomp_msg_clear(struct pomp_msg *msg);

/*
 * Encoder API (Advanced).
 */

/**
 * Create a new encoder object.
 * @return new encoder object or NULL in case of error.
 */
POMP_API struct pomp_encoder *pomp_encoder_new(void);

/**
 * Destroy an encoder object.
 * @param enc : encoder.
 * @return 0 in case of success, negative errno value in case of error.
 *
 * @remarks the associated message is NOT destroyed.
 */
POMP_API int pomp_encoder_destroy(struct pomp_encoder *enc);

/**
 * Initialize an encoder object before starting to encode a message.
 * @param enc : encoder.
 * @param msg : message to encode.
 * @return 0 in case of success, negative errno value in case of error.
 *
 * @remarks the message ownership is not transferred, it will NOT be destroyed
 * when encoder object is destroyed.
 */
POMP_API int pomp_encoder_init(struct pomp_encoder *enc, struct pomp_msg *msg);

/**
 * Clear encoder object.
 * @param enc : encoder.
 * @return 0 in case of success, negative errno value in case of error.
 *
 * @remarks the associated message is NOT destroyed.
 */
POMP_API int pomp_encoder_clear(struct pomp_encoder *enc);

/**
 * Encode arguments according to given format string.
 * @param enc : encoder.
 * @param fmt : format string. Can be NULL if no arguments given.
 * @param ... : arguments.
 * @return 0 in case of success, negative errno value in case of error.
 */
POMP_API int pomp_encoder_write(struct pomp_encoder *enc,
		const char *fmt, ...) POMP_ATTRIBUTE_FORMAT_PRINTF(2, 3);

/**
 * Encode arguments according to given format string.
 * @param enc : encoder.
 * @param fmt : format string. Can be NULL if no arguments given.
 * @param args : arguments.
 * @return 0 in case of success, negative errno value in case of error.
 */
POMP_API int pomp_encoder_writev(struct pomp_encoder *enc,
		const char *fmt, va_list args);

/**
 * Encode arguments according to given format string.
 * @param enc : encoder.
 * @param fmt : format string. Can be NULL if no arguments given.
 * @param argc : number of arguments.
 * @param argv : array of arguments as strings. Each string will be converted
 *               according to its real type specified in format.
 * @return 0 in case of success, negative errno value in case of error.
 *
 * @remarks this is mainly used by the command line tool.
 */
POMP_API int pomp_encoder_write_argv(struct pomp_encoder *enc,
		const char *fmt, int argc, const char * const *argv);

/**
 * Encode a 8-bit signed integer.
 * @param enc : encoder.
 * @param v : value to encode.
 * @return 0 in case of success, negative errno value in case of error.
 */
POMP_API int pomp_encoder_write_i8(struct pomp_encoder *enc, int8_t v);

/**
 * Encode a 8-bit unsigned integer.
 * @param enc : encoder.
 * @param v : value to encode.
 * @return 0 in case of success, negative errno value in case of error.
 */
POMP_API int pomp_encoder_write_u8(struct pomp_encoder *enc, uint8_t v);

/**
 * Encode a 16-bit signed integer.
 * @param enc : encoder.
 * @param v : value to encode.
 * @return 0 in case of success, negative errno value in case of error.
 */
POMP_API int pomp_encoder_write_i16(struct pomp_encoder *enc, int16_t v);

/**
 * Encode a 16-bit unsigned integer.
 * @param enc : encoder.
 * @param v : value to encode.
 * @return 0 in case of success, negative errno value in case of error.
 */
POMP_API int pomp_encoder_write_u16(struct pomp_encoder *enc, uint16_t v);

/**
 * Encode a 32-bit signed integer.
 * @param enc : encoder.
 * @param v : value to encode.
 * @return 0 in case of success, negative errno value in case of error.
 */
POMP_API int pomp_encoder_write_i32(struct pomp_encoder *enc, int32_t v);

/**
 * Encode a 32-bit unsigned integer.
 * @param enc : encoder.
 * @param v : value to encode.
 * @return 0 in case of success, negative errno value in case of error.
 */
POMP_API int pomp_encoder_write_u32(struct pomp_encoder *enc, uint32_t v);

/**
 * Encode a 64-bit signed integer.
 * @param enc : encoder.
 * @param v : value to encode.
 * @return 0 in case of success, negative errno value in case of error.
 */
POMP_API int pomp_encoder_write_i64(struct pomp_encoder *enc, int64_t v);

/**
 * Encode a 64-bit unsigned integer.
 * @param enc : encoder.
 * @param v : value to encode.
 * @return 0 in case of success, negative errno value in case of error.
 */
POMP_API int pomp_encoder_write_u64(struct pomp_encoder *enc, uint64_t v);

/**
 * Encode a string.
 * @param enc : encoder.
 * @param v : string to encode.
 * @return 0 in case of success, negative errno value in case of error.
 */
POMP_API int pomp_encoder_write_str(struct pomp_encoder *enc, const char *v);

/**
 * Encode a buffer.
 * @param enc : encoder.
 * @param v : buffer to encode.
 * @param n : buffer size.
 * @return 0 in case of success, negative errno value in case of error.
 */
POMP_API int pomp_encoder_write_buf(struct pomp_encoder *enc, const void *v,
		uint32_t n);

/**
 * Encode a 32-bit floating point.
 * @param enc : encoder.
 * @param v : value to encode.
 * @return 0 in case of success, negative errno value in case of error.
 */
POMP_API int pomp_encoder_write_f32(struct pomp_encoder *enc, float v);

/**
 * Encode a 64-bit floating point.
 * @param enc : encoder.
 * @param v : value to encode.
 * @return 0 in case of success, negative errno value in case of error.
 */
POMP_API int pomp_encoder_write_f64(struct pomp_encoder *enc, double v);

/**
 * Encode a file descriptor. It will be internally duplicated and close when
 * associated message is released.
 * @param enc : encoder.
 * @param v : value to encode.
 * @return 0 in case of success, negative errno value in case of error.
 *
 * @remarks : the encoded message will only be able to be exchanged over
 * unix local socket.
 */
POMP_API int pomp_encoder_write_fd(struct pomp_encoder *enc, int v);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !_LIBPOMP_H_ */
