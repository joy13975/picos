#ifndef _PICOS_H_
#define _PICOS_H_

void picos_init();

void picos_register_primitive(void *ptr_to_prim, size_t size)
{
    __picos_register(false, (void **) ptr_to_prim, size);
}

void picos_register_ptr(void **ptr_to_ptr_to_data, size_t size)
{
    __picos_register(true, ptr_to_ptr_to_data, size);
}

void __picos_register(bool is_ptr, void **ptr_or_double_ptr, size_t size);

void picos_enable_disk_dump(const char* prefix, int every_n_chkpts);

void picos_checkpoint_now();

void picos_warm_recover();

void picos_cold_recover(const char* prefix, long from_pid);

void picos_finalise();


#endif //_PICOS_H_

