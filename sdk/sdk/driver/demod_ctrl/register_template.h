#ifndef __REGISTER_TEMPLETE_H_UC0LRYGY_8FQW_E480_94WK_SZP8KCJGWMTC__
#define __REGISTER_TEMPLETE_H_UC0LRYGY_8FQW_E480_94WK_SZP8KCJGWMTC__

#ifdef __cplusplus
extern "C" {
#endif


#define DEFINE_REGISTER_TEMPLATE(descType, id_enum_type)     \
    static descType *first_##descType##_item = 0; \
    static void                                   \
    _register_##descType##_item(                  \
        descType     *format){                    \
        descType **p;                             \
        p = &first_##descType##_item;             \
        while (*p != 0) p = &(*p)->next;          \
        *p = format;                              \
        format->next = 0;                         \
    }                                             \
    static descType*                              \
    descType##_next(                              \
        descType  *f){                            \
        if(f) return f->next;                     \
        else  return first_##descType##_item;     \
    }                                             \
    static descType*                              \
    descType##_find(                              \
        id_enum_type   id){                       \
        descType *dev = 0;                        \
        while(dev = descType##_next(dev))         \
        if( dev->id == id ) return dev;           \
        return 0;                                 \
    }

#define REGISTER_ITEM(type, X, x) { \
    extern type type##_##x##_desc; \
    if(CONFIG_##type##_##X##_DESC)  _register_##type##_item(&type##_##x##_desc); }

#define FIND_DESC_ITEM(descType, id)    descType##_find(id)


#ifdef __cplusplus
}
#endif

#endif
