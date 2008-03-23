/////////////////////////////////////////////////////////////////////////////
//
//  Captury's strip-soname tool - http://rm-rf.in/captury
//  $Id$
//
//  Copyright (c) 2007 by Christian Parpart <trapni@gentoo.org>
//
//  This file as well as its whole library is licensed under
//  the terms of GPL. See the file COPYING.
//
/////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

#include <libelf.h>
#include <gelf.h>

int main(int argc, const char *argv[]) {
	elf_version(EV_CURRENT);

	int fd = open(argv[1], O_RDWR);
	assert(fd != -1);

	Elf *elf = elf_begin(fd, ELF_C_RDWR_MMAP, 0);
	assert(elf != 0);

	GElf_Ehdr ehdr_mem;
	GElf_Ehdr *ehdr = gelf_getehdr(elf, &ehdr_mem);
	assert(ehdr != 0);

	for (int i = 0; i < ehdr->e_phnum; ++i) {
		GElf_Phdr phdr_mem;
		GElf_Phdr *phdr = gelf_getphdr(elf, i, &phdr_mem);
		assert(phdr != 0);

		if (phdr->p_type == PT_DYNAMIC) {
			Elf_Scn *scn = gelf_offscn(elf, phdr->p_offset);
			GElf_Shdr shdr_mem;
			GElf_Shdr *shdr = gelf_getshdr(scn, &shdr_mem);
			if (!shdr)
				continue; // coult not retrieve no section header

			if (shdr->sh_type == SHT_DYNAMIC) {
				Elf_Data *data = elf_getdata(scn, 0);
				if (!data)
					continue; // could not get section data

				size_t shstrndx;
				if (elf_getshstrndx(elf, &shstrndx) < 0)
					continue; // could not get section header string table index

				//printf("section header string table index: %d\n", shstrndx);

				for (int k = 0; k < shdr->sh_size / shdr->sh_entsize; ++k) {
					GElf_Dyn dyn_mem;
					GElf_Dyn *dyn = gelf_getdyn(data, k, &dyn_mem);
					if (!dyn)
						continue;

					switch (dyn->d_tag) {
#if 0
						case DT_NEEDED: {
							char *soname = elf_strptr(elf, shdr->sh_link, dyn->d_un.d_val);
							printf("Shared library: [%s]\n", soname);
							break;
						}
#endif
						case DT_SONAME: {
							char *soname = elf_strptr(elf, shdr->sh_link, dyn->d_un.d_val);
							printf("Library soname: [%s] (section idx %d, offset %ld)\n", soname, shdr->sh_link, dyn->d_un.d_val);

							// FIXME: how to drop this entry from string table, or how to rename the string value?
							dyn->d_tag = DT_LOPROC; // something unused
							gelf_update_dyn(data, k, dyn);
							break;
						}
						default:
							break;
					}
				}
			}
		}
	}

//	elf_update(elf, ELF_C_WRITE);

	elf_end(elf);
	close(fd);

	return 0;
}
