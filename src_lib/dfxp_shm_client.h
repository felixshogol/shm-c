#ifndef DFXP_SHM_CLIENT_H
#define DFXP_SHM_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif


int ShmInit(const char *name, int oflag, int mode);
int ShmWrite(dfxp_shm_t *shm);


#ifdef __cplusplus
}
#endif

#endif