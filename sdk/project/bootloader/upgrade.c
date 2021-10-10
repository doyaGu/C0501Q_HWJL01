#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include "bootloader.h"
#include "config.h"

static char pkgFilePath[PATH_MAX];
static ITCFileStream fileStream;

#define MAX_ADDRESSBOOK 24

static uint32_t DcpsGetSize( uint8_t *Src, uint32_t SrcLen )
{
	uint32_t DcpsSize=0;
	uint32_t i=0;
	uint32_t out_len=0;
	uint32_t in_len=0;

	while(i<SrcLen)
	{
		out_len = ((Src[i+3]) | (Src[i+2]<<8) | (Src[i+1]<<16) | (Src[i]<<24));
        i=i+4;
        in_len = ((Src[i+3]) | (Src[i+2]<<8) | (Src[i+1]<<16) | (Src[i]<<24));
        i=i+4;

        if (out_len == 0)
			break;

        if( in_len < out_len)	DcpsSize += out_len;
        else					DcpsSize += in_len;

		i += in_len;
	}
	return	DcpsSize;
}

static int CountAddressbook(void)
{
    struct stat sb;
    char *filepath[PATH_MAX];
    int i=0;

    for(i= 1; i <= MAX_ADDRESSBOOK; i++) {
        sprintf(filepath, "A:/addressbook%d.ucl", i);
        if(stat(filepath, &sb) != 0) {
            break;
        }
    }

    return i-1;
}

static int CopyUclFileMore(char *ucl_filename)
{
    int ret = 0;
    FILE *f = NULL;
    uint8_t* filebuf = NULL;
    uint8_t* xmlbuf = NULL;
    char xml_filename[PATH_MAX];
    char buf[PATH_MAX];
    char filepath[PATH_MAX];
    struct stat sb;
    int readsize, compressedSize, xmlsize;
    int index = 0;

    sprintf(filepath, "A:/%s", ucl_filename);

    f = fopen(filepath, "rb");
    if (!f)
    {
        printf("file open %s fail\n", filepath);
        goto error;
    }

    if (fstat(fileno(f), &sb) == -1)
    {
        printf("get file size fail\n");
        goto error;
    }

    filebuf = malloc(sb.st_size);
    if (!filebuf)
        goto error;

    readsize = fread(filebuf, 1, sb.st_size, f);
    assert(readsize == sb.st_size);

    fclose(f);
    f = NULL;

    compressedSize = sb.st_size - 18;
    xmlsize = DcpsGetSize(filebuf + 18, compressedSize);

    xmlbuf = malloc(xmlsize + xmlsize / 8 + 256);
    if (!xmlbuf)
        goto error;

#ifdef CFG_DCPS_ENABLE
    // hardware decompress
    ioctl(ITP_DEVICE_DECOMPRESS, ITP_IOCTL_INIT, NULL);
    ret = write(ITP_DEVICE_DECOMPRESS, filebuf + 18, compressedSize);
    assert(ret == compressedSize);
    ret = read(ITP_DEVICE_DECOMPRESS, xmlbuf, xmlsize);
    assert(ret == xmlsize);
    ioctl(ITP_DEVICE_DECOMPRESS, ITP_IOCTL_EXIT, NULL);

#else
    // software decompress
    ret = SoftwareDecompress(xmlbuf, filebuf);
    assert(ret == 0);

#endif // CFG_DCPS_ENABLE

    free(filebuf);
    filebuf = NULL;

    sscanf(ucl_filename, "addressbook%d.ucl", &index);
    sprintf(xml_filename, "addressbook%d.xml", index);

    strcpy(buf, "D:/");
    strcat(buf, xml_filename);

    f = fopen(buf, "wb");
    if (!f)
    {
        printf("file open %s fail\n", buf);
        goto error;
    }

    ret = fwrite(xmlbuf, 1, xmlsize, f);
    assert(ret == xmlsize);

    free(xmlbuf);

    fclose(f);
    f = NULL;

    printf("save to %s\n", buf);

    return 0;
error:
    if (xmlbuf)
        free(xmlbuf);

    if (filebuf)
        free(filebuf);

    if (f)
        fclose(f);

    return -1;
}

/*Notice: copy addressbook only for outdoor*/
int CopyUclFile(void)
{
        int ret = 0;
        FILE *f = NULL;
        uint8_t* filebuf = NULL;
        uint8_t* xmlbuf = NULL;
        int k = 0;
        struct stat sb;
        int readsize, compressedSize, xmlsize, addressbook_count = 0;
        char ucl_filename[32];

        f = fopen("A:/addressbook.ucl", "rb");
        if (!f)
        {
            printf("file open addressbook.ucl fail\n");
            goto error;
        }

        if (fstat(fileno(f), &sb) == -1)
        {
            printf("get file size fail\n");
            goto error;
        }

        filebuf = malloc(sb.st_size);
        if (!filebuf)
            goto error;

        readsize = fread(filebuf, 1, sb.st_size, f);
        assert(readsize == sb.st_size);

        fclose(f);
        f = NULL;

        compressedSize = sb.st_size - 18;
        xmlsize = DcpsGetSize(filebuf + 18, compressedSize);

        xmlbuf = malloc(xmlsize + xmlsize / 8 + 256);
        if (!xmlbuf)
            goto error;

#ifdef CFG_DCPS_ENABLE
        // hardware decompress
        ioctl(ITP_DEVICE_DECOMPRESS, ITP_IOCTL_INIT, NULL);
        ret = write(ITP_DEVICE_DECOMPRESS, filebuf + 18, compressedSize);
        assert(ret == compressedSize);
        ret = read(ITP_DEVICE_DECOMPRESS, xmlbuf, xmlsize);
        assert(ret == xmlsize);
        ioctl(ITP_DEVICE_DECOMPRESS, ITP_IOCTL_EXIT, NULL);

#else
        // software decompress
        ret = SoftwareDecompress(xmlbuf, filebuf);
        assert(ret == 0);

#endif // CFG_DCPS_ENABLE

        free(filebuf);
        filebuf = NULL;

        f = fopen("D:/addressbook.xml", "wb");
        if (!f)
        {
            printf("file open addressbook.xml fail\n");
            goto error;
        }

        ret = fwrite(xmlbuf, 1, xmlsize, f);
        assert(ret == xmlsize);

        free(xmlbuf);

        fclose(f);
        f = NULL;

        printf("save to D:/addressbook.xml\n");

        addressbook_count = CountAddressbook();
        for(k = 1;k <= addressbook_count; k++) {
            sprintf(ucl_filename, "addressbook%d.ucl", k);
            CopyUclFileMore(ucl_filename);
        }

        return 0;
    error:
        if (xmlbuf)
            free(xmlbuf);

        if (filebuf)
            free(filebuf);

        if (f)
            fclose(f);

        return -1;

}

ITCStream* OpenUpgradePackage(void)
{
    ITPDriveStatus* driveStatusTable;
    ITPDriveStatus* driveStatus = NULL;
    int i;

    // try to find the package drive
    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_GET_TABLE, &driveStatusTable);

    for (i = ITP_MAX_DRIVE - 1; i >= 0; i--)
    {
        driveStatus = &driveStatusTable[i];
        if (driveStatus->avail && driveStatus->removable)
        {
            char buf[PATH_MAX], *ptr;

            LOG_DBG "drive[%d]:disk=%d\n", i, driveStatus->disk LOG_END

            // get file path from list
            strcpy(buf, CFG_UPGRADE_FILENAME_LIST);
            ptr = strtok(buf, " ");
            do
            {
                strcpy(pkgFilePath, driveStatus->name);
                strcat(pkgFilePath, ptr);

                if (itcFileStreamOpen(&fileStream, pkgFilePath, false) == 0)
                {
                    LOG_INFO "Found package file %s\n", pkgFilePath LOG_END
                    return &fileStream.stream;
                }
                else
                {
                    LOG_DBG "try to fopen(%s) fail:0x%X\n", pkgFilePath, errno LOG_END
                }
            }
            while ((ptr = strtok(NULL, " ")) != NULL);
        }
    }
    LOG_DBG "cannot find package file.\n" LOG_END
    return NULL;
}

void DeleteUpgradePackage(void)
{
    if (remove(pkgFilePath))
        LOG_ERR "Delete %s fail: %d\n", pkgFilePath, errno LOG_END
}
