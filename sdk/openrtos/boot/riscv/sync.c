/* sync.c */

typedef char            _Sync_word_1;
typedef short           _Sync_word_2;
typedef int             _Sync_word_4;
typedef long long       _Sync_word_8;
typedef int             bool;

_Sync_word_1 __sync_add_and_fetch_1 (_Sync_word_1 *ptr, _Sync_word_1 value) { *ptr =  *ptr + value; return *ptr; }
_Sync_word_2 __sync_add_and_fetch_2 (_Sync_word_2 *ptr, _Sync_word_2 value) { *ptr =  *ptr + value; return *ptr; }
_Sync_word_4 __sync_add_and_fetch_4 (_Sync_word_4 *ptr, _Sync_word_4 value) { *ptr =  *ptr + value; return *ptr; }
_Sync_word_8 __sync_add_and_fetch_8 (_Sync_word_8 *ptr, _Sync_word_8 value) { *ptr =  *ptr + value; return *ptr; }
_Sync_word_1 __sync_and_and_fetch_1 (_Sync_word_1 *ptr, _Sync_word_1 value) { *ptr =  *ptr & value; return *ptr; }
_Sync_word_2 __sync_and_and_fetch_2 (_Sync_word_2 *ptr, _Sync_word_2 value) { *ptr =  *ptr & value; return *ptr; }
_Sync_word_4 __sync_and_and_fetch_4 (_Sync_word_4 *ptr, _Sync_word_4 value) { *ptr =  *ptr & value; return *ptr; }
_Sync_word_8 __sync_and_and_fetch_8 (_Sync_word_8 *ptr, _Sync_word_8 value) { *ptr =  *ptr & value; return *ptr; }
_Sync_word_1 __sync_nand_and_fetch_1(_Sync_word_1 *ptr, _Sync_word_1 value) { *ptr = ~*ptr & value; return *ptr; }
_Sync_word_2 __sync_nand_and_fetch_2(_Sync_word_2 *ptr, _Sync_word_2 value) { *ptr = ~*ptr & value; return *ptr; }
_Sync_word_4 __sync_nand_and_fetch_4(_Sync_word_4 *ptr, _Sync_word_4 value) { *ptr = ~*ptr & value; return *ptr; }
_Sync_word_8 __sync_nand_and_fetch_8(_Sync_word_8 *ptr, _Sync_word_8 value) { *ptr = ~*ptr & value; return *ptr; }
_Sync_word_1 __sync_or_and_fetch_1  (_Sync_word_1 *ptr, _Sync_word_1 value) { *ptr =  *ptr | value; return *ptr; }
_Sync_word_2 __sync_or_and_fetch_2  (_Sync_word_2 *ptr, _Sync_word_2 value) { *ptr =  *ptr | value; return *ptr; }
_Sync_word_4 __sync_or_and_fetch_4  (_Sync_word_4 *ptr, _Sync_word_4 value) { *ptr =  *ptr | value; return *ptr; }
_Sync_word_8 __sync_or_and_fetch_8  (_Sync_word_8 *ptr, _Sync_word_8 value) { *ptr =  *ptr | value; return *ptr; }
_Sync_word_1 __sync_sub_and_fetch_1 (_Sync_word_1 *ptr, _Sync_word_1 value) { *ptr =  *ptr - value; return *ptr; }
_Sync_word_2 __sync_sub_and_fetch_2 (_Sync_word_2 *ptr, _Sync_word_2 value) { *ptr =  *ptr - value; return *ptr; }
_Sync_word_4 __sync_sub_and_fetch_4 (_Sync_word_4 *ptr, _Sync_word_4 value) { *ptr =  *ptr - value; return *ptr; }
_Sync_word_8 __sync_sub_and_fetch_8 (_Sync_word_8 *ptr, _Sync_word_8 value) { *ptr =  *ptr - value; return *ptr; }
_Sync_word_1 __sync_xor_and_fetch_1 (_Sync_word_1 *ptr, _Sync_word_1 value) { *ptr =  *ptr ^ value; return *ptr; }
_Sync_word_2 __sync_xor_and_fetch_2 (_Sync_word_2 *ptr, _Sync_word_2 value) { *ptr =  *ptr ^ value; return *ptr; }
_Sync_word_4 __sync_xor_and_fetch_4 (_Sync_word_4 *ptr, _Sync_word_4 value) { *ptr =  *ptr ^ value; return *ptr; }
_Sync_word_8 __sync_xor_and_fetch_8 (_Sync_word_8 *ptr, _Sync_word_8 value) { *ptr =  *ptr ^ value; return *ptr; }

_Sync_word_1 __sync_fetch_and_add_1 (_Sync_word_1 *ptr, _Sync_word_1 value) { _Sync_word_1 tmp = *ptr; *ptr =  tmp + value; return tmp; }
_Sync_word_2 __sync_fetch_and_add_2 (_Sync_word_2 *ptr, _Sync_word_2 value) { _Sync_word_2 tmp = *ptr; *ptr =  tmp + value; return tmp; }
_Sync_word_4 __sync_fetch_and_add_4 (_Sync_word_4 *ptr, _Sync_word_4 value) { _Sync_word_4 tmp = *ptr; *ptr =  tmp + value; return tmp; }
_Sync_word_8 __sync_fetch_and_add_8 (_Sync_word_8 *ptr, _Sync_word_8 value) { _Sync_word_8 tmp = *ptr; *ptr =  tmp + value; return tmp; }
_Sync_word_1 __sync_fetch_and_and_1 (_Sync_word_1 *ptr, _Sync_word_1 value) { _Sync_word_1 tmp = *ptr; *ptr =  tmp & value; return tmp; }
_Sync_word_2 __sync_fetch_and_and_2 (_Sync_word_2 *ptr, _Sync_word_2 value) { _Sync_word_2 tmp = *ptr; *ptr =  tmp & value; return tmp; }
_Sync_word_4 __sync_fetch_and_and_4 (_Sync_word_4 *ptr, _Sync_word_4 value) { _Sync_word_4 tmp = *ptr; *ptr =  tmp & value; return tmp; }
_Sync_word_8 __sync_fetch_and_and_8 (_Sync_word_8 *ptr, _Sync_word_8 value) { _Sync_word_8 tmp = *ptr; *ptr =  tmp & value; return tmp; }
_Sync_word_1 __sync_fetch_and_nand_1(_Sync_word_1 *ptr, _Sync_word_1 value) { _Sync_word_1 tmp = *ptr; *ptr = ~tmp & value; return tmp; }
_Sync_word_2 __sync_fetch_and_nand_2(_Sync_word_2 *ptr, _Sync_word_2 value) { _Sync_word_2 tmp = *ptr; *ptr = ~tmp & value; return tmp; }
_Sync_word_4 __sync_fetch_and_nand_4(_Sync_word_4 *ptr, _Sync_word_4 value) { _Sync_word_4 tmp = *ptr; *ptr = ~tmp & value; return tmp; }
_Sync_word_8 __sync_fetch_and_nand_8(_Sync_word_8 *ptr, _Sync_word_8 value) { _Sync_word_8 tmp = *ptr; *ptr = ~tmp & value; return tmp; }
_Sync_word_1 __sync_fetch_and_or_1  (_Sync_word_1 *ptr, _Sync_word_1 value) { _Sync_word_1 tmp = *ptr; *ptr =  tmp | value; return tmp; }
_Sync_word_2 __sync_fetch_and_or_2  (_Sync_word_2 *ptr, _Sync_word_2 value) { _Sync_word_2 tmp = *ptr; *ptr =  tmp | value; return tmp; }
_Sync_word_4 __sync_fetch_and_or_4  (_Sync_word_4 *ptr, _Sync_word_4 value) { _Sync_word_4 tmp = *ptr; *ptr =  tmp | value; return tmp; }
_Sync_word_8 __sync_fetch_and_or_8  (_Sync_word_8 *ptr, _Sync_word_8 value) { _Sync_word_8 tmp = *ptr; *ptr =  tmp | value; return tmp; }
_Sync_word_1 __sync_fetch_and_sub_1 (_Sync_word_1 *ptr, _Sync_word_1 value) { _Sync_word_1 tmp = *ptr; *ptr =  tmp - value; return tmp; }
_Sync_word_2 __sync_fetch_and_sub_2 (_Sync_word_2 *ptr, _Sync_word_2 value) { _Sync_word_2 tmp = *ptr; *ptr =  tmp - value; return tmp; }
_Sync_word_4 __sync_fetch_and_sub_4 (_Sync_word_4 *ptr, _Sync_word_4 value) { _Sync_word_4 tmp = *ptr; *ptr =  tmp - value; return tmp; }
_Sync_word_8 __sync_fetch_and_sub_8 (_Sync_word_8 *ptr, _Sync_word_8 value) { _Sync_word_8 tmp = *ptr; *ptr =  tmp - value; return tmp; }
_Sync_word_1 __sync_fetch_and_xor_1 (_Sync_word_1 *ptr, _Sync_word_1 value) { _Sync_word_1 tmp = *ptr; *ptr =  tmp ^ value; return tmp; }
_Sync_word_2 __sync_fetch_and_xor_2 (_Sync_word_2 *ptr, _Sync_word_2 value) { _Sync_word_2 tmp = *ptr; *ptr =  tmp ^ value; return tmp; }
_Sync_word_4 __sync_fetch_and_xor_4 (_Sync_word_4 *ptr, _Sync_word_4 value) { _Sync_word_4 tmp = *ptr; *ptr =  tmp ^ value; return tmp; }
_Sync_word_8 __sync_fetch_and_xor_8 (_Sync_word_8 *ptr, _Sync_word_8 value) { _Sync_word_8 tmp = *ptr; *ptr =  tmp ^ value; return tmp; }

bool         __sync_bool_compare_and_swap_1(_Sync_word_1 *ptr, _Sync_word_1 oldval, _Sync_word_1 newval) { _Sync_word_1 tmp = *ptr; if (*ptr == oldval) *ptr = newval; return (bool)(tmp==oldval); }
bool         __sync_bool_compare_and_swap_2(_Sync_word_2 *ptr, _Sync_word_2 oldval, _Sync_word_2 newval) { _Sync_word_2 tmp = *ptr; if (*ptr == oldval) *ptr = newval; return (bool)(tmp==oldval); }
bool         __sync_bool_compare_and_swap_4(_Sync_word_4 *ptr, _Sync_word_4 oldval, _Sync_word_4 newval) { _Sync_word_4 tmp = *ptr; if (*ptr == oldval) *ptr = newval; return (bool)(tmp==oldval); }
bool         __sync_bool_compare_and_swap_8(_Sync_word_8 *ptr, _Sync_word_8 oldval, _Sync_word_8 newval) { _Sync_word_8 tmp = *ptr; if (*ptr == oldval) *ptr = newval; return (bool)(tmp==oldval); }
_Sync_word_1 __sync_val_compare_and_swap_1(_Sync_word_1 *ptr, _Sync_word_1 oldval, _Sync_word_1 newval) { _Sync_word_1 tmp = *ptr; if (*ptr == oldval) *ptr = newval; return tmp; }
_Sync_word_2 __sync_val_compare_and_swap_2(_Sync_word_2 *ptr, _Sync_word_2 oldval, _Sync_word_2 newval) { _Sync_word_2 tmp = *ptr; if (*ptr == oldval) *ptr = newval; return tmp; }
_Sync_word_4 __sync_val_compare_and_swap_4(_Sync_word_4 *ptr, _Sync_word_4 oldval, _Sync_word_4 newval) { _Sync_word_4 tmp = *ptr; if (*ptr == oldval) *ptr = newval; return tmp; }
_Sync_word_8 __sync_val_compare_and_swap_8(_Sync_word_8 *ptr, _Sync_word_8 oldval, _Sync_word_8 newval) { _Sync_word_8 tmp = *ptr; if (*ptr == oldval) *ptr = newval; return tmp; }

/*
_Sync_word_1 __sync_lock_test_and_set_1(_Sync_word_1 *ptr, _Sync_word_1 value) { }
_Sync_word_2 __sync_lock_test_and_set_2(_Sync_word_2 *ptr, _Sync_word_2 value) { }
_Sync_word_4 __sync_lock_test_and_set_4(_Sync_word_4 *ptr, _Sync_word_4 value) { }
_Sync_word_8 __sync_lock_test_and_set_8(_Sync_word_8 *ptr, _Sync_word_8 value) { }
void __sync_synchronize(void) { }
*/

