#include "pillowtalk_impl.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
//#include <curl/types.h>
#include <curl/easy.h>

#include <yajl/yajl_gen.h>
#include <yajl/yajl_parse.h>
#ifdef PT_HAVE_YAJL_VERSION
#include <yajl/yajl_version.h>
#endif
#include <assert.h>

#include "bsd_queue.h"

/* YAJL Version checking */
//#if defined(YAJL_MAJOR) && (YAJL_MAJOR > 1)
#  define HAVE_YAJL_V2 1
//#endif

/* Structs */
struct memory_chunk {
    char *memory;
    char *offset;
    size_t size;
};

/* Prototypes */
static pt_response_t* http_operation(const char* method,const char* server_target, const char* data, unsigned data_len);
static void *myrealloc(void *ptr, size_t size);
static size_t recv_memory_callback(void *ptr, size_t size, size_t nmemb, void *data);
static size_t send_memory_callback(void *ptr, size_t size, size_t nmemb, void *data);
static int json_null(void* ctx);
static int json_boolean(void* ctx,int boolean);
#ifdef HAVE_YAJL_V2
static int json_map_key(void * ctx, const unsigned char* str, size_t length);
static int json_string(void* ctx, const unsigned char* str, size_t length);
static int json_integer(void* ctx,long long integer);
#else
static int json_map_key(void * ctx, const unsigned char* str, unsigned int length);
static int json_string(void* ctx, const unsigned char* str, unsigned int length);
static int json_integer(void* ctx,long integer);
#endif
static int json_double(void* ctx,double dbl);
static int json_start_map(void* ctx);
static int json_end_map(void* ctx);
static int json_start_array(void* ctx);
static int json_end_array(void* ctx);
static void generate_map_json(pt_map_t* map, yajl_gen g);
static void generate_array_json(pt_array_t* map , yajl_gen g);
static void generate_node_json(pt_node_t* node, yajl_gen g);
static void free_map_node(pt_map_t* map);
static void add_node_to_context_container(pt_parser_ctx_t* context, pt_node_t* value);
static pt_node_t* parse_json(const char* json, int json_len);
static pt_response_t *put_patch_or_post_operation(const char *method, const char *server_target, pt_node_t *doc);
static pt_response_t *put_patch_or_post_raw_operation(const char *method, const char *server_target, const char *data, unsigned int data_len);

/* Globals */
static yajl_callbacks callbacks = {
    json_null, // null
    json_boolean, // boolean
    json_integer, // integer
    json_double, // double
    NULL, // number_string
    json_string, // string
    json_start_map, // start map
    json_map_key, // MAP KEY
    json_end_map, // end map
    json_start_array, // start array
    json_end_array, // end array
};


/* Public Implementation */

void pt_init()
{
    curl_global_init(CURL_GLOBAL_ALL);
}

void pt_cleanup()
{
    /* we're done with libcurl, so clean it up */
    curl_global_cleanup();
}

void pt_free_response(pt_response_t* response)
{
    if (response) {
        if (response->root) {
            pt_free_node(response->root);
        }
        free(response->raw_json);
        free(response);
    }
}

void free_parser_ctx(pt_parser_ctx_t* parser_ctx)
{
    while(parser_ctx->stack) {
        pt_container_ctx_t* old_head = parser_ctx->stack;
        LL_DELETE(parser_ctx->stack,old_head);
        free(old_head);
    }
    free(parser_ctx);
}

pt_response_t* pt_delete(const char* server_target)
{
    pt_response_t* res = http_operation("DELETE",server_target,NULL,0);
    res->root = parse_json(res->raw_json,res->raw_json_len);
    return res;
}

pt_response_t* pt_put(const char* server_target, pt_node_t* doc)
{
    return put_patch_or_post_operation("PUT", server_target, doc);
}

pt_response_t* pt_put_raw(const char* server_target, const char* data, unsigned int data_len)
{
    return put_patch_or_post_raw_operation("PUT", server_target, data, data_len);
}

pt_response_t *pt_post(const char *server_target, pt_node_t *doc)
{
    return put_patch_or_post_operation("POST", server_target, doc);
}

pt_response_t *pt_post_raw(const char *server_target, const char *data, unsigned int data_len)
{
    return put_patch_or_post_raw_operation("POST", server_target, data, data_len);
}

pt_response_t *pt_patch(const char *server_target, pt_node_t *doc)
{
	return put_patch_or_post_operation("PATCH", server_target, doc);
}

pt_response_t *pt_patch_raw(const char *server_target, const char *data, unsigned int data_len)
{
	return put_patch_or_post_raw_operation("PATCH", server_target, data, data_len);
}

pt_response_t* pt_unparsed_get(const char* server_target)
{
    pt_response_t* res = http_operation("GET",server_target,NULL,0);
    return res;
}

pt_response_t* pt_get(const char* server_target)
{
    pt_response_t* res = http_operation("GET",server_target,NULL,0);
    res->root = parse_json(res->raw_json,res->raw_json_len);
    return res;
}

pt_node_t* pt_map_get(pt_node_t* map,const char* key)
{
    if (map && map->type == PT_MAP && key) {
        pt_map_t* real_map = (pt_map_t*) map;
        pt_key_value_t* search_result = NULL;
		HASH_FIND(hh, real_map->key_values, key, strlen(key), search_result);
        if (search_result) {
            return search_result->value;
        } else {
            return NULL;
        }
    } else {
        return NULL;
    }
}

unsigned int pt_array_len(pt_node_t* array)
{
    if (array && array->type == PT_ARRAY) {
        return ((pt_array_t*) array)->len;
    } else {
        return 0;
    }
}

pt_node_t* pt_array_get(pt_node_t* array, unsigned int idx)
{
    if (array && array->type == PT_ARRAY) {
        pt_array_t* real_array = (pt_array_t*) array;
        int index = 0;
        pt_array_elem_t* cur = NULL;
        TAILQ_FOREACH(cur,&real_array->head, entries) {
            if (index == idx) {
                return cur->node;
            }
            index++;
        }
    }
    return NULL;
}

/* Pass in the pointer to the elem and remove it if it exists */
void pt_array_remove(pt_node_t* array, pt_node_t* node)
{
    if (array && array->type == PT_ARRAY) {
        pt_array_t* real_array = (pt_array_t*) array;
        pt_array_elem_t* cur = NULL;
        pt_array_elem_t* tmp = NULL;
        TAILQ_FOREACH_SAFE(cur,&real_array->head, entries, tmp) {
            if (cur->node == node) {
                TAILQ_REMOVE(&real_array->head,cur,entries);
                pt_free_node(cur->node);
                free(cur);
                real_array->len--;
                break;
            }
        }
    }
}

void pt_array_push_front(pt_node_t* array, pt_node_t* node)
{
    if (array && array->type == PT_ARRAY) {
        pt_array_t* real_array = (pt_array_t*) array;
        pt_array_elem_t* elem = (pt_array_elem_t*) malloc(sizeof(pt_array_elem_t));
        elem->node = node;
        real_array->len++;
        TAILQ_INSERT_HEAD(&real_array->head,elem,entries);
    }
}

void pt_array_push_back(pt_node_t* array, pt_node_t* node)
{
    if (array && array->type == PT_ARRAY) {
        pt_array_t* real_array = (pt_array_t*) array;
        pt_array_elem_t* elem = (pt_array_elem_t*) malloc(sizeof(pt_array_elem_t));
        elem->node = node;
        real_array->len++;
        TAILQ_INSERT_TAIL(&real_array->head,elem,entries);
    }
}

pt_iterator_t* pt_iterator(pt_node_t* node) 
{
    if (node) {
        if (node->type == PT_ARRAY) {
            pt_array_t* real_array = (pt_array_t*) node;
            pt_iterator_impl_t* iter = (pt_iterator_impl_t*) calloc(1,sizeof(pt_iterator_impl_t));
            iter->type = PT_ARRAY_ITERATOR;
            iter->next_array_elem = TAILQ_FIRST(&real_array->head);
            return (pt_iterator_t*) iter;
        } else if (node->type == PT_MAP) {
            pt_map_t* real_map = (pt_map_t*) node;
            pt_iterator_impl_t* iter = (pt_iterator_impl_t*) calloc(1,sizeof(pt_iterator_impl_t));
            iter->type = PT_MAP_ITERATOR;
            iter->next_map_pair = real_map->key_values;
            return (pt_iterator_t*) iter;
        }
    }
    return NULL;
}

pt_node_t* pt_iterator_next(pt_iterator_t* iter, const char** key)
{
    if (iter) {
        pt_iterator_impl_t* real_iter = (pt_iterator_impl_t*) iter;
        if (real_iter->type == PT_MAP_ITERATOR) {
            pt_key_value_t* kv = real_iter->next_map_pair;
            if (kv) {
                pt_node_t* ret = kv->value;
                if (key)
                    *key = kv->key;
                real_iter->next_map_pair = kv->hh.next;
                return ret;
            }
        } else if (real_iter->type == PT_ARRAY_ITERATOR) {
            pt_array_elem_t* elem = real_iter->next_array_elem;
            if (elem) {
                pt_node_t* ret = elem->node;
                real_iter->next_array_elem = TAILQ_NEXT(elem,entries);
                return ret;
            }
        }
    }
    return NULL;
}

int pt_is_null(pt_node_t* null)
{
    return !null || null->type == PT_NULL;
}

int pt_boolean_get(pt_node_t* boolean)
{
    if (boolean && boolean->type == PT_BOOLEAN) {
        pt_bool_value_t* bool_node = (pt_bool_value_t*) boolean;
        return bool_node->value;
    } else {
        return 0;
    }
}

int pt_integer_get(pt_node_t* integer)
{
    if (integer && integer->type == PT_INTEGER) {
        return ((pt_int_value_t*) integer)->value;
    } else if (integer && integer->type == PT_DOUBLE) {
        return (int) ((pt_double_value_t*) integer)->value;
    } else {
        return 0;
    }
}

double pt_double_get(pt_node_t* dbl)
{
    if (dbl && dbl->type == PT_DOUBLE) {
        return ((pt_double_value_t*) dbl)->value;
    } else if (dbl && dbl->type == PT_INTEGER) {
        return (double) ((pt_int_value_t*) dbl)->value;
    } else {
        return 0;
    }
}

const char* pt_string_get(pt_node_t* string)
{
    if (string && string->type == PT_STRING) {
        return ((pt_str_value_t*) string)->value;
    } else {
        return NULL;
    }
}

/* Build a new pt_map_t* and initialize it */
pt_node_t* pt_map_new()
{
    pt_node_t* new_node = (pt_node_t*) calloc(1,sizeof(pt_map_t));
    new_node->type = PT_MAP;
    return new_node;
}

pt_node_t* pt_patch_new(const char* op, const char* path, pt_node_t* value)
{
    pt_node_t* new_node = pt_map_new();
    pt_map_set(new_node, "op", pt_string_new(op));
    pt_map_set(new_node, "path", pt_string_new(path));
    pt_map_set(new_node, "value", value);
    return new_node;
}

pt_node_t* pt_null_new()
{
    pt_node_t* new_node = (pt_node_t*) calloc(1,sizeof(pt_null_value_t));
    new_node->type = PT_NULL;
    return new_node;
}

pt_node_t* pt_bool_new(int boolean)
{
    pt_bool_value_t* new_node = (pt_bool_value_t*) calloc(1,sizeof(pt_bool_value_t));
    new_node->parent.type = PT_BOOLEAN;
    new_node->value = boolean;
    return (pt_node_t*) new_node;
}

pt_node_t* pt_integer_new(int integer)
{
    pt_int_value_t* new_node = (pt_int_value_t*) calloc(1,sizeof(pt_int_value_t));
    new_node->parent.type = PT_INTEGER;
    new_node->value = integer;
    return (pt_node_t*) new_node;
}

pt_node_t* pt_double_new(double dbl)
{
    pt_double_value_t* new_node = (pt_double_value_t*) calloc(1,sizeof(pt_double_value_t));
    new_node->parent.type = PT_DOUBLE;
    new_node->value = dbl;
    return (pt_node_t*) new_node;
}

void pt_map_set(pt_node_t* map, const char* key, pt_node_t* value)
{
    if (map && map->type == PT_MAP && key && value) {
        pt_map_t* real_map = (pt_map_t*) map;
        pt_key_value_t* search_result = NULL;
        HASH_FIND(hh,real_map->key_values,key,strlen(key),search_result);
        if (search_result) {
            // free the old value
            pt_free_node(search_result->value);
            search_result->value = value;
        } else {
            pt_key_value_t* new_node = (pt_key_value_t*) calloc(1,sizeof(pt_key_value_t));
            char* new_key = strdup(key);
            new_node->parent.type = PT_KEY_VALUE;
            new_node->key = new_key;
            new_node->value = value;
            HASH_ADD_KEYPTR(hh,real_map->key_values,new_node->key,strlen(new_node->key),new_node);
        }
    }
}

void pt_map_unset(pt_node_t* map, const char* key)
{
    if (map && map->type == PT_MAP) {
        pt_map_t* real_map = (pt_map_t*) map;
        pt_key_value_t* search_result = NULL;
        HASH_FIND(hh,real_map->key_values,key,strlen(key),search_result);
        if (search_result) {
            HASH_DEL(real_map->key_values,search_result);
            pt_free_node(search_result->value);
            free(search_result->key);
            free(search_result);
        }
    }
}

pt_node_t* pt_string_new(const char* str)
{
    pt_str_value_t* new_node = (pt_str_value_t*) calloc(1,sizeof(pt_str_value_t));
    new_node->parent.type = PT_STRING;
    new_node->value = strdup(str);
    return (pt_node_t*) new_node;
}

pt_node_t* pt_array_new()
{
    pt_node_t* new_node = (pt_node_t*) calloc(1,sizeof(pt_array_t));
    new_node->type = PT_ARRAY;
    TAILQ_INIT(&((pt_array_t*) new_node)->head);
    return new_node;
}

char* pt_to_json(pt_node_t* root, int beautify)
{
    const unsigned char * gen_buf = NULL;
    char* json = NULL;
#ifdef HAVE_YAJL_V2
    size_t len = 0;
    yajl_gen g = yajl_gen_alloc(NULL);
    yajl_gen_config(g, yajl_gen_beautify, beautify);
    yajl_gen_config(g, yajl_gen_indent_string, "  ");
#else
    unsigned int len = 0;
    yajl_gen_config conf = { beautify,"  "};
    yajl_gen g = yajl_gen_alloc(&conf, NULL);
#endif

    generate_node_json(root,g);

    yajl_gen_get_buf(g, &gen_buf, &len);

    json = (char*) malloc(len + 1);
    memcpy(json,gen_buf,len);
    json[len] = '\0';

    yajl_gen_free(g);
    return json;
}

pt_node_t* pt_from_json(const char* json)
{
    pt_node_t* root = parse_json(json,strlen(json));
    return root;
}

int pt_map_update(pt_node_t* root, pt_node_t* additions, int append)
{
    //pt_map_t* root_map = (pt_map_t*) root;
    pt_map_t* additions_map = (pt_map_t*) additions;
    pt_key_value_t* key_value = NULL;

    if (!root || !additions || root->type != PT_MAP || additions->type != PT_MAP)
        return 1;

    for(key_value = additions_map->key_values; key_value != NULL; key_value = key_value->hh.next) {
        if (key_value->value) {
            pt_node_t* existing = pt_map_get(root,key_value->key);
            if (!existing) {
                pt_map_set(root,key_value->key,pt_clone(key_value->value));
            } else {
                if (key_value->value->type != existing->type) {
                    return 1;
                }
                switch(key_value->value->type) {
                case PT_MAP:
                    pt_map_update(existing,key_value->value,append);
                    break;
                default:
                    pt_map_set(root,key_value->key,pt_clone(key_value->value));
                    break;
                }
            }
        }
    }
    return 0;
}

pt_node_t* pt_clone(pt_node_t* root)
{
    if (root) {
        switch(root->type) {
        case PT_MAP:
        {
            pt_node_t* clone = pt_map_new();
            pt_map_t* map = (pt_map_t*) root;
            pt_key_value_t* key_value = NULL;
            for(key_value = map->key_values; key_value != NULL; key_value = key_value->hh.next) {
                pt_map_set(clone,key_value->key,pt_clone(key_value->value));
            }
            return (pt_node_t*) clone;
        }
        case PT_ARRAY:
        {
            pt_node_t* clone = pt_array_new();
            pt_array_t* array = (pt_array_t*) root;
            pt_array_elem_t* elem = TAILQ_FIRST(&array->head);
            while(elem) {
                pt_array_push_back(clone,pt_clone(elem->node));
                elem = TAILQ_NEXT(elem,entries);
            }
            return clone;
        }
        case PT_NULL:
            return pt_null_new();
        case PT_BOOLEAN:
            return pt_bool_new(((pt_bool_value_t*) root)->value);
        case PT_INTEGER:
            return pt_integer_new(((pt_int_value_t*) root)->value);
        case PT_DOUBLE:
            return pt_double_new(((pt_double_value_t*) root)->value);
        case PT_STRING:
            return pt_string_new(((pt_str_value_t*) root)->value);
        case PT_KEY_VALUE:
            break;
        }
    }
    return NULL;
}


/* Static Implementation */

/*
* This method wraps basic curl functionality
*/
static pt_response_t* http_operation(const char* http_method, const char* server_target, const char* data, unsigned data_len)
{
    CURL *curl_handle;
    CURLcode ret;
    pt_response_t* res;
    struct memory_chunk send_chunk = {0,0,0};
    struct memory_chunk recv_chunk;

    recv_chunk.memory=NULL; /* we expect realloc(NULL, size) to work */
    recv_chunk.size = 0;    /* no data at this point */

    /* init the curl session */
    curl_handle = curl_easy_init();
    if (curl_handle) {
        struct curl_slist *slist;

        if (!strcmp("PATCH", http_method))
            slist = curl_slist_append(NULL, "Content-Type: application/json-patch+json; charset=utf-8");
        else
            slist = curl_slist_append(NULL, "Content-Type: application/json; charset=utf-8");

        if (slist) {
    /* specify URL to get */
    curl_easy_setopt(curl_handle, CURLOPT_URL, server_target);

    curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT, 10);

    // Want to avoid CURL SIGNALS
    curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 1);

    printf("%s : %s\n",http_method,server_target);

            curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, slist);
            if (!strcmp("PUT", http_method)) {
        curl_easy_setopt(curl_handle, CURLOPT_UPLOAD, 1);
            } else {
                if ((!strcmp("POST", http_method)) || (!strcmp("PATCH", http_method))) {
                    curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, data);
                }
        curl_easy_setopt(curl_handle, CURLOPT_CUSTOMREQUEST, http_method);
            }

    if (data && data_len > 0) {
        send_chunk.memory = (char*) malloc(data_len);
        memcpy(send_chunk.memory,data,data_len);
        send_chunk.offset = send_chunk.memory;
        send_chunk.size = data_len;
        curl_easy_setopt(curl_handle, CURLOPT_READFUNCTION, send_memory_callback);
        curl_easy_setopt(curl_handle, CURLOPT_READDATA, (void*) &send_chunk);
    }

    /* send all data to this function  */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, recv_memory_callback);

    /* we pass our 'chunk' struct to the callback function */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&recv_chunk);

    /* some servers don't like requests that are made without a user-agent
    field, so we provide one */
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "pillowtalk-agent/0.1");

    /* get it! */
    ret = curl_easy_perform(curl_handle);

    res = calloc(1,sizeof(pt_response_t));
    if ((!ret)) {
        ret = curl_easy_getinfo(curl_handle,CURLINFO_RESPONSE_CODE, &res->response_code);
        if (ret != CURLE_OK)
            res->response_code = 500;

        if (recv_chunk.size > 0) {
            // Parse the JSON chunk returned
            recv_chunk.memory[recv_chunk.size] = '\0';
            res->raw_json = recv_chunk.memory;
            res->raw_json_len = recv_chunk.size;
        }
    } else {
        res->response_code = 500;
    }

    if (send_chunk.memory)
        free(send_chunk.memory);
            curl_slist_free_all(slist);
        }

    /* cleanup curl stuff */
    curl_easy_cleanup(curl_handle);
    }
    return res;
}

static void *myrealloc(void *ptr, size_t size)
{
    /* There might be a realloc() out there that doesn't like reallocing
    NULL pointers, so we take care of it here */
    if(ptr)
        return (void*) realloc(ptr, size);
    else
        return (void*) malloc(size);
}

static size_t recv_memory_callback(void *ptr, size_t size, size_t nmemb, void *data)
{
    size_t realsize = size * nmemb;
    struct memory_chunk *mem = (struct memory_chunk*)data;

    mem->memory = (char*) myrealloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory) {
        memcpy(&(mem->memory[mem->size]), ptr, realsize);
        mem->size += realsize;
        mem->memory[mem->size] = 0;
    }
    return realsize;
}

static size_t send_memory_callback(void *ptr, size_t size, size_t nmemb, void *data)
{
    struct memory_chunk* mem = (struct memory_chunk*) data;
    size_t realsize = size * nmemb;
    if (realsize < 1)
        return 0;

    if (mem->size > 0) {
        size_t bytes_to_copy = (mem->size > realsize) ? realsize : mem->size;
        memcpy(ptr,mem->offset,bytes_to_copy);
        mem->offset += bytes_to_copy;
        mem->size -= bytes_to_copy;
        return bytes_to_copy;
    }
    return 0;
}

/* Yajl Callbacks */
static int json_null(void* ctx)
{
    pt_node_t* node = (pt_node_t*) malloc(sizeof(pt_node_t));
    node->type = PT_NULL;
    add_node_to_context_container((pt_parser_ctx_t*) ctx,node);
    return 1;
}

static int json_boolean(void* ctx,int boolean)
{
    pt_bool_value_t * node = (pt_bool_value_t*) calloc(1,sizeof(pt_bool_value_t));
    node->parent.type = PT_BOOLEAN;
    node->value = boolean;
    add_node_to_context_container((pt_parser_ctx_t*) ctx,(pt_node_t*)node);
    return 1;
}

#ifdef HAVE_YAJL_V2
static int json_map_key(void* ctx, const unsigned char* str, size_t length)
#else
static int json_map_key(void* ctx, const unsigned char* str, unsigned int length)
#endif
{
    pt_parser_ctx_t* parser_ctx= (pt_parser_ctx_t*) ctx;
    assert(parser_ctx->stack && parser_ctx->stack->container->type == PT_MAP);
    {
        pt_map_t* container = (pt_map_t*) parser_ctx->stack->container;
        pt_key_value_t* new_node = (pt_key_value_t*) calloc(1,sizeof(pt_key_value_t));
        char* new_str = (char*) malloc(length + 1);
        memcpy(new_str,str,length);
        new_str[length] = 0x0;
        new_node->key = new_str;
        new_node->parent.type = PT_KEY_VALUE;
        HASH_ADD_KEYPTR(hh,container->key_values,new_node->key,length,new_node);
        parser_ctx->stack->cur = (pt_node_t*) new_node;
    }
    return 1;
}

#ifdef HAVE_YAJL_V2
static int json_integer(void* ctx,long long integer)
#else
static int json_integer(void* ctx,long integer)
#endif
{
    pt_int_value_t* node = (pt_int_value_t*) calloc(1,sizeof(pt_int_value_t));
    node->parent.type = PT_INTEGER;
    node->value = integer;

    add_node_to_context_container(ctx,(pt_node_t*) node);
    return 1;
}

static int json_double(void* ctx,double dbl)
{
    pt_double_value_t* node = (pt_double_value_t*) calloc(1,sizeof(pt_double_value_t));
    node->parent.type = PT_DOUBLE;
    node->value = dbl;

    add_node_to_context_container(ctx,(pt_node_t*) node);
    return 1;
}

#ifdef HAVE_YAJL_V2
static int json_string(void* ctx, const unsigned char* str, size_t length)
#else
static int json_string(void* ctx, const unsigned char* str, unsigned int length)
#endif
{
    pt_str_value_t* node;
    char* new_str = (char*) malloc(length + 1);
    memcpy(new_str,str,length);
    new_str[length] = 0x0;
    
    node = (pt_str_value_t*) calloc(1,sizeof(pt_str_value_t));
    node->parent.type = PT_STRING;
    node->value = new_str;
    add_node_to_context_container(ctx,(pt_node_t*) node);
    return 1;
}

/* If we aren't in a key value pair then we create a new node, otherwise we are
* the value 
*/
static int json_start_map(void* ctx)
{
    pt_container_ctx_t* new_ctx;
    pt_parser_ctx_t* parser_ctx = (pt_parser_ctx_t*) ctx;
    pt_node_t* new_node = (pt_node_t*) calloc(1,sizeof(pt_map_t));
    new_node->type = PT_MAP;
    add_node_to_context_container(parser_ctx,new_node);
    
    new_ctx = (pt_container_ctx_t*) calloc(1,sizeof(pt_container_ctx_t));
    new_ctx->container = new_node;
    LL_PREPEND(parser_ctx->stack,new_ctx);
    return 1;
}

static int json_end_map(void* ctx)
{
    pt_parser_ctx_t* parser_ctx = (pt_parser_ctx_t*) ctx;
    assert(parser_ctx->stack->container->type == PT_MAP);
    if (parser_ctx->stack) {
        pt_container_ctx_t* old_head = parser_ctx->stack;
        LL_DELETE(parser_ctx->stack,old_head);
        free(old_head);
    }
    return 1;
}

static int json_start_array(void* ctx)
{
    pt_container_ctx_t* new_ctx;
    pt_parser_ctx_t* parser_ctx = (pt_parser_ctx_t*) ctx;
    pt_array_t* new_node = (pt_array_t*) calloc(1,sizeof(pt_array_t));
    TAILQ_INIT(&new_node->head);
    new_node->parent.type = PT_ARRAY;
    add_node_to_context_container(parser_ctx,(pt_node_t*) new_node);

    new_ctx = (pt_container_ctx_t*) calloc(1,sizeof(pt_container_ctx_t));
    new_ctx->container = (pt_node_t*) new_node;
    new_ctx->cur = (pt_node_t*) new_node;
    LL_PREPEND(parser_ctx->stack,new_ctx);
    return 1;
}

static int json_end_array(void* ctx)
{
    pt_parser_ctx_t* parser_ctx = (pt_parser_ctx_t*) ctx;
    assert(parser_ctx->stack->container->type == PT_ARRAY);
    if (parser_ctx->stack) {
        pt_container_ctx_t* old_head = parser_ctx->stack;
        LL_DELETE(parser_ctx->stack,old_head);
        free(old_head);
    }
    return 1;
}

/*
* This function looks to see what the current node and adds the new value node to it.
* If it is an array it appends the value to the array.
* If it is a key value pair it adds it to the value field of that pair.
*/
static void add_node_to_context_container(pt_parser_ctx_t* context, pt_node_t* value)
{
    if (context->stack && context->stack->cur) {
        pt_node_t* cur = context->stack->cur;
        if (cur->type == PT_ARRAY) {
            pt_array_t* resolved = (pt_array_t*) cur;
            pt_array_elem_t* elem = (pt_array_elem_t*) malloc(sizeof(pt_array_elem_t));
            elem->node = value;
            TAILQ_INSERT_TAIL(&resolved->head,elem,entries);
            resolved->len++;
        } else if (cur->type == PT_KEY_VALUE) {
            pt_key_value_t* resolved = (pt_key_value_t*) cur;
            resolved->value = value;
        } else {
            printf("Shouldn't get here: %d:%d\n", cur->type, value->type);
        }
    } else {
        context->root = value;
    }
}

static pt_node_t* parse_json(const char* json, int json_len)
{
    yajl_status stat;
    yajl_handle hand;
#ifndef HAVE_YAJL_V2
    yajl_parser_config cfg = { 0, 1 };
#endif
    pt_node_t* root;
    pt_parser_ctx_t* parser_ctx = (pt_parser_ctx_t*) calloc(1,sizeof(pt_parser_ctx_t));

#ifdef HAVE_YAJL_V2
    hand = yajl_alloc(&callbacks, NULL, parser_ctx);
    // don't allow comments
    yajl_config(hand, yajl_allow_comments, 0);
    // DO validate strings
    yajl_config(hand, yajl_dont_validate_strings, 0);
#else
    hand = yajl_alloc(&callbacks, &cfg, NULL, parser_ctx);
#endif

    stat = yajl_parse(hand, (const unsigned char*) json, json_len);
#ifdef HAVE_YAJL_V2
    if (stat != yajl_status_ok && stat != yajl_status_client_canceled) {
#else
    if (stat != yajl_status_ok && stat != yajl_status_insufficient_data) {
#endif
        unsigned char * str = yajl_get_error(hand, 1, (const unsigned char*) json, json_len);
        fprintf(stderr, "%s",(const char *) str);
        yajl_free_error(hand, str);
    }

    root = parser_ctx->root;
    free_parser_ctx(parser_ctx);
    yajl_free(hand);
    return root;
}

static void free_map_node(pt_map_t* map)
{
    pt_key_value_t* cur = NULL;
    while(map->key_values) {
        cur = map->key_values;
        HASH_DEL(map->key_values, cur);
        pt_free_node(cur->value);
        free(cur->key);
        free(cur);
    }
}

static void free_array_node(pt_array_t* array)
{
    while (!TAILQ_EMPTY(&array->head)) {
        pt_array_elem_t* elem;
        elem = TAILQ_FIRST(&array->head);
        TAILQ_REMOVE(&array->head, elem, entries);
        pt_free_node(elem->node);
        free(elem);
    }
}

/* Recursive Free Function.  Watch the fireworks! */
void pt_free_node(pt_node_t* node)
{
    if (node) {
        switch(node->type) {
case PT_MAP:
    {
        free_map_node((pt_map_t*) node);
    }
    break;
case PT_ARRAY:
    {
        free_array_node((pt_array_t*) node);
    }
    break;
case PT_STRING:
    free(((pt_str_value_t*) node)->value);
    break;
default:
    break;
    // the basic value types will get handled in the free(node) below
        }
        free(node);
    }
}

static void generate_map_json(pt_map_t* map, yajl_gen g)
{
    pt_key_value_t* cur = NULL;

    yajl_gen_map_open(g);
    for(cur=map->key_values; cur != NULL; cur=cur->hh.next) {
        yajl_gen_string(g,(const unsigned char*) cur->key,strlen(cur->key));
        generate_node_json(cur->value,g);
    }
    yajl_gen_map_close(g);
}

static void generate_array_json(pt_array_t* array, yajl_gen g)
{
    pt_array_t* real_array = (pt_array_t*) array;
    pt_array_elem_t* cur = NULL;
    yajl_gen_array_open(g);
    TAILQ_FOREACH(cur,&real_array->head, entries) {
        generate_node_json(cur->node,g);
    }
    yajl_gen_array_close(g);
}

static void generate_node_json(pt_node_t* node, yajl_gen g)
{
    if (node) {
        switch(node->type) {
        case PT_ARRAY:
            generate_array_json((pt_array_t*) node,g);
            break;

        case PT_MAP:
            generate_map_json((pt_map_t*) node,g);
            break;

        case PT_NULL:
            yajl_gen_null(g);
            break;

        case PT_BOOLEAN:
            yajl_gen_bool(g,((pt_bool_value_t*) node)->value);
            break;

        case PT_INTEGER:
            yajl_gen_integer(g,((pt_int_value_t*) node)->value);
            break;

        case PT_DOUBLE:
            yajl_gen_double(g,((pt_double_value_t*) node)->value);
            break;

        case PT_STRING:
            yajl_gen_string(g,(const unsigned char*) ((pt_str_value_t*) node)->value, strlen(((pt_str_value_t*) node)->value));
            break;

        case PT_KEY_VALUE:
            break;
        }
    } else {
        yajl_gen_null(g);
    }
}

static pt_response_t *put_patch_or_post_operation(const char *method, const char *server_target, pt_node_t *doc)
{
    pt_response_t *res;
    char          *data    = NULL;
    int           data_len = 0;
    if (doc) {
        data = pt_to_json(doc, 0);
        if (data)
            data_len = strlen(data);
    }
    res       = http_operation(method, server_target, data, data_len);
    res->root = parse_json(res->raw_json, res->raw_json_len);
    if (data)
        free(data);
    return res;
}

static pt_response_t *put_patch_or_post_raw_operation(const char *method, const char *server_target, const char *data, unsigned int data_len)
{
    pt_response_t *res = http_operation(method, server_target, data, data_len);
    res->root = parse_json(res->raw_json, res->raw_json_len);
    return res;
}