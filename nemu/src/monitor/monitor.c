/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <elf.h>
#include <memory/paddr.h>

#define MAX_ELF_NUM  5

void init_rand();
void init_log(const char *log_file);
void init_mem();
void init_difftest(char *ref_so_file, long img_size, int port);
void init_device();
void init_sdb();
void init_disasm(const char *triple);

static void welcome() {
  Log("Trace: %s", MUXDEF(CONFIG_TRACE, ANSI_FMT("ON", ANSI_FG_GREEN), ANSI_FMT("OFF", ANSI_FG_RED)));
  IFDEF(CONFIG_TRACE, Log("If trace is enabled, a log file will be generated "
        "to record the trace. This may lead to a large log file. "
        "If it is not necessary, you can disable it in menuconfig"));
  Log("Build time: %s, %s", __TIME__, __DATE__);
  printf("Welcome to %s-NEMU!\n", ANSI_FMT(str(__GUEST_ISA__), ANSI_FG_YELLOW ANSI_BG_RED));
  printf("For help, type \"help\"\n");
  //Log("Exercise: Please remove me in the source code and compile NEMU again.");
  //assert(0);
}

#ifndef CONFIG_TARGET_AM
#include <getopt.h>

void sdb_set_batch_mode();

static char *log_file = NULL;
static char *diff_so_file = NULL;
static char *img_file = NULL;
static char *elf_file[MAX_ELF_NUM] = {NULL};

static uint32_t elf_num = 0;

static int difftest_port = 1234;

#ifdef CONFIG_FTRACE
static uint32_t functs_address[MAX_ELF_NUM][999], functs_size[MAX_ELF_NUM];
static char functs_name[MAX_ELF_NUM][999][999];
static uint32_t ret_address[MAX_ELF_NUM][999], ret_size[MAX_ELF_NUM];
static int ret_id[MAX_ELF_NUM][999];
static uint32_t t_num = 0;
#endif

#ifdef CONFIG_ETRACE
void print_exception(uint32_t pc, uint32_t dnpc, uint32_t errorId)
{
	printf("Jump to 0x%x to deal with exception at pc = 0x%x, errorId: %d\n", pc, dnpc, errorId);
	printf("And current mstatus = 0x%x\n", cpu.mstatus);
}

void exception_return(uint32_t pc, uint32_t dnpc)
{
	printf("Exception returned from 0x%x, and next pc is 0x%x\n", pc, dnpc);
}
#endif

#ifdef CONFIG_FTRACE
static void print_funct(bool flag, uint32_t pos, uint32_t nowpc, uint32_t q)
{
	printf("0x%x: ", nowpc);
	for (int i = 1; i <= t_num * 2; i++) printf(" ");
	if (!flag) 
		printf("call [%s@0x%x]", functs_name[q][pos], functs_address[q][pos]);
	else 
		printf("ret [%s]", functs_name[q][ret_id[q][pos]]);
	printf("\n");
}

void check_funct(uint32_t nowpc, uint32_t jmp_add, uint32_t snpc)
{
	for (int q = 0; q < elf_num; q++) {
		for (int i = 0; i < functs_size[q]; i++)
			if (functs_address[q][i] == jmp_add) {
				print_funct(0, i, nowpc, q);
				t_num++;
				ret_address[q][ret_size[q]] = snpc;
				ret_id[q][ret_size[q]++] = i;
			}
		if (ret_address[q][ret_size[q] - 1] == jmp_add) {
			t_num--;
			print_funct(1, ret_size[q] - 1, nowpc, q);
			ret_size[q]--;
		}
	}
	
	return ;
}

static void load_elf()
{
	char ch[99] = {'\0'};

	if (elf_file[0] == NULL) {
		Log("No elf document is given.");
		return ;
	}
	for (int q = 0; q < elf_num; q++) {
		FILE *fp = fopen(elf_file[q], "rb");

		Elf32_Ehdr ehdr;
		Elf32_Shdr shdrs[99];
		Elf32_Sym symtabs[999];
		int num_of_sym = 0;
		int sym_off = 0;
		bool p = fread(&ehdr, sizeof(Elf32_Ehdr), 1, fp); p = 1; assert(p);
		fseek(fp, ehdr.e_shoff, SEEK_SET);
		p = fread(shdrs, sizeof(Elf32_Shdr), ehdr.e_shnum, fp); p = 1; assert(p);
		for (int i = 0; i < ehdr.e_shnum; i++) {
			//Elf32_Shdr *shdr = &shdrs[i];
			fseek(fp, shdrs[ehdr.e_shstrndx].sh_offset + shdrs[i].sh_name, SEEK_SET);
			p = fread(ch, sizeof(char *), 1, fp); p = 1; assert(p);
			if (strcmp(ch, ".symtab") == 0) {
				num_of_sym = shdrs[i].sh_size / sizeof(Elf32_Sym);
				fseek(fp, shdrs[i].sh_offset, SEEK_SET);
				p = fread(symtabs, sizeof(Elf32_Sym), num_of_sym, fp); p = 1; assert(p);
			}
			if (strcmp(ch, ".strtab") == 0) sym_off = shdrs[i].sh_offset;
		}
		for (int i = 0; i < num_of_sym; i++) {
			if (ELF32_ST_TYPE(symtabs[i].st_info) != STT_FUNC) continue;
			functs_address[q][functs_size[q]] = symtabs[i].st_value;
			fseek(fp, sym_off + symtabs[i].st_name, SEEK_SET);
			p = fread(functs_name[q][functs_size[q]], sizeof(char *), 1, fp); p = 1; assert(p);
			functs_size[q]++;
		}
		fclose(fp);
	}
}
#endif

static long load_img() {
  if (img_file == NULL) {
    Log("No image is given. Use the default build-in image.");
    return 4096; // built-in image size
  }

  FILE *fp = fopen(img_file, "rb");
  Assert(fp, "Can not open '%s'", img_file);

  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);

  Log("The image is %s, size = %ld", img_file, size);

  fseek(fp, 0, SEEK_SET);
  int ret = fread(guest_to_host(RESET_VECTOR), size, 1, fp);
  assert(ret == 1);

  fclose(fp);
  return size;
}

static int parse_args(int argc, char *argv[]) {
  const struct option table[] = {
    {"batch"    , no_argument      , NULL, 'b'},
    {"log"      , required_argument, NULL, 'l'},
    {"diff"     , required_argument, NULL, 'd'},
    {"port"     , required_argument, NULL, 'p'},
    {"help"     , no_argument      , NULL, 'h'},
	{"elf"      , required_argument, NULL, 'e'},
    {0          , 0                , NULL,  0 },
  };
  int o;
  while ( (o = getopt_long(argc, argv, "-bhl:d:p:e:", table, NULL)) != -1) {
    switch (o) {
	  case 'e': elf_file[elf_num++] = optarg; break;
      case 'b': sdb_set_batch_mode(); break;
      case 'p': sscanf(optarg, "%d", &difftest_port); break;
      case 'l': log_file = optarg; break;
      case 'd': diff_so_file = optarg; break;
      case 1: img_file = optarg; return 0;
      default:
	  	printf("%c\n", o);
        printf("Usage: %s [OPTION...] IMAGE [args]\n\n", argv[0]);
        printf("\t-b,--batch              run with batch mode\n");
        printf("\t-l,--log=FILE           output log to FILE\n");
        printf("\t-d,--diff=REF_SO        run DiffTest with reference REF_SO\n");
        printf("\t-p,--port=PORT          run DiffTest with port PORT\n");
		printf("\t-e,--elf=ELF            receive elf document");
        printf("\n");
        exit(0);
    }
  }

  
  return 0;
}

void init_monitor(int argc, char *argv[]) {
  /* Perform some global initialization. */

  /* Parse arguments. */
  parse_args(argc, argv);

  /* Set random seed. */
  init_rand();

  /* Open the log file. */
  init_log(log_file);

  /* Initialize memory. */
  init_mem();

  /* Initialize devices. */
  IFDEF(CONFIG_DEVICE, init_device());

  /* Perform ISA dependent initialization. */
  init_isa();

  /* Load the image to memory. This will overwrite the built-in image. */
  long img_size = load_img();

  /* load elf document*/
  IFDEF(CONFIG_FTRACE, load_elf());

  /* Initialize differential testing. */
  init_difftest(diff_so_file, img_size, difftest_port);

  /* Initialize the simple debugger. */
  init_sdb();

  IFDEF(CONFIG_ITRACE, init_disasm(
    MUXDEF(CONFIG_ISA_x86,     "i686",
    MUXDEF(CONFIG_ISA_mips32,  "mipsel",
    MUXDEF(CONFIG_ISA_riscv32, "riscv32",
    MUXDEF(CONFIG_ISA_riscv64, "riscv64", "bad")))) "-pc-linux-gnu"
  ));

  /* Display welcome message. */
  welcome();
}
#else // CONFIG_TARGET_AM
static long load_img() {
  extern char bin_start, bin_end;
  size_t size = &bin_end - &bin_start;
  Log("img size = %ld", size);
  memcpy(guest_to_host(RESET_VECTOR), &bin_start, size);
  return size;
}

void am_init_monitor() {
  init_rand();
  init_mem();
  init_isa();
  load_img();
  IFDEF(CONFIG_DEVICE, init_device());
  welcome();
}
#endif
