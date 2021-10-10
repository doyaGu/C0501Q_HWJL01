
#ifndef ITE_FSG_H
#define ITE_FSG_H

#ifdef __cplusplus
extern "C" {
#endif


#define FSG_MEDIA_NOT_PRESENT	0
#define FSG_MEDIA_READY			1
#define FSG_MEDIA_CHANGE		2

struct fsg_operations
{
    int (*open)(void);
    int (*close)(void);
    int (*response)(uint8_t lun);
    int (*get_capacity)(uint8_t lun, uint32_t *sector_num, uint32_t *block_size);
    bool (*ro)(uint8_t lun);
    void (*eject)(uint8_t lun);
    int (*read_sector)(uint8_t lun, uint32_t blockId, uint32_t sizeInSector, uint8_t *buf); 
    int (*write_sector)(uint8_t lun, uint32_t blockId, uint32_t sizeInSector, uint8_t *buf); 
};


#define FSG_MAX_LUNS	4

struct fsg_config
{
    uint32_t	nluns;

    struct fsg_lun_config {
        uint32_t		removable:1;
        const char*		inquiry_string;
    } luns[FSG_MAX_LUNS];

    const struct fsg_operations *ops;

    char		*manufacturer;
    char		*product;
    char		*serial;

    uint16_t	vendor_id;
    uint16_t	product_id;
    uint16_t	release_num;
    uint8_t	    reserved[2];
};


int iteFsgInitialize(const struct fsg_config *cfg);

int iteFsgTerminate(void);

void* fsg_main_thread(void *);


#ifdef __cplusplus
}
#endif

#endif // ITE_FSG_H
