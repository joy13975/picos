#ifndef _PICOS_H_
#define _PICOS_H_

void picos_init();

#define picos_register(a, b) _picos_register((void**) a, b)
void _picos_register(void **ptr_to_src, size_t bytes);

void picos_enable_disk_dump(const char* prefix, int every_n_chkpts);

void picos_checkpoint_now();

void picos_warm_recover();

void picos_cold_recover(const char* prefix, long from_pid);

void picos_finalise();


#endif //_PICOS_H_

