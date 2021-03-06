/*
  Copyright (C) 2007-2008 Tomash Brechko.  All rights reserved.

  When used to build Perl module:

  This library is free software; you can redistribute it and/or modify
  it under the same terms as Perl itself, either Perl version 5.8.8
  or, at your option, any later version of Perl 5 you may have
  available.

  When used as a standalone library:

  This library is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
*/

#ifndef CLIENT_H
#define CLIENT_H 1

#include <stddef.h>
#include "array.h"
#include "dispatch_key.h"


enum server_status {
  MEMCACHED_SUCCESS,
  MEMCACHED_FAILURE,
  MEMCACHED_EAGAIN,
  MEMCACHED_ERROR,
  MEMCACHED_UNKNOWN,
  MEMCACHED_CLOSED
};

enum set_cmd_e { CMD_SET, CMD_ADD, CMD_REPLACE, CMD_APPEND, CMD_PREPEND,
                 CMD_CAS };

enum get_cmd_e { CMD_GET, CMD_GETS };

enum gat_cmd_e { CMD_GAT, CMD_GATS };

enum arith_cmd_e { CMD_INCR, CMD_DECR };


typedef unsigned int flags_type;
#define FMT_FLAGS "%u"

typedef int exptime_type;
#define FMT_EXPTIME "%d"

typedef unsigned int delay_type;
#define FMT_DELAY "%u"

typedef unsigned long value_size_type;
#define FMT_VALUE_SIZE "%lu"

typedef unsigned long long cas_type;
#define FMT_CAS "%llu"

typedef unsigned long long arith_type;
#define FMT_ARITH "%llu"


typedef void *(*alloc_value_func)(value_size_type value_size, void **opaque);
typedef void (*store_value_func)(void *arg, void *opaque, int key_index,
                                 void *meta);
typedef void (*free_value_func)(void *opaque);
typedef unsigned long long generation_type;

struct result_object
{
  alloc_value_func alloc;
  store_value_func store;
  free_value_func free;

  void *arg;
};

struct meta_object
{
  flags_type flags;
  int use_cas;
  cas_type cas;
};

struct client;
typedef void (*error_callback)(struct client *, const char *, char *);

struct client
{
  struct array pollfds;
  struct array servers;

  struct dispatch_state dispatch;

  char *prefix;
  size_t prefix_len;

  int connect_timeout;          /* 1/1000 sec.  */
  int io_timeout;               /* 1/1000 sec.  */
  int max_failures;
  int failure_timeout;          /* 1 sec.  */
  int close_on_error;
  int nowait;
  int hash_namespace;

  struct array index_list;
  struct array str_buf;
  int iov_max;

  generation_type generation;

  struct result_object *object;
  int noreply;
  error_callback error_cb;
  void *memd;
  int no_rehash;
};

extern
struct client *
client_init();

extern
void
client_destroy(struct client *c);

extern
void
client_reinit(struct client *c);

/*
  client_set_ketama_points() should be called before adding any server.
*/
extern
int
client_set_ketama_points(struct client *c, int ketama_points);

/*
  client_set_hash_namespace() should be called before setting the
  namespace.
*/
extern
void
client_set_hash_namespace(struct client *c, int enable);

extern
int
client_add_server(struct client *c, const char *host, size_t host_len,
                  const char *port, size_t port_len, double weight,
                  int noreply);

extern
int
client_set_prefix(struct client *c, const char *ns, size_t ns_len);

extern
const char *
client_get_prefix(struct client *c, size_t *ns_len);

extern
void
client_set_connect_timeout(struct client *c, int to);

extern
void
client_set_io_timeout(struct client *c, int to);

extern
void
client_set_max_failures(struct client *c, int f);

extern
void
client_set_failure_timeout(struct client *c, int to);

extern
void
client_set_close_on_error(struct client *c, int enable);

extern
void
client_set_nowait(struct client *c, int enable);

extern
void
client_reset(struct client *c, struct result_object *o, int noreply);

extern
int
client_prepare_set(struct client *c, enum set_cmd_e cmd, int key_index,
                   const char *key, size_t key_len,
                   flags_type flags, exptime_type exptime,
                   const void *value, value_size_type value_size);

extern
int
client_prepare_cas(struct client *c, int key_index,
                   const char *key, size_t key_len,
                   cas_type cas, flags_type flags, exptime_type exptime,
                   const void *value, value_size_type value_size);

extern
int
client_prepare_get(struct client *c, enum get_cmd_e cmd, int key_index,
                   const char *key, size_t key_len);

extern
int
client_prepare_gat(struct client *c, enum gat_cmd_e cmd,
                   int key_index, const char *key, size_t key_len, const char *exptime, size_t exptime_len);

extern
int
client_prepare_incr(struct client *c, enum arith_cmd_e cmd, int key_index,
                    const char *key, size_t key_len, arith_type arg);

extern
int
client_prepare_delete(struct client *c, int key_index,
                      const char *key, size_t key_len);

extern
int
client_prepare_touch(struct client *c, int key_index,
                      const char* key, size_t key_len,
                      exptime_type exptime);

extern
int
client_execute(struct client *c, int key_index);

extern
int
client_flush_all(struct client *c, delay_type delay,
                 struct result_object *o, int noreply);

extern
int
client_nowait_push(struct client *c);

extern
int
client_server_versions(struct client *c, struct result_object *o);

extern
void
client_set_onerror(struct client *c, void *memd, error_callback onerror);

extern
void
client_set_no_rehash(struct client *c, int enable);

#endif /* ! CLIENT_H */
