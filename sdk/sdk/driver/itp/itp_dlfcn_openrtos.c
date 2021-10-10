/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL dynamic loading functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include <errno.h>
#include <dlfcn.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include "itp_cfg.h"

typedef struct 
{
    char   *symbol_name;
    void   *handler;
} table_entry;

static const table_entry table[] =
{
    { "printf", printf },
    { "puts",   puts }
};

typedef uint32_t Elf32_Addr;
typedef uint32_t Elf32_Off;
typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Word;
typedef int32_t  Elf32_Sword;

#define EI_MAG0                 0
#define EI_MAG1                 1
#define EI_MAG2                 2
#define EI_MAG3                 3
#define EI_CLASS                4
#define EI_DATA                 5 
#define EI_NIDENT               16
#define ELFMAG0                 0x7f            // magic number, byte 0 
#define ELFMAG1                 'E'             // magic number, byte 1 
#define ELFMAG2                 'L'             // magic number, byte 2 
#define ELFMAG3                 'F'             // magic number, byte 3 
#define ELFCLASS32              1               // 32-bit objects.
#define ET_REL                  1               // Relocatable file.
#define SHN_COMMON              0xfff2          /* common symbol */
#define SHN_UNDEF               0               /* undefined */
#define SHT_RELA                4               /* relocation section with addends*/
#define SHT_REL                 9               /* relation section without addends */
#define STB_LOCAL               0               /* Local symbol */
#define STB_GLOBAL              1               /* Global symbol */
#define STB_WEAK                2               /* like global - lower precedence */
#define STT_NOTYPE              0               /* not specified */
#define STT_OBJECT              1               /* data object */
#define STT_FUNC                2               /* function */
#define STT_SECTION             3               /* section */
#define ELF_STRING_strtab       ".strtab"
#define ELF_STRING_symtab       ".symtab"
#define ELF32_R_SYM(i)          ((i) >> 8)
#define ELF32_R_TYPE(i)         ((unsigned char) (i))
#define ELF32_ST_TYPE(x)        (((unsigned int) x) & 0xf)
#define ELF32_ST_BIND(x)        ((x) >> 4)
#define ELFDATA2LSB             1 // Little Endian
#define ELFDATA2MSB             2 // Big Endian.

#define R_ARM_PC24              1  // PC relative 26 bit branch.
#define R_ARM_ABS32             2  // Direct 32 bit.
#define R_ARM_CALL              28 // PC relative 26 bit call (EABI).
#define R_ARM_JUMP24            29 // PC relative 26 bit branch (EABI).
#define R_ARM_V4BX              40 // Fix of interworking for ARMv4 cores.
#define ELF_ARCH_MACHINE_TYPE   40 // ARM
#define ELF_ARCH_ENDIANNESS     ELFDATA2LSB

typedef struct
{
    unsigned char e_ident[EI_NIDENT];
    Elf32_Half e_type;
    Elf32_Half e_machine;
    Elf32_Word e_version;
    Elf32_Addr e_entry;
    Elf32_Off  e_phoff;
    Elf32_Off  e_shoff;
    Elf32_Word e_flags;
    Elf32_Half e_ehsize;
    Elf32_Half e_phentsize;
    Elf32_Half e_phnum;
    Elf32_Half e_shentsize;
    Elf32_Half e_shnum;
    Elf32_Half e_shstrndx;
} Elf32_Ehdr;

typedef struct {
    Elf32_Word  sh_name;        /* name - index into section header
                                   string table section */
    Elf32_Word  sh_type;        /* type */
    Elf32_Word  sh_flags;       /* flags */
    Elf32_Addr  sh_addr;        /* address */
    Elf32_Off   sh_offset;      /* file offset */
    Elf32_Word  sh_size;        /* section size */
    Elf32_Word  sh_link;        /* section header table index link */
    Elf32_Word  sh_info;        /* extra information */
    Elf32_Word  sh_addralign;   /* address alignment */
    Elf32_Word  sh_entsize;     /* section entry size */
} Elf32_Shdr;

typedef struct {
    Elf32_Word          st_name;        /* name - index into string table */
    Elf32_Addr          st_value;       /* symbol value */
    Elf32_Word          st_size;        /* symbol size */
    unsigned char       st_info;        /* type and binding */
    unsigned char       st_other;       /* visibility */
    Elf32_Half          st_shndx;       /* section header index */
} Elf32_Sym;

typedef struct 
{
        Elf32_Addr      r_offset;       /* offset of relocation */
        Elf32_Word      r_info;         /* symbol table index and type */
} Elf32_Rel;

typedef struct ELF_OBJECT
{
    FILE*  ptr;

    // This is the absolute address in memory where the library resides.
    Elf32_Ehdr*   p_elfhdr;

    // Start of the section header.
    Elf32_Shdr*   p_sechdr;

    int32_t     hdrndx_symtab;
    int32_t     hdrndx_strtab;

    uint32_t    **sections;

} ELF_OBJECT, *PELF_OBJECT;

static char *last_error;

// Allocates memory and loads the contents of a specific ELF section.
// Returns the address of the newly allocated memory, of 0 for any error.
static uint32_t *load_elf_section(PELF_OBJECT p, uint32_t idx)
{
    // Make sure we are not requesting the loading of a section for which we
    //  have no pointer.
    ASSERT(idx < (uint32_t)p->p_elfhdr->e_shnum + 1);
    
    // If this section has already been loaded its pointer is already available
    //  in the sections[] array.
    if (p->sections[idx] != 0)
        return p->sections[idx];
        
    p->sections[idx] = (uint32_t*)malloc(p->p_sechdr[idx].sh_size);
    if (p->sections[idx] == 0)
    {
        LOG_ERR "ERROR IN MALLOC\r\n" LOG_END
        last_error = "ERROR IN MALLOC";
        return (void*)0;
    }
    fseek(p->ptr, p->p_sechdr[idx].sh_offset, SEEK_SET);
    fread((void *)p->sections[idx], sizeof(char), p->p_sechdr[idx].sh_size, p->ptr);
    
    return p->sections[idx];
}    

static void delete_elf_section(PELF_OBJECT p, uint32_t idx)
{
    if (p->sections[idx] == 0)
        return;
        
    free(p->sections[idx]);
    p->sections[idx] = 0; 
}    

// Frees all the memory allocated for a particular ELF object. Also calls
//  the close() function to close files or sockets, and finally frees up
//  the ELF object altogether.
static void free_elf_object(PELF_OBJECT p)
{
    int32_t i;
        
    for (i = 0; i < p->p_elfhdr->e_shnum + 1; i++)
        if (p->sections[i] != 0)
            delete_elf_section(p, i);

    if (p->sections != 0)
        free(p->sections); 

    if (p->p_sechdr != 0)
        free(p->p_sechdr); 

    if (p->p_elfhdr != 0)
        free(p->p_elfhdr); 

    fclose(p->ptr);    
    free(p);
}        

static char
*sanity_check(PELF_OBJECT p)
{
    if ((p->p_elfhdr->e_ident[EI_MAG0] != ELFMAG0)  || 
         (p->p_elfhdr->e_ident[EI_MAG1] != ELFMAG1)  ||
         (p->p_elfhdr->e_ident[EI_MAG2] != ELFMAG2 ) || 
         (p->p_elfhdr->e_ident[EI_MAG3] != ELFMAG3)  || 
         (p->p_elfhdr->e_ident[EI_CLASS] != ELFCLASS32))
        return "INVALID ELF HEADER";

    // We only work with relocatable files. No dynamic linking.
    if (p->p_elfhdr->e_type != ET_REL)
        return "NOT RELOCATABLE";

    // These #defines are sitting in the hal.
    if (p->p_elfhdr->e_machine != ELF_ARCH_MACHINE_TYPE)
    {
        return "INVALID ARCHITECTURE";
    }    

    if (p->p_elfhdr->e_ident[EI_DATA] != ELF_ARCH_ENDIANNESS)
        return "INVALID ENDIAN";

    return 0;
}     

// Load only the ELF header and the sections header. These are the only
//  sections loaded during library initialization. All the other sections
//  will be loaded on demand when needed during the relocation process and,
//  when possible, dumped after use.
static int32_t load_sections(PELF_OBJECT p)
{
    char     *error_string;
    int32_t  idx;
    uint32_t* section_addr;

    // Load the ELF header.
    p->p_elfhdr = (Elf32_Ehdr*)malloc(sizeof(Elf32_Ehdr));
    if (p->p_elfhdr == 0)
        return -1;
        
    fseek(p->ptr, 0, SEEK_SET);
    fread(p->p_elfhdr, sizeof(char), sizeof(Elf32_Ehdr), p->ptr);

    error_string = sanity_check(p);
    if (error_string != 0)
    {
        last_error = "ERROR IN ELF HEADER";
        return -1;
    }    

    // Allocate an array that can hold an address to all the section of this
    //  library. This is not strictly optimal, since some sections do not
    //  need to be loaded all the time. Allocate an extra pointer for the
    //  COMMON area. 
    p->sections = malloc((p->p_elfhdr->e_shnum + 1) * sizeof(uint32_t));
    if (p->sections == 0)
    {
        LOG_ERR "ERROR IN MALLOC\r\n" LOG_END
        last_error = "ERROR IN MALLOC";
        return -1;
    }
    memset(p->sections, 0, (p->p_elfhdr->e_shnum + 1) * sizeof(uint32_t));
    
    // Now that the header is loaded, load the sections header.
    p->p_sechdr = (Elf32_Shdr*)malloc(p->p_elfhdr->e_shnum * p->p_elfhdr->e_shentsize);
    if (p->p_sechdr == 0)
    {
        LOG_ERR "ERROR IN MALLOC\r\n" LOG_END
        last_error = "ERROR IN MALLOC";
        return -1;
    }
    fseek(p->ptr, p->p_elfhdr->e_shoff, SEEK_SET);
    fread(p->p_sechdr, p->p_elfhdr->e_shentsize, p->p_elfhdr->e_shnum, p->ptr);

    // Load the section header string table. This is a byte oriented table,
    //  so alignment is not an issue.
    idx = p->p_elfhdr->e_shstrndx;
    section_addr = load_elf_section(p, idx);
    if (section_addr == 0)
        return -1;
        
    return 0;
}

// Returns the starting address of a section. If the section is not already
//  loaded in memory, area for it will be allocated and the section will be
//  loaded.
static uint32_t *section_address(PELF_OBJECT p, uint32_t idx)
{
    if (p->sections[idx] == 0)
        p->sections[idx] = load_elf_section(p, idx);
        
    return p->sections[idx];
}

static uint32_t find_common_size(PELF_OBJECT p)
{
    int32_t i, common_size = 0;
    Elf32_Sym *p_symtab = (Elf32_Sym*)p->sections[p->hdrndx_symtab];
     
    // Total number of entries in the symbol table.
    int symtab_entries = p->p_sechdr[p->hdrndx_symtab].sh_size / 
                                p->p_sechdr[p->hdrndx_symtab].sh_entsize;
                                
    for (i = 1; i < symtab_entries; i++)
        if (p_symtab[i].st_shndx == SHN_COMMON)
        {
            // In the case of an SHN_COMMON symbol the st_value field holds 
            //  alignment constraints.
            uint32_t boundary = p_symtab[i].st_value - 1;

            // Calculate the next byte boundary.
            common_size = (common_size + boundary) & ~boundary;
            common_size += p_symtab[i].st_size;
        }    

    LOG_DBG "common_size = %d\n", common_size LOG_END

    return common_size;
}

static void *external_address(PELF_OBJECT p, uint32_t sym_index)
{
    uint8_t*    tmp2;
    Elf32_Sym *p_symtab;
    uint8_t *p_strtab;
    const table_entry *entry = table;
    const int32_t count = ITH_COUNT_OF(table);
  
    p_symtab = (Elf32_Sym*)section_address(p, p->hdrndx_symtab);
    p_strtab = (uint8_t*)section_address(p, p->hdrndx_strtab);
  
    // This is the name of the external reference to search.
    tmp2 = p_strtab + p_symtab[sym_index].st_name;
    while (entry != &table[count])
    {
        if (!strcmp((const char*)tmp2, entry->symbol_name ))
            return entry->handler;
        entry++;
    }

    // Symbol not found.
    return 0;
}

static void *local_address(PELF_OBJECT p, uint32_t sym_index)
{
    uint32_t data_sec, addr;
    Elf32_Sym *p_symtab;

    p_symtab = (Elf32_Sym*)section_address(p, p->hdrndx_symtab);
    
    // Find out the section number in which the data for this symbol is 
    //  located.
    data_sec = p_symtab[sym_index].st_shndx;    

    // From the section number we get the start of the memory area in 
    //  memory.
    addr = (uint32_t)section_address(p, data_sec);

    // And now return the address of the data.
    return (void*)(addr + p_symtab[sym_index].st_value);
}    

// input:
// p          : Pointer to the elf file object
// sym_index  : Index of the symbol to be searched (in the SYMTAB)
//
// out:
// 0          : Symbol not found
// Other      : Address of the symbol in absolute memory.
static void *symbol_address(PELF_OBJECT p, uint32_t sym_index)
{
    uint32_t addr;
    Elf32_Sym *p_symtab = (Elf32_Sym*)section_address(p, p->hdrndx_symtab);
    uint8_t sym_info = p_symtab[sym_index].st_info;
    
    switch (ELF32_ST_TYPE(sym_info))
    {
    case STT_NOTYPE:
    case STT_FUNC:
    case STT_OBJECT:
        switch (ELF32_ST_BIND(sym_info))
        {
        case STB_LOCAL:
        case STB_GLOBAL:
            if (p_symtab[sym_index].st_shndx == SHN_UNDEF) 
                return external_address(p, sym_index);
            else
                return local_address(p, sym_index);
                
        case STB_WEAK:
            addr = (uint32_t)external_address(p, sym_index);
            if (addr != 0)
                return (void*)addr;
            else    
                return local_address(p, sym_index);
                
        default:
            return 0;
        }
        break;
        
    case STT_SECTION:
        // Return the starting address of a section, given its index.
        return (void*)section_address(p, p_symtab[sym_index].st_shndx);
        
    default:
        return 0;
    }
}

// sym_type  Type of relocation to apply,
// mem       Address in memory to modify (relocate).
// sym_value The value of the symbol to use for the relocation.
// The proper relocation to apply (i.e. the proper use of mem and sym_value)
//  depend on the relocation to apply. The number and types of relocations
//  that must be supported by any given architecture is spelled in the ELF/EABI
//  guide for that architecture.
static int32_t relocate(int32_t sym_type, uint32_t mem, int32_t sym_value)
{
    int32_t offset;
    uint32_t *mem_addr = (uint32_t *)mem;

    switch(sym_type)
    {
    case R_ARM_ABS32:
        memcpy(&offset, mem_addr, sizeof (int32_t));
        offset += sym_value;
        memcpy(mem_addr, &offset, sizeof (int32_t));
        break;
        
    case R_ARM_PC24:
    case R_ARM_CALL:
    case R_ARM_JUMP24:
        offset = (*mem_addr & 0x00FFFFFF) << 2;
        if (offset & 0x02000000)
            offset -= 0x04000000;     // Sign extend.
        *mem_addr &= 0xff000000;      // Mask off the entire offset bits.
        offset = sym_value - mem + offset;  // This is the new offset.
        if ((offset & 0x03) || (offset >= (int32_t)0x04000000) ||
                                (offset <= (int32_t)0xFC000000))
            return -1;                                     
        *mem_addr |= (offset >> 2) & 0x00FFFFFF;
        break;
        
    case R_ARM_V4BX:
        // For now only ARMv4T and later cores (with Thumb) are supported.
        break;
        
    default:
        ASSERT(!"FIXME: Unknown relocation value!!!\r\n");
        return -1;
    }
    return 0;
}

// Loads the relocation information, relocates, and dumps the relocation
//  information once the process is complete.
static int32_t relocate_section(PELF_OBJECT p, uint32_t r_shndx)
{
    int       rc;
    Elf32_Rel *p_rel = (Elf32_Rel *)load_elf_section(p, r_shndx);
	uint32_t i, r_target_shndx, r_target_addr, r_entries;

    if (p_rel == 0)
        return -1;

    // Now we can get the address of the contents of the section to modify.
    r_target_shndx = p->p_sechdr[r_shndx].sh_info;
    r_target_addr  = (uint32_t)section_address(p, r_target_shndx);

    // Perform relocatation for each of the members of this table.
    r_entries = p->p_sechdr[r_shndx].sh_size / p->p_sechdr[r_shndx].sh_entsize;
    for (i = 0; i < r_entries; i++)
    {
        Elf32_Addr  r_offset  = p_rel[i].r_offset; 
        Elf32_Word  r_type    = ELF32_R_TYPE(p_rel[i].r_info); 
        uint32_t  sym_index = ELF32_R_SYM(p_rel[i].r_info);
        Elf32_Sword r_addend  = 0; 

        uint32_t sym_value = (uint32_t)symbol_address(p, sym_index);
        
        rc = relocate(r_type,
                              r_target_addr + r_offset, 
                              sym_value + r_addend);
        if (rc != 0)
        {
            LOG_DBG "Error while relocating symbol\n" LOG_END
            return -1;
        }    
    }

    // After the relocation is done, the relocation table can be dumped.
    delete_elf_section(p, r_shndx);
    return 0;
}

void* dlopen(const char *__file, int __mode)
{
    int i, rc;
    char *p_shstrtab;
    uint32_t common_size;
    PELF_OBJECT e_obj = (PELF_OBJECT)0;

    FILE *fp = fopen(__file, "rb");
    if (fp == NULL)
    {
        LOG_ERR "FILE NOT FOUND\r\n" LOG_END
        last_error = "FILE NOT FOUND";
        return (void*)0;
    }
    
    // Create a file object to keep track of this library.
    e_obj = (PELF_OBJECT)malloc(sizeof(ELF_OBJECT));
    if (e_obj == 0)
    {
        LOG_ERR "ERROR IN MALLOC\r\n" LOG_END
        last_error = "ERROR IN MALLOC";
        fclose(fp); 
        return (void*)0;
    }
    memset(e_obj, 0, sizeof(ELF_OBJECT));
    e_obj->ptr = fp;
        
    rc = load_sections(e_obj);
    if (rc != 0)
    {
        free_elf_object(e_obj);
        return 0;
    }    

    // Find the section index for the .shstrtab section. The names of the 
    //  sections are held here, and are the only way to identify them.
    p_shstrtab = (char*)section_address(e_obj, 
                                                e_obj->p_elfhdr->e_shstrndx);
    if (p_shstrtab == 0)
    {
        free_elf_object(e_obj);
        return 0;
    }    

    // .symtab section and .strtab. We have to go through the section names
    //  to find where they are.
    for (i = 1; i < e_obj->p_elfhdr->e_shnum; i++)
    {
        // Now look for the index of .symtab. These are the symbols needed for 
        //  the symbol retrieval as well as relocation.
        if (!strcmp(p_shstrtab + e_obj->p_sechdr[i].sh_name, ELF_STRING_symtab))
        {              
            e_obj->hdrndx_symtab = i;
            load_elf_section(e_obj, i);
            if (e_obj->sections[i] == 0)
            {
                free_elf_object(e_obj);
                return 0;
            }    
        }    
        
        // Load the table with the names of all the symbols. We need this
        //  to compare the name of external references symbols against the
        //  names in the in the CYG_HAL_TABLE provided by the user.
        if (!strcmp(p_shstrtab + e_obj->p_sechdr[i].sh_name, ELF_STRING_strtab))
        {              
            e_obj->hdrndx_strtab = i;
            load_elf_section(e_obj, i);
            if (e_obj->sections[i] == 0)
            {
                free_elf_object(e_obj);
                return 0;
            }    
        }    
    }                                              

    ASSERT(e_obj->hdrndx_symtab != 0);
    ASSERT(e_obj->hdrndx_strtab != 0);

    // Now look for symbols in the COMMON area. The COMMON symbols are a 
    //  special case, because the area they reside in must be sized up
    //  and allocated separately from the other sections, which appear in
    //  the sections header and can be read out of the library itself.
    // Extra room in the 'sections' array has already been allocated to hold 
    //  the pointer to the commons area.
    common_size = find_common_size(e_obj); 
    if (common_size != 0)
    {
        uint32_t com_shndx = e_obj->p_elfhdr->e_shnum;
        int32_t  com_offset = 0;
        int symtab_entries;
        Elf32_Sym *p_symtab;
        
        e_obj->sections[com_shndx] = (uint32_t*)malloc(common_size);
        if (e_obj->sections[com_shndx] == 0)
        {
            free_elf_object(e_obj);
            return 0;
        }    

        // Now find all the symbols in the SHN_COMMON area and make 
        //  them  point to the newly allocated COM area.
        symtab_entries = e_obj->p_sechdr[e_obj->hdrndx_symtab].sh_size / 
                              e_obj->p_sechdr[e_obj->hdrndx_symtab].sh_entsize;
        p_symtab = (Elf32_Sym*)e_obj->sections[e_obj->hdrndx_symtab];
    
        for (i = 1; i < symtab_entries; i++)
        {
            if (p_symtab[i].st_shndx == SHN_COMMON)
            {             
                uint32_t boundary = p_symtab[i].st_value - 1;
                // Calculate the next byte boundary.
                com_offset = (com_offset + boundary) & ~boundary;
                p_symtab[i].st_shndx = com_shndx;
                p_symtab[i].st_value = com_offset;
                com_offset += p_symtab[i].st_size;
            }
        }    
    }

    for (i = 1; i < e_obj->p_elfhdr->e_shnum; i++)
    {
        // Find all the '.rel' or '.rela' sections and relocate them.
        if ((e_obj->p_sechdr[i].sh_type == SHT_REL) ||
                                  (e_obj->p_sechdr[i].sh_type == SHT_RELA))
        {
            // Load and relocate the section.
            rc = relocate_section(e_obj, i);
            if (rc < 0)
            {
                LOG_DBG "Relocation unsuccessful\n", common_size LOG_END

                free_elf_object(e_obj);
                return 0;
            }    
        }    
    }    

    // Synch up the caches before calling any function in the library.
    ithFlushDCache();
    ithInvalidateICache();

    return ((void*)e_obj);
}

void* dlsym(void* __handle, const char* __name)
{
    PELF_OBJECT p = (PELF_OBJECT)__handle;
    int         i;
    char *p_strtab = (char*)p->sections[p->hdrndx_strtab];
    Elf32_Sym *p_symtab = (Elf32_Sym*)p->sections[p->hdrndx_symtab];
 
    int symtab_entries = p->p_sechdr[p->hdrndx_symtab].sh_size / 
                               p->p_sechdr[p->hdrndx_symtab].sh_entsize;

    for (i = 0; i < symtab_entries; i++)
    {
        char* tmp2 = p_strtab + p_symtab[i].st_name;
        if (!strcmp(tmp2, __name))
        {
            void *const funcPtr = symbol_address(p, i);

            // Synch up the caches before calling any function in the library.
            ithFlushDCache();
            ithInvalidateICache();
            return funcPtr;
        }            
    }

    // Symbol not found.
    LOG_ERR "SYMBOL NOT FOUND\r\n" LOG_END
    return NULL;
}

int dlclose(void *__handle)
{
    PELF_OBJECT p = (PELF_OBJECT)__handle;
    free_elf_object(p);

	return 0;
}

char *dlerror(void)
{
    char* p = last_error;
    last_error = NULL;
    return p;
}
