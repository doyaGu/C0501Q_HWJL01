
#ifndef ITE_IDB_H
#define ITE_IDB_H

#ifdef __cplusplus
extern "C" {
#endif

struct idb_operations
{
    int (*upgradefw)(const char* firmwareData, int firmwareSize);
	int (*upgradepercentage)(void);
};

struct idb_config
{
    const struct idb_operations *ops;
};

int iteIdbInitialize(const struct idb_config *cfg);

int iteIdbTerminate(void);

#ifdef __cplusplus
}
#endif

#endif // ITE_IDB_H

