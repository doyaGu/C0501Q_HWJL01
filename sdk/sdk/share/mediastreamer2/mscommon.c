/*
mediastreamer2 library - modular sound and video processing and streaming
Copyright (C) 2006  Simon MORLAT (simon.morlat@linphone.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/



#include "mediastreamer2/mscommon.h"
#include "mediastreamer2/mscodecutils.h"
#include "mediastreamer2/msfilter.h"

#if !defined(_WIN32_WCE)
#include <sys/types.h>
#endif
#if 1 //ndef WIN32
#include <dirent.h>
#else
#ifndef PACKAGE_PLUGINS_DIR
#if (defined(WIN32) || defined(_WIN32_WCE))
#define PACKAGE_PLUGINS_DIR "lib\\mediastreamer\\plugins\\"
#else
#define PACKAGE_PLUGINS_DIR "."
#endif
#endif
#endif
#ifdef HAVE_DLOPEN
#include <dlfcn.h>
#endif

#ifdef __APPLE__
   #include "TargetConditionals.h"
#endif

#ifdef __QNX__
#include <sys/syspage.h>
#endif

unsigned int ms_get_cpu_count() {
    return ms_factory_get_cpu_count(ms_factory_get_fallback());
}

void ms_set_cpu_count(unsigned int c) {
    ms_factory_set_cpu_count(ms_factory_get_fallback(),c);
}

MSList *ms_list_new(void *data){
    MSList *new_elem=(MSList *)ms_new0(MSList,1);
    new_elem->data=data;
    return new_elem;
}

MSList *ms_list_append_link(MSList *elem, MSList *new_elem){
    MSList *it=elem;
    if (elem==NULL) return new_elem;
    while (it->next!=NULL) it=ms_list_next(it);
    it->next=new_elem;
    new_elem->prev=it;
    return elem;
}

MSList * ms_list_append(MSList *elem, void * data){
    MSList *new_elem=ms_list_new(data);
    return ms_list_append_link(elem,new_elem);
}

MSList * ms_list_prepend(MSList *elem, void *data){
    MSList *new_elem=ms_list_new(data);
    if (elem!=NULL) {
        new_elem->next=elem;
        elem->prev=new_elem;
    }
    return new_elem;
}


MSList * ms_list_concat(MSList *first, MSList *second){
    MSList *it=first;
    if (it==NULL) return second;
    while(it->next!=NULL) it=ms_list_next(it);
    it->next=second;
    second->prev=it;
    return first;
}

MSList * ms_list_free_with_data(MSList *list, void (*freefunc)(void*)){
    MSList *elem;
    MSList *tmp;

    for (elem=list;elem!=NULL;){
        tmp=elem->next;
        if (freefunc) freefunc(elem->data);
        ms_free(elem);
        elem=tmp;
    }
    return NULL;
}

MSList * ms_list_free(MSList *elem){
    return ms_list_free_with_data(elem,NULL);
}

MSList * ms_list_remove(MSList *first, void *data){
    MSList *it;
    it=ms_list_find(first,data);
    if (it) return ms_list_remove_link(first,it);
    else {
        ms_warning("ms_list_remove: no element with %p data was in the list", data);
        return first;
    }
}

MSList * ms_list_remove_custom(MSList *first, MSCompareFunc compare_func, const void *user_data) {
    MSList *cur;
    MSList *elem = first;
    while (elem != NULL) {
        cur = elem;
        elem = elem->next;
        if (compare_func(cur->data, user_data) == 0) {
            first = ms_list_remove(first, cur->data);
        }
    }
    return first;
}

int ms_list_size(const MSList *first){
    int n=0;
    while(first!=NULL){
        ++n;
        first=first->next;
    }
    return n;
}

void ms_list_for_each(const MSList *list, void (*func)(void *)){
    for(;list!=NULL;list=list->next){
        func(list->data);
    }
}

void ms_list_for_each2(const MSList *list, void (*func)(void *, void *), void *user_data){
    for(;list!=NULL;list=list->next){
        func(list->data,user_data);
    }
}

MSList *ms_list_remove_link(MSList *list, MSList *elem){
    MSList *ret;
    if (elem==list){
        ret=elem->next;
        //[iclai+] useless code
        //elem->prev=NULL;
        //elem->next=NULL;
        //[iclai-]
        if (ret!=NULL) ret->prev=NULL;
        ms_free(elem);
        return ret;
    }
    elem->prev->next=elem->next;
    if (elem->next!=NULL) elem->next->prev=elem->prev;
    //[iclai+] useless code
    //elem->next=NULL;
    //elem->prev=NULL;
    //[iclai-]
    ms_free(elem);
    return list;
}

MSList *ms_list_find(MSList *list, void *data){
    for(;list!=NULL;list=list->next){
        if (list->data==data) return list;
    }
    return NULL;
}

MSList *ms_list_find_custom(MSList *list, int (*compare_func)(const void *, const void*), const void *user_data){
    for(;list!=NULL;list=list->next){
        if (compare_func(list->data,user_data)==0) return list;
    }
    return NULL;
}

void * ms_list_nth_data(const MSList *list, int index){
    int i;
    for(i=0;list!=NULL;list=list->next,++i){
        if (i==index) return list->data;
    }
    ms_error("ms_list_nth_data: no such index in list.");
    return NULL;
}

int ms_list_position(const MSList *list, MSList *elem){
    int i;
    for(i=0;list!=NULL;list=list->next,++i){
        if (elem==list) return i;
    }
    ms_error("ms_list_position: no such element in list.");
    return -1;
}

int ms_list_index(const MSList *list, void *data){
    int i;
    for(i=0;list!=NULL;list=list->next,++i){
        if (data==list->data) return i;
    }
    ms_error("ms_list_index: no such element in list.");
    return -1;
}

MSList *ms_list_insert_sorted(MSList *list, void *data, int (*compare_func)(const void *, const void*)){
    MSList *it,*previt=NULL;
    MSList *nelem;
    MSList *ret=list;
    if (list==NULL) return ms_list_append(list,data);
    else{
        nelem=ms_list_new(data);
        for(it=list;it!=NULL;it=it->next){
            previt=it;
            if (compare_func(data,it->data)<=0){
                nelem->prev=it->prev;
                nelem->next=it;
                if (it->prev!=NULL)
                    it->prev->next=nelem;
                else{
                    ret=nelem;
                }
                it->prev=nelem;
                return ret;
            }
        }
        previt->next=nelem;
        nelem->prev=previt;
    }
    return ret;
}

MSList *ms_list_insert(MSList *list, MSList *before, void *data){
    MSList *elem;
    if (list==NULL || before==NULL) return ms_list_append(list,data);
    for(elem=list;elem!=NULL;elem=ms_list_next(elem)){
        if (elem==before){
            if (elem->prev==NULL)
                return ms_list_prepend(list,data);
            else{
                MSList *nelem=ms_list_new(data);
                nelem->prev=elem->prev;
                nelem->next=elem;
                elem->prev->next=nelem;
                elem->prev=nelem;
            }
        }
    }
    return list;
}

MSList *ms_list_copy(const MSList *list){
    MSList *copy=NULL;
    const MSList *iter;
    for(iter=list;iter!=NULL;iter=ms_list_next(iter)){
        copy=ms_list_append(copy,iter->data);
    }
    return copy;
}

int ms_load_plugins(const char *dir){
    return ms_factory_load_plugins(ms_factory_get_fallback(),dir);
}

static int ms_base_ref=0;
static int ms_plugins_ref=0;

void ms_base_init(){
    ms_base_ref++;
    if ( ms_base_ref>1 ) {
        ms_message ("Skiping ms_base_init, because [%i] ref",ms_base_ref);
        return;
    }
    ms_factory_create_fallback();
    //[iclai+] useless code
    //ms_factory_get_fallback();
    //[iclai-]
}

void ms_base_exit(){
    --ms_base_ref;
    if ( ms_base_ref>0 ) {
        ms_message ("Skiping ms_base_exit, still [%i] ref",ms_base_ref);
        return;
    }
    ms_factory_destroy(ms_factory_get_fallback());
}

//[iclai+] we don't support plugins
#if 0
void ms_plugins_init(void) {
    ms_plugins_ref++;
    if ( ms_plugins_ref>1 ) {
        ms_message ("Skiping ms_plugins_init, because [%i] ref",ms_plugins_ref);
        return;
    }
    ms_factory_init_plugins(ms_factory_get_fallback());
}

void ms_plugins_exit(void) {
    --ms_plugins_ref;
    if ( ms_plugins_ref>0 ) {
        ms_message ("Skiping ms_plugins_exit, still [%i] ref",ms_plugins_ref);
        return;
    }
    ms_factory_uninit_plugins(ms_factory_get_fallback());
}

void ms_set_plugins_dir(const char *path) {
    ms_factory_set_plugins_dir(ms_factory_get_fallback(),path);
}
#endif
//[iclai-]

void ms_sleep(int seconds){
#if 0 //def WIN32
    Sleep(seconds*1000);
#else
    sleep(seconds);
    //struct timespec ts,rem;
    //int err;
    //ts.tv_sec=seconds;
    //ts.tv_nsec=0;
    //do {
    //  err=nanosleep(&ts,&rem);
    //  ts=rem;
    //}while(err==-1 && errno==EINTR);
#endif
}

void ms_usleep(uint64_t usec){
#if 0 //def WIN32
    Sleep((DWORD)(usec/1000));
#else
    usleep(usec);
    //struct timespec ts,rem;
    //int err;
    //ts.tv_sec=usec/1000000LL;
    //ts.tv_nsec=(usec%1000000LL)*1000;
    //do {
    //  err=nanosleep(&ts,&rem);
    //  ts=rem;
    //}while(err==-1 && errno==EINTR);
#endif
}

int ms_get_payload_max_size(){
    return ms_factory_get_payload_max_size(ms_factory_get_fallback());
}

void ms_set_payload_max_size(int size){
    ms_factory_set_payload_max_size(ms_factory_get_fallback(),size);
}

extern void _android_key_cleanup(void*);

void ms_thread_exit(void* ref_val) {
#ifdef ANDROID
    // due to a bug in old Bionic version
    // cleanup of jni manually
    // works directly with Android 2.2
    _android_key_cleanup(NULL);
#endif
#if !defined(__linux) || defined(ANDROID)
    ortp_thread_exit(ref_val); // pthread_exit futex issue: http://lkml.indiana.edu/hypermail/linux/kernel/0902.0/00153.html
#endif
}
