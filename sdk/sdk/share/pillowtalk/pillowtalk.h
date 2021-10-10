//
// Copyright (c) 2009, Curtis Spencer. All rights reserved.
//
#ifndef __PILLOWTALK__H_
#define __PILLOWTALK__H_

#ifdef	__cplusplus
extern "C" {
#endif

typedef enum {PT_MAP,PT_ARRAY,PT_NULL, PT_BOOLEAN, PT_INTEGER, PT_DOUBLE, PT_STRING, PT_KEY_VALUE} pt_type_t;

typedef struct {
  pt_type_t type;
} pt_node_t;

typedef struct {
  pt_node_t* root;
  long response_code;
  char* raw_json;
  int raw_json_len;
} pt_response_t;

// Opaque type for iterator
typedef struct {
    void* opaque;
} pt_iterator_t;

/*
 * NAME pt_init()
 *
 * DESCRIPTION
 *
 * pt_init() should be invoked exactly once for each application that
 * uses pillowtalk and before any call of other pillowtalk functions.
 *
 * This function is not thread-safe!
 */
void pt_init();

/*
 * NAME pt_cleanup()
 *
 * DESCRIPTION
 *
 * pt_cleanup() should be invoked exactly once for each application
 * that uses pillowtalk
 */
void pt_cleanup();

void pt_free_node(pt_node_t* node);
void pt_free_response(pt_response_t* res);

/***** HTTP Related Functions ******/

pt_response_t* pt_delete(const char* server_target);

pt_response_t* pt_put(const char* server_target, pt_node_t* document);
pt_response_t* pt_put_raw(const char* server_target, const char* data, unsigned int data_len);
pt_response_t* pt_post(const char* server_target, pt_node_t* document);
pt_response_t* pt_post_raw(const char* server_target, const char* data, unsigned int data_len);
pt_response_t* pt_patch(const char* server_target, pt_node_t* document);
pt_response_t* pt_patch_raw(const char* server_target, const char* data, unsigned int data_len);

/* 
 * Do an HTTP get request on the target and parse the resulting JSON into the
 * pt_response object
 */
pt_response_t* pt_get(const char* server_target);


/*
 * This will just do a get against the server target and not try to parse it at all.
 * It is useful for doing your own parsing with the resultant JSON 
 */
pt_response_t* pt_unparsed_get(const char* server_target);

/***** Node Related Functions ******/

/*
 * Once you have a node, you can call various functions on it, and most will
 * return NULL if you do it on the wrong type.  Check the type attribute of the
 * pt_node_t to ensure you are doing the correct operation.
 */
pt_node_t* pt_map_get(pt_node_t* map,const char* key);

unsigned int pt_array_len(pt_node_t* array);
pt_node_t* pt_array_get(pt_node_t* array, unsigned int idx);

int pt_is_null(pt_node_t* null);
int pt_boolean_get(pt_node_t* boolean);
int pt_integer_get(pt_node_t* integer);
double pt_double_get(pt_node_t* dbl);
const char* pt_string_get(pt_node_t* string);

/* 
 * The following functions are used to change a pt_node_t to do update
 * operations or get new json strings 
 */

void pt_map_set(pt_node_t* map, const char* key, pt_node_t* value);
void pt_map_unset(pt_node_t* map, const char* key);

pt_node_t* pt_null_new();
pt_node_t* pt_bool_new(int boolean);
pt_node_t* pt_integer_new(int integer);
pt_node_t* pt_double_new(double dbl);
pt_node_t* pt_string_new(const char* str);
pt_node_t* pt_map_new();
pt_node_t* pt_array_new();
pt_node_t* pt_patch_new(const char* op, const char* path, pt_node_t* value);

void pt_array_push_back(pt_node_t* array, pt_node_t* elem);
void pt_array_push_front(pt_node_t* array, pt_node_t* elem);

/*
 * This will remove elem if it exists in the array and free it as well, so
 * don't use elem after this
 */
void pt_array_remove(pt_node_t* array, pt_node_t* elem);

/* 
 * Build an iterator from an array/map node.  If you pass in an unsupported
 * node it will return NULL
 */
pt_iterator_t* pt_iterator(pt_node_t* node);

/*
 * This returns the next node in the iterator back and NULL when complete.
 *
 * The key char** is also set to the key of a key value pair if you are
 * iterating through a map
 *
 */
pt_node_t* pt_iterator_next(pt_iterator_t* iter, const char** key);


/*
 * Convert a pt_node_t structure into a raw json string
 */
char* pt_to_json(pt_node_t* root, int beautify);

/* 
 * Take a raw json string and turn it into a pillowtalk structure
 */
pt_node_t* pt_from_json(const char* json);

/*
 * Merge additions into an existing pt_node
 *
 * For example if your root looks like this
 * {
 *    "name" : "Curtis",
 *    "favorite_food" : "Bread"
 * }
 *
 * and your additions look like this
 *
 * {
 *    "favorite_game" : "Street Fighter II"
 * }
 *
 * then the resulting json in root would be
 *
 * {
 *    "name" : "Curtis",
 *    "favorite_food" : "Bread",
 *    "favorite_game" : "Street Fighter II"
 * }
 *
 * @return a nonzero error code if something cannot properly be merged.  For
 * example, if a key in the root is an array and the additions has it as a hash
 * then it will give up there, but it won't rollback so be careful.
 */    
int pt_map_update(pt_node_t* root, pt_node_t* additions,int append);

/*
 * This method is useful if you want to clone a root you are working on to make
 * changes to it
 */
pt_node_t* pt_clone(pt_node_t* root);


#ifdef	__cplusplus
}
#endif

#endif // _PILLOWTALK_H_
