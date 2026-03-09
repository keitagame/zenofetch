/*
 * sysinfo - neofetch/fastfetch-style system info tool
 * Layout: ASCII logo LEFT, info panel RIGHT (side-by-side)
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include <dirent.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <ctype.h>
#include <glob.h>

/* ══════════════════════════════════════════════════════════
   ANSI helpers
   ══════════════════════════════════════════════════════════ */
#define RESET          "\033[0m"
#define BOLD           "\033[1m"
#define BRIGHT_BLACK   "\033[90m"
#define BRIGHT_RED     "\033[91m"
#define BRIGHT_GREEN   "\033[92m"
#define BRIGHT_YELLOW  "\033[93m"
#define BRIGHT_BLUE    "\033[94m"
#define BRIGHT_MAGENTA "\033[95m"
#define BRIGHT_CYAN    "\033[96m"
#define BRIGHT_WHITE   "\033[97m"
#define WHITE          "\033[37m"
#define YELLOW         "\033[33m"
#define RED            "\033[31m"
#define GREEN          "\033[32m"
#define BLUE           "\033[34m"

/* ══════════════════════════════════════════════════════════
   Visible-width of a string (strips ANSI escapes + handles
   basic UTF-8 CJK full-width characters)
   ══════════════════════════════════════════════════════════ */
static int visible_width(const char *s) {
    int w = 0;
    while (*s) {
        if (*s == '\033') {
            /* skip ESC [ ... m */
            s++;
            if (*s == '[') s++;
            while (*s && *s != 'm') s++;
            if (*s) s++;
            continue;
        }
        unsigned char c = (unsigned char)*s;
        if ((c & 0x80) == 0) {
            w++; s++;
        } else if ((c & 0xE0) == 0xC0) {
            /* 2-byte UTF-8 → width 1 */
            w++; s += 2;
        } else if ((c & 0xF0) == 0xE0) {
            /* 3-byte UTF-8 – CJK etc. → width 2 */
            w += 2; s += 3;
        } else if ((c & 0xF8) == 0xF0) {
            /* 4-byte UTF-8 → width 1 */
            w++; s += 4;
        } else {
            w++; s++;
        }
    }
    return w;
}

/* ══════════════════════════════════════════════════════════
   ASCII Art tables
   ══════════════════════════════════════════════════════════ */
typedef struct {
    const char **lines;
    int          count;
    int          width;   /* max visible width (computed at runtime) */
} Logo;

/* ── Ubuntu ── */
static const char *ubuntu_lines[] = {
    BRIGHT_RED "          .-/+oossssoo+/-.",
    BRIGHT_RED "      `:+ssssssssssssssssss+:`",
    BRIGHT_RED "    -+ssssssssssssssssssyyssss+-",
    BRIGHT_RED "  .ossssssssssssssssss" BRIGHT_WHITE "dMMMNy" BRIGHT_RED "sssso.",
    BRIGHT_RED " /sssssssssss" BRIGHT_WHITE "hdmmNNmmyNMMMMh" BRIGHT_RED "ssssss\\",
    BRIGHT_RED "+sssssssss" BRIGHT_WHITE "hm" BRIGHT_RED "yd" BRIGHT_WHITE "MMMMMMMNddddy" BRIGHT_RED "ssssssss+",
    BRIGHT_RED "/ssssssss" BRIGHT_WHITE "hNMMM" BRIGHT_RED "yh" BRIGHT_WHITE "hyyyyhmNMMMNh" BRIGHT_RED "ssssssss\\",
    BRIGHT_RED ".ssssssss" BRIGHT_WHITE "dMMMNh" BRIGHT_RED "ssssssssss" BRIGHT_WHITE "hNMMMd" BRIGHT_RED "ssssssss.",
    BRIGHT_RED "+ssss" BRIGHT_WHITE "hhhyNMMNy" BRIGHT_RED "ssssssssssss" BRIGHT_WHITE "yNMMMy" BRIGHT_RED "sssssss+",
    BRIGHT_RED "oss" BRIGHT_WHITE "yNMMMNyMMh" BRIGHT_RED "ssssssssssssss" BRIGHT_WHITE "hmmmh" BRIGHT_RED "ssssssso",
    BRIGHT_RED "oss" BRIGHT_WHITE "yNMMMNyMMh" BRIGHT_RED "ssssssssssssss" BRIGHT_WHITE "hmmmh" BRIGHT_RED "ssssssso",
    BRIGHT_RED "+ssss" BRIGHT_WHITE "hhhyNMMNy" BRIGHT_RED "ssssssssssss" BRIGHT_WHITE "yNMMMy" BRIGHT_RED "sssssss+",
    BRIGHT_RED ".ssssssss" BRIGHT_WHITE "dMMMNh" BRIGHT_RED "ssssssssss" BRIGHT_WHITE "hNMMMd" BRIGHT_RED "ssssssss.",
    BRIGHT_RED "/ssssssss" BRIGHT_WHITE "hNMMM" BRIGHT_RED "yh" BRIGHT_WHITE "hyyyyhmNMMMNh" BRIGHT_RED "ssssssss\\",
    BRIGHT_RED "+sssssssss" BRIGHT_WHITE "hm" BRIGHT_RED "yd" BRIGHT_WHITE "MMMMMMMNddddy" BRIGHT_RED "ssssssss+",
    BRIGHT_RED " \\sssssssssss" BRIGHT_WHITE "hdmmNNmmyNMMMMh" BRIGHT_RED "ssssss/",
    BRIGHT_RED "  .ossssssssssssssssss" BRIGHT_WHITE "dMMMNy" BRIGHT_RED "sssso.",
    BRIGHT_RED "    -+sssssssssssssssssss" BRIGHT_WHITE "yyy" BRIGHT_RED "ssss+-",
    BRIGHT_RED "      `:+ssssssssssssssssss+:`",
    BRIGHT_RED "          .-/+oossssoo+/-.",
};

/* ── Arch Linux ── */
static const char *arch_lines[] = {
    BRIGHT_CYAN "                  -`",
    BRIGHT_CYAN "                 .o+`",
    BRIGHT_CYAN "                `ooo/",
    BRIGHT_CYAN "               `+oooo:",
    BRIGHT_CYAN "              `+oooooo:",
    BRIGHT_CYAN "              -+oooooo+:",
    BRIGHT_CYAN "            `/:-:++oooo+:",
    BRIGHT_CYAN "           `/++++/+++++++:",
    BRIGHT_CYAN "          `/++++++++++++++:",
    BRIGHT_CYAN "         `/+++ooooooooooooo/`",
    BRIGHT_CYAN "        ./ooosssso++osssssso+`",
    BRIGHT_CYAN "       .oossssso-````/ossssss+`",
    BRIGHT_CYAN "      -osssssso.      :ssssssso.",
    BRIGHT_CYAN "     :osssssss/        osssso+++.",
    BRIGHT_CYAN "    /ossssssss/        +ssssooo/-",
    BRIGHT_CYAN "  `/ossssso+/:-        -:/+osssso+-",
    BRIGHT_CYAN " `+sso+:-`                 `.-/+oso:",
    BRIGHT_CYAN "`++:.                           `-/+/",
    BRIGHT_CYAN ".`                                 `/",
};

/* ── Debian ── */
static const char *debian_lines[] = {
    BRIGHT_RED "       _,met$$$$$gg.",
    BRIGHT_RED "    ,g$$$$$$$$$$$$$$$P.",
    BRIGHT_RED "  ,g$$P\"\"       \"\"\"Y$$.\",",
    BRIGHT_RED " ,$$P'              `$$.",
    BRIGHT_RED "',$$P       ,ggs.     `$$b:",
    BRIGHT_RED "`d$$'     ,$P\"'   .    $$$",
    BRIGHT_RED " $$P      d$'     ,    $$P",
    BRIGHT_RED " $$:      $$.   -    ,d$$'",
    BRIGHT_RED " $$\\;      Y$b._   _,d$P'",
    BRIGHT_RED " Y$$.    `.$`\"Y$$$$P\"'",
    BRIGHT_RED " `$$b      \"-.__",
    BRIGHT_RED "  `Y$$b",
    BRIGHT_RED "   `Y$$$b.",
    BRIGHT_RED "     `\"Y$$b.",
    BRIGHT_RED "        `\"\"Y$b._",
    BRIGHT_RED "            `\"\"\"\"",
};

/* ── Fedora ── */
static const char *fedora_lines[] = {
    BRIGHT_BLUE "          /:-------------:\\",
    BRIGHT_BLUE "       :-------------------:::",
    BRIGHT_BLUE "     :-----------" BRIGHT_WHITE "/shhOHbmp" BRIGHT_BLUE "---:\\",
    BRIGHT_BLUE "   /-----------" BRIGHT_WHITE "omMMMNNNMMD" BRIGHT_BLUE "  ---:",
    BRIGHT_BLUE "  :-----------" BRIGHT_WHITE "sMMMMNMNMP" BRIGHT_BLUE "        \\",
    BRIGHT_BLUE " :-----------" BRIGHT_WHITE ":MMMdP" BRIGHT_BLUE "-------    ---\\",
    BRIGHT_BLUE ",------------" BRIGHT_WHITE ":MMMd" BRIGHT_BLUE "--------    ---.",
    BRIGHT_BLUE ":------------" BRIGHT_WHITE ":MMMd" BRIGHT_BLUE "-------    .---:",
    BRIGHT_BLUE ":----    ----" BRIGHT_WHITE "/MMMN" BRIGHT_BLUE "-------    :---:",
    BRIGHT_BLUE ":--     -----" BRIGHT_WHITE "/MMMN" BRIGHT_BLUE "-------    :---:",
    BRIGHT_BLUE ":-       ----" BRIGHT_WHITE "/MMMN" BRIGHT_BLUE "----    -------:",
    BRIGHT_BLUE ":-----------" BRIGHT_WHITE ":MMMd" BRIGHT_BLUE "--------    ---.",
    BRIGHT_BLUE ":-----------" BRIGHT_WHITE ":MMMd" BRIGHT_BLUE "-------    ---.",
    BRIGHT_BLUE ":----------" BRIGHT_WHITE "sMMMMN" BRIGHT_BLUE "-------    ---:",
    BRIGHT_BLUE "          " BRIGHT_WHITE "sNMMMNmds" BRIGHT_BLUE "             ::",
    BRIGHT_BLUE "        YMMSamMN",
    BRIGHT_BLUE "         \"\"\"\"\"\"\"\"",
};

/* ── Manjaro ── */
static const char *manjaro_lines[] = {
    BRIGHT_GREEN "██████████████████  ████████",
    BRIGHT_GREEN "██████████████████  ████████",
    BRIGHT_GREEN "██████████████████  ████████",
    BRIGHT_GREEN "██████████████████  ████████",
    BRIGHT_GREEN "████████            ████████",
    BRIGHT_GREEN "████████  ████████  ████████",
    BRIGHT_GREEN "████████  ████████  ████████",
    BRIGHT_GREEN "████████  ████████  ████████",
    BRIGHT_GREEN "████████  ████████  ████████",
    BRIGHT_GREEN "████████  ████████  ████████",
    BRIGHT_GREEN "████████  ████████  ████████",
    BRIGHT_GREEN "████████  ████████  ████████",
    BRIGHT_GREEN "████████  ████████  ████████",
    BRIGHT_GREEN "████████  ████████  ████████",
};

/* ── openSUSE ── */
static const char *opensuse_lines[] = {
    BRIGHT_GREEN "           .;ldkO0000Okdl;.",
    BRIGHT_GREEN "       .;d00xl:^''''''^:ok00d;.",
    BRIGHT_GREEN "     .d00l'                'o00d.",
    BRIGHT_GREEN "   .d0Kd'  Okxol:;,.          :O0d.",
    BRIGHT_GREEN "  .OKKKK0kOKKKKKKKKKKKKKKKKKKOKOKKd.",
    BRIGHT_GREEN " ,0KKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK0o,",
    BRIGHT_GREEN ".kKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKd.",
    BRIGHT_GREEN ",OKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKo,",
    BRIGHT_GREEN ":KKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK:",
    BRIGHT_GREEN "dKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKd",
    BRIGHT_GREEN "dKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKd",
    BRIGHT_GREEN ":KKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK:",
    BRIGHT_GREEN " 'OKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKo'",
    BRIGHT_GREEN "  .kKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKd.",
    BRIGHT_GREEN "   ,0KKKKKKKKKKKKKKKKKKKKKKKKKKKKKK0,",
    BRIGHT_GREEN "    .dKKKKKKKKKKKKKKKKKKKKKKKKKKd.",
    BRIGHT_GREEN "      .d00l.                .o00d.",
    BRIGHT_GREEN "         .;d00xl:^':ok00d;.",
    BRIGHT_GREEN "              .;ldkO0Okdl;.",
};

/* ── Kali ── */
static const char *kali_lines[] = {
    BRIGHT_BLUE "...............",
    BRIGHT_BLUE "            ..,;:ccc,.",
    BRIGHT_BLUE "          ......''';lxO.",
    BRIGHT_BLUE ".....''''..........,:ld;",
    BRIGHT_BLUE "           .';;;:::;,,.x,",
    BRIGHT_BLUE "      ..'''.            0Xxoc:,.  ...",
    BRIGHT_BLUE "  ....                ,ONkc;,;cokOdc',.",
    BRIGHT_BLUE " .                   OMo           ':ddo.",
    BRIGHT_BLUE "                    dMc               :OO;",
    BRIGHT_BLUE "                    0M.                 .:o.",
    BRIGHT_BLUE "                    ;Wd",
    BRIGHT_BLUE "                     ;XO,",
    BRIGHT_BLUE "                       ,d0Odlc;,..",
    BRIGHT_BLUE "                          ..',;:cdOOd::,'.",
    BRIGHT_BLUE "                                   .:d;.''",
    BRIGHT_BLUE "                                       'OOo.",
    BRIGHT_BLUE "                                        ,0Oo.",
    BRIGHT_BLUE "                                         .o+.",
};

/* ── Raspbian ── */
static const char *raspbian_lines[] = {
    BRIGHT_RED "    .~~.   .~~.",
    BRIGHT_RED "   '. \\ ' ' / .'",
    BRIGHT_RED "    .~ .~~~..~.",
    BRIGHT_RED "   : .~.'~'.~. :",
    BRIGHT_RED "  ~ (   ) (   ) ~",
    BRIGHT_RED " ( : '~'.~.'~' : )",
    BRIGHT_RED "  ~ .~  (   )  ~. ~",
    BRIGHT_RED "   (   ) (   ) (   )",
    BRIGHT_RED "    '~'   '~'   '~'",
};

/* ── Generic Linux ── */
static const char *generic_lines[] = {
    BRIGHT_WHITE "        #####",
    BRIGHT_WHITE "       #######",
    BRIGHT_WHITE "       ##" BRIGHT_BLACK "O" BRIGHT_WHITE "#" BRIGHT_BLACK "O" BRIGHT_WHITE "##",
    BRIGHT_WHITE "       #" YELLOW "#####" BRIGHT_WHITE "#",
    BRIGHT_WHITE "     ##" BRIGHT_WHITE "#########" BRIGHT_WHITE "##",
    BRIGHT_WHITE "    #" BRIGHT_WHITE "###########" BRIGHT_WHITE "#",
    BRIGHT_WHITE "   #" BRIGHT_WHITE "#####" BRIGHT_BLACK "###" BRIGHT_WHITE "#####" BRIGHT_WHITE "#",
    BRIGHT_WHITE "   ##" BRIGHT_WHITE "####" BRIGHT_BLACK "#####" BRIGHT_WHITE "####" BRIGHT_WHITE "##",
    BRIGHT_WHITE "   ###" BRIGHT_WHITE "##" BRIGHT_BLACK "#######" BRIGHT_WHITE "##" BRIGHT_WHITE "###",
    BRIGHT_WHITE "   ####" BRIGHT_BLACK "#########" BRIGHT_WHITE "####",
    BRIGHT_WHITE "   #####" BRIGHT_BLACK "#######" BRIGHT_WHITE "#####",
    BRIGHT_WHITE "   ######" BRIGHT_BLACK "#####" BRIGHT_WHITE "######",
    BRIGHT_WHITE "   #######" BRIGHT_BLACK "###" BRIGHT_WHITE "#######",
    BRIGHT_WHITE "   ########" BRIGHT_BLACK "#" BRIGHT_WHITE "########",
    BRIGHT_WHITE "   #################",
    BRIGHT_WHITE "   #################",
};

/* ══════════════════════════════════════════════════════════
   Info line builder
   ══════════════════════════════════════════════════════════ */
#define MAX_INFO  80
#define ILEN     300

typedef struct { char lines[MAX_INFO][ILEN]; int n; } Info;

static void iadd(Info *I, const char *s) {
    if (I->n < MAX_INFO) snprintf(I->lines[I->n++], ILEN, "%s", s);
}
static void iblank(Info *I) { iadd(I, ""); }

static void isection(Info *I, const char *col, const char *title) {
    char b[ILEN];
    snprintf(b, sizeof(b), "%s%s%s%s", BOLD, col, title, RESET);
    iadd(I, b);
}

static void irow(Info *I,
                 const char *kcol, const char *key,
                 const char *vcol, const char *val) {
    if (!val || !val[0]) return;
    char b[ILEN];
    snprintf(b, sizeof(b), "%s%s%-15s%s%s%s",
             BOLD, kcol, key, RESET, vcol, val);
    iadd(I, b);
}

/* ══════════════════════════════════════════════════════════
   SysInfo struct
   ══════════════════════════════════════════════════════════ */
typedef struct {
    char username[64], hostname[256];
    char os_name[256], os_id[64];
    char kernel[256], uptime[64];
    char shell[128],  terminal[128];
    char cpu_model[256], cpu_cores[16], cpu_freq[32];
    char gpu[512];
    char mb_vendor[128], mb_name[128];
    char ram_total[32], ram_used[32], ram_type[32];
    char swap_total[32], swap_used[32];
    char disks[1024];
    char net_iface[64], net_ipv4[64], net_ipv6[128], net_speed[64];
    char datetime[64], timezone[64];
    char packages[64], de_wm[128], resolution[64];
    char battery[64];
} SysInfo;

/* ══════════════════════════════════════════════════════════
   Helpers
   ══════════════════════════════════════════════════════════ */
static void trim(char *s) {
    if (!s || !*s) return;
    while (isspace((unsigned char)*s)) memmove(s, s+1, strlen(s));
    char *e = s + strlen(s) - 1;
    while (e > s && isspace((unsigned char)*e)) *e-- = '\0';
}
static int rfl(const char *p, char *b, size_t n) {
    FILE *f = fopen(p,"r"); if (!f) return 0;
    int ok = fgets(b,(int)n,f)!=NULL; fclose(f);
    if (ok) { trim(b); } return ok && b[0];
}
static int rcmd(const char *c, char *b, size_t n) {
    FILE *f = popen(c,"r"); if (!f) return 0;
    int ok = fgets(b,(int)n,f)!=NULL; pclose(f);
    if (ok) { trim(b); } return ok && b[0];
}

/* ══════════════════════════════════════════════════════════
   Collectors
   ══════════════════════════════════════════════════════════ */
static void c_user(SysInfo *s) {
    const char *u = getenv("USER"); if (!u) u = getenv("LOGNAME");
    snprintf(s->username, 64, "%s", u?u:"user");
    gethostname(s->hostname, 256);
}

static void c_os(SysInfo *s) {
    FILE *f = fopen("/etc/os-release","r");
    if (!f) f = fopen("/usr/lib/os-release","r");
    s->os_name[0] = s->os_id[0] = '\0';
    if (f) {
        char ln[512];
        while (fgets(ln,512,f)) {
            if (!strncmp(ln,"PRETTY_NAME=",12)&&!s->os_name[0]) {
                char *v=ln+12; if(*v=='"')v++;
                snprintf(s->os_name,256,"%s",v);
                size_t l=strlen(s->os_name);
                if(l&&s->os_name[l-1]=='"') s->os_name[l-1]='\0';
                trim(s->os_name);
            }
            if (!strncmp(ln,"ID=",3)&&!s->os_id[0]) {
                char *v=ln+3; if(*v=='"')v++;
                snprintf(s->os_id,64,"%s",v);
                size_t l=strlen(s->os_id);
                if(l&&s->os_id[l-1]=='"') s->os_id[l-1]='\0';
                trim(s->os_id);
                for(int i=0;s->os_id[i];i++) s->os_id[i]=tolower((unsigned char)s->os_id[i]);
            }
        }
        fclose(f);
    }
    if (!s->os_name[0]) {
        struct utsname u; uname(&u);
        snprintf(s->os_name,256,"%s",u.sysname);
        snprintf(s->os_id,64,"linux");
    }
    char m[256]={0};
    if (rfl("/proc/device-tree/model",m,256)&&strstr(m,"Raspberry Pi"))
        snprintf(s->os_id,64,"raspbian");
}

static void c_kernel(SysInfo *s) {
    struct utsname u; uname(&u);
    snprintf(s->kernel,256,"%s %s",u.sysname,u.release);
}

static void c_uptime(SysInfo *s) {
    struct sysinfo si; sysinfo(&si);
    long up=si.uptime; int d=up/86400,h=(up%86400)/3600,m=(up%3600)/60;
    if(d)      snprintf(s->uptime,64,"%dd %dh %dm",d,h,m);
    else if(h) snprintf(s->uptime,64,"%dh %dm",h,m);
    else       snprintf(s->uptime,64,"%dm",m);
}

static void c_shell(SysInfo *s) {
    const char *sh=getenv("SHELL");
    if (!sh){snprintf(s->shell,128,"unknown");return;}
    const char *b=strrchr(sh,'/');
    snprintf(s->shell,128,"%s",b?b+1:sh);
}

static void c_terminal(SysInfo *s) {
    const char *t=getenv("TERM");
    snprintf(s->terminal,128,"%s",t?t:"unknown");
}

static void c_de(SysInfo *s) {
    const char *de=getenv("XDG_CURRENT_DESKTOP");
    const char *ss=getenv("DESKTOP_SESSION");
    const char *wl=getenv("WAYLAND_DISPLAY");
    if(de)      snprintf(s->de_wm,128,"%s (%s)",de,wl?"Wayland":"X11");
    else if(ss) snprintf(s->de_wm,128,"%s",ss);
    else        snprintf(s->de_wm,128,"N/A");
}

static void c_cpu(SysInfo *s) {
    FILE *f=fopen("/proc/cpuinfo","r");
    s->cpu_model[0]='\0'; int cores=0; double mhz=0;
    if(f){
        char ln[512];
        while(fgets(ln,512,f)){
            if(!strncmp(ln,"model name",10)&&!s->cpu_model[0]){
                char *c=strchr(ln,':');
                if(c){snprintf(s->cpu_model,256,"%s",c+2);trim(s->cpu_model);}
            }
            if(!strncmp(ln,"processor",9)) cores++;
            if(!strncmp(ln,"cpu MHz",7)){
                char *c=strchr(ln,':');
                if(c){double v=atof(c+2);if(v>mhz)mhz=v;}
            }
        }
        fclose(f);
    }
    snprintf(s->cpu_cores,16,"%d",cores);
    char fb[32]={0};
    if(rfl("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq",fb,32)){
        long hz=atol(fb); if(hz>0) mhz=hz/1000.0;
    }
    if(mhz>1000) snprintf(s->cpu_freq,32,"%.2f GHz",mhz/1000.0);
    else if(mhz>0) snprintf(s->cpu_freq,32,"%.0f MHz",mhz);
    if(!s->cpu_model[0]) snprintf(s->cpu_model,256,"Unknown CPU");
    char *p;
    while((p=strstr(s->cpu_model,"(R)")))  memmove(p,p+3,strlen(p+3)+1);
    while((p=strstr(s->cpu_model,"(TM)"))) memmove(p,p+4,strlen(p+4)+1);
    trim(s->cpu_model);
}

static void c_gpu(SysInfo *s) {
    s->gpu[0]='\0';
    rcmd("lspci 2>/dev/null | grep -i 'vga\\|3d\\|display' | sed 's/.*: //' | head -3 | tr '\\n' '|'",
         s->gpu, sizeof(s->gpu));
    if(!s->gpu[0]) snprintf(s->gpu,sizeof(s->gpu),"N/A");
    else { size_t l=strlen(s->gpu); if(l&&s->gpu[l-1]=='|') s->gpu[l-1]='\0'; }
    for(char *p=s->gpu;*p;p++) if(*p=='|') *p=';';
}

static void c_mb(SysInfo *s) {
    if(!rfl("/sys/class/dmi/id/board_vendor",s->mb_vendor,128))
        rfl("/sys/class/dmi/id/sys_vendor",s->mb_vendor,128);
    if(!rfl("/sys/class/dmi/id/board_name",s->mb_name,128))
        rfl("/sys/class/dmi/id/product_name",s->mb_name,128);
    if(!s->mb_vendor[0]) snprintf(s->mb_vendor,128,"Unknown");
    if(!s->mb_name[0])   snprintf(s->mb_name,128,"Unknown");
}

static void c_ram(SysInfo *s) {
    FILE *f=fopen("/proc/meminfo","r"); if(!f) return;
    long tot=0,free_=0,buf=0,cached=0,srec=0,stot=0,sfree=0;
    char ln[256];
    while(fgets(ln,256,f)){
        sscanf(ln,"MemTotal: %ld",&tot);
        sscanf(ln,"MemFree: %ld",&free_);
        sscanf(ln,"Buffers: %ld",&buf);
        sscanf(ln,"Cached: %ld",&cached);
        sscanf(ln,"SReclaimable: %ld",&srec);
        sscanf(ln,"SwapTotal: %ld",&stot);
        sscanf(ln,"SwapFree: %ld",&sfree);
    }
    fclose(f);
    long used=tot-free_-buf-cached-srec;
    if(used<0) used=tot-free_;
    #define KiB2GiB(x) ((x)/1024.0/1024.0)
    if(tot>1<<20) snprintf(s->ram_total,32,"%.1f GiB",KiB2GiB(tot));
    else          snprintf(s->ram_total,32,"%ld MiB",tot>>10);
    if(used>1<<20) snprintf(s->ram_used,32,"%.1f GiB",KiB2GiB(used));
    else           snprintf(s->ram_used,32,"%ld MiB",used>>10);
    if(stot>0){
        snprintf(s->swap_total,32,"%.1f GiB",KiB2GiB(stot));
        snprintf(s->swap_used,32,"%.1f GiB",KiB2GiB(stot-sfree));
    }
    char rt[32]={0};
    rcmd("dmidecode -t 17 2>/dev/null | grep -m1 'Type:' | grep -v Error | awk '{print $2}'",rt,32);
    if(rt[0]) snprintf(s->ram_type,32,"%s",rt);
}

static void c_disks(SysInfo *s) {
    s->disks[0]='\0';
    FILE *f=fopen("/proc/mounts","r"); if(!f) return;
    char ln[512], seen[32][64]; int nseen=0;
    while(fgets(ln,512,f)){
        char dev[256],mnt[256],fs[64],opts[256]; int a,b;
        if(sscanf(ln,"%255s %255s %63s %255s %d %d",dev,mnt,fs,opts,&a,&b)<4) continue;
        if(strncmp(dev,"/dev/",5)) continue;
        if(strstr(fs,"tmpfs")||strstr(fs,"devtmpfs")||strstr(fs,"proc")||
           strstr(fs,"sysfs")||strstr(fs,"cgroup")) continue;
        int dup=0;
        for(int i=0;i<nseen;i++) if(!strcmp(seen[i],dev)){dup=1;break;}
        if(dup||nseen>=32) continue;
        snprintf(seen[nseen++],64,"%s",dev);
        struct statvfs st; if(statvfs(mnt,&st)!=0) continue;
        unsigned long long tot=(unsigned long long)st.f_blocks*st.f_frsize;
        unsigned long long avl=(unsigned long long)st.f_bavail*st.f_frsize;
        unsigned long long usd=tot-avl;
        if(tot<1<<20) continue;
        const char *dn=strrchr(dev,'/'); dn=dn?dn+1:dev;
        char base[64]={0}; snprintf(base,64,"%s",dn);
        int dl=strlen(base);
        while(dl>0&&isdigit((unsigned char)base[dl-1])) base[--dl]='\0';
        if(dl>0&&base[dl-1]=='p'&&strstr(base,"nvme")) base[--dl]='\0';
        char model[128]={0},mp[256];
        snprintf(mp,256,"/sys/block/%s/device/model",base);
        if(!rfl(mp,model,128)) snprintf(model,128,"%s",base);
        trim(model);
        char entry[256];
        if(tot>=(unsigned long long)1<<30)
            snprintf(entry,256,"%s  [%s]  %.1f / %.1f GiB",
                dn,model,usd/1073741824.0,tot/1073741824.0);
        else
            snprintf(entry,256,"%s  [%s]  %llu / %llu MiB",
                dn,model,(unsigned long long)(usd>>20),(unsigned long long)(tot>>20));
        if(s->disks[0]) strncat(s->disks,"\n",sizeof(s->disks)-strlen(s->disks)-1);
        strncat(s->disks,entry,sizeof(s->disks)-strlen(s->disks)-1);
    }
    fclose(f);
    if(!s->disks[0]) snprintf(s->disks,sizeof(s->disks),"N/A");
}

static void c_net(SysInfo *s) {
    struct ifaddrs *ifa,*it;
    if(getifaddrs(&ifa)!=0) return;
    for(it=ifa;it;it=it->ifa_next){
        if(!it->ifa_addr) continue;
        if(it->ifa_flags&IFF_LOOPBACK) continue;
        if(!(it->ifa_flags&IFF_UP)) continue;
        if(it->ifa_addr->sa_family==AF_INET){
            struct sockaddr_in *sa=(struct sockaddr_in*)it->ifa_addr;
            inet_ntop(AF_INET,&sa->sin_addr,s->net_ipv4,64);
            snprintf(s->net_iface,64,"%s",it->ifa_name);
            break;
        }
    }
    if(s->net_iface[0]){
        for(it=ifa;it;it=it->ifa_next){
            if(!it->ifa_addr||strcmp(it->ifa_name,s->net_iface)) continue;
            if(it->ifa_addr->sa_family==AF_INET6){
                struct sockaddr_in6 *s6=(struct sockaddr_in6*)it->ifa_addr;
                inet_ntop(AF_INET6,&s6->sin6_addr,s->net_ipv6,128);
                break;
            }
        }
        char sp[256],sb[32]={0};
        snprintf(sp,256,"/sys/class/net/%s/speed",s->net_iface);
        if(rfl(sp,sb,32)){ int v=atoi(sb);
            if(v>=1000) snprintf(s->net_speed,64,"%d Gbps",v/1000);
            else if(v>0) snprintf(s->net_speed,64,"%d Mbps",v);
        }
        char wp[256]; snprintf(wp,256,"/sys/class/net/%s/wireless",s->net_iface);
        DIR *wd=opendir(wp);
        if(wd){
            closedir(wd);
            char cmd[256],ssid[64]={0};
            snprintf(cmd,256,"iw dev %s info 2>/dev/null | awk '/ssid/{print $2}'",s->net_iface);
            rcmd(cmd,ssid,64);
            char tag[80];
            if(ssid[0]) snprintf(tag,80," WiFi (%s)",ssid);
            else        snprintf(tag,80," WiFi");
            strncat(s->net_speed,tag,64-strlen(s->net_speed)-1);
        }
        if(!s->net_speed[0]) snprintf(s->net_speed,64,"N/A");
    } else {
        snprintf(s->net_iface,64,"none");
        snprintf(s->net_ipv4,64,"N/A");
        snprintf(s->net_speed,64,"Disconnected");
    }
    freeifaddrs(ifa);
}

static void c_time(SysInfo *s) {
    time_t now=time(NULL); struct tm *tm=localtime(&now);
    strftime(s->datetime,64,"%Y-%m-%d  %H:%M:%S",tm);
    if(!rfl("/etc/timezone",s->timezone,64)){
        char lnk[512]={0};
        ssize_t n=readlink("/etc/localtime",lnk,511);
        if(n>0){ lnk[n]='\0'; char *p=strstr(lnk,"zoneinfo/");
            snprintf(s->timezone,64,"%s",p?p+9:lnk); }
    }
    if(!s->timezone[0]) strftime(s->timezone,64,"%Z",tm);
}

static void c_pkgs(SysInfo *s) {
    char b[32]={0};
    if(rcmd("dpkg --get-selections 2>/dev/null | wc -l",b,32)&&atoi(b)>0)
        {snprintf(s->packages,64,"%d (dpkg)",atoi(b));return;}
    if(rcmd("rpm -qa 2>/dev/null | wc -l",b,32)&&atoi(b)>0)
        {snprintf(s->packages,64,"%d (rpm)",atoi(b));return;}
    if(rcmd("pacman -Q 2>/dev/null | wc -l",b,32)&&atoi(b)>0)
        {snprintf(s->packages,64,"%d (pacman)",atoi(b));return;}
    snprintf(s->packages,64,"N/A");
}

static void c_res(SysInfo *s) {
    if(!rcmd("xrandr 2>/dev/null | grep ' connected' | grep -o '[0-9]*x[0-9]*+0+0' | head -1",
             s->resolution,64))
        snprintf(s->resolution,64,"N/A");
}

static void c_bat(SysInfo *s) {
    glob_t g;
    if(glob("/sys/class/power_supply/BAT*/capacity",0,NULL,&g)==0&&g.gl_pathc>0){
        char cap[16]={0};
        if(rfl(g.gl_pathv[0],cap,16)){
            char sp[256]; strncpy(sp,g.gl_pathv[0],255);
            char *p=strstr(sp,"capacity"); if(p) memcpy(p,"status\0",7);
            char st[32]={0}; rfl(sp,st,32);
            snprintf(s->battery,64,"%s%% (%s)",cap,st[0]?st:"?");
        }
    }
    globfree(&g);
    if(!s->battery[0]) snprintf(s->battery,64,"N/A");
}

/* ══════════════════════════════════════════════════════════
   Logo selector
   ══════════════════════════════════════════════════════════ */
static Logo select_logo(const char *id) {
    Logo L={0};
    #define SET(a) do { L.lines=(a); L.count=(int)(sizeof(a)/sizeof*(a)); } while(0)
    if     (strstr(id,"ubuntu")||strstr(id,"mint")||strstr(id,"pop")) SET(ubuntu_lines);
    else if(strstr(id,"arch")||strstr(id,"endeavour")||strstr(id,"garuda")) SET(arch_lines);
    else if(strstr(id,"debian")) SET(debian_lines);
    else if(strstr(id,"fedora")) SET(fedora_lines);
    else if(strstr(id,"manjaro")) SET(manjaro_lines);
    else if(strstr(id,"opensuse")||strstr(id,"suse")) SET(opensuse_lines);
    else if(strstr(id,"kali"))   SET(kali_lines);
    else if(strstr(id,"raspbian")||strstr(id,"raspberry")) SET(raspbian_lines);
    else SET(generic_lines);
    #undef SET
    for(int i=0;i<L.count;i++){
        int w=visible_width(L.lines[i]);
        if(w>L.width) L.width=w;
    }
    return L;
}

/* ══════════════════════════════════════════════════════════
   Side-by-side renderer
   Prints logo on left, info on right, aligned column.
   ══════════════════════════════════════════════════════════ */
static void render(const Logo *L, const Info *I) {
    int rows = L->count > I->n ? L->count : I->n;
    int pad  = L->width + 4;   /* spaces between logo edge and info */

    for (int i = 0; i < rows; i++) {
        /* left: logo */
        if (i < L->count) {
            int vw = visible_width(L->lines[i]);
            fputs(L->lines[i], stdout);
            fputs(RESET, stdout);
            int sp = pad - vw;
            for (int p = 0; p < sp; p++) putchar(' ');
        } else {
            for (int p = 0; p < pad; p++) putchar(' ');
        }
        /* right: info */
        if (i < I->n) {
            fputs(I->lines[i], stdout);
            fputs(RESET, stdout);
        }
        putchar('\n');
    }
}

/* ══════════════════════════════════════════════════════════
   16-color palette bar
   ══════════════════════════════════════════════════════════ */
static void palette(int indent) {
    for (int i = 0; i < indent; i++) putchar(' ');
    static const char *c[] = {
        "\033[40m","\033[41m","\033[42m","\033[43m",
        "\033[44m","\033[45m","\033[46m","\033[47m",
        "\033[100m","\033[101m","\033[102m","\033[103m",
        "\033[104m","\033[105m","\033[106m","\033[107m",
    };
    for (int i = 0; i < 16; i++) printf("%s   " RESET, c[i]);
    putchar('\n');
}

/* ══════════════════════════════════════════════════════════
   MAIN
   ══════════════════════════════════════════════════════════ */
int main(void) {
    SysInfo si = {0};
    c_user(&si);  c_os(&si);    c_kernel(&si); c_uptime(&si);
    c_shell(&si); c_terminal(&si); c_de(&si);
    c_cpu(&si);   c_gpu(&si);   c_mb(&si);     c_ram(&si);
    c_disks(&si); c_net(&si);   c_time(&si);
    c_pkgs(&si);  c_res(&si);   c_bat(&si);

    /* ── Build info panel ── */
    Info info = {0};

    /* user@host */
    {
        char hdr[ILEN];
        snprintf(hdr, ILEN, BOLD BRIGHT_CYAN "%s" RESET BOLD "@" RESET BOLD BRIGHT_CYAN "%s" RESET,
                 si.username, si.hostname);
        iadd(&info, hdr);
        /* underline ─── same visible length */
        int len = (int)(strlen(si.username)+strlen(si.hostname)+1);
        char sep[ILEN]; int j=0;
        const char *dash = "─";   /* UTF-8 box char, width 1 */
        for (int i=0;i<len&&j<ILEN-8;i++) { memcpy(sep+j,dash,3); j+=3; }
        sep[j]='\0';
        iadd(&info, sep);
    }

    //iblank(&info);

    /* System */
    isection(&info, BRIGHT_CYAN, "System");
    irow(&info, BRIGHT_CYAN, " OS:",         BRIGHT_WHITE, si.os_name);
    irow(&info, BRIGHT_CYAN, " Kernel:",     BRIGHT_WHITE, si.kernel);
    irow(&info, BRIGHT_CYAN, " Uptime:",     BRIGHT_WHITE, si.uptime);
    irow(&info, BRIGHT_CYAN, " Packages:",   BRIGHT_WHITE, si.packages);
    irow(&info, BRIGHT_CYAN, " Shell:",      BRIGHT_WHITE, si.shell);
    if (strcmp(si.terminal,"unknown")!=0)
        irow(&info, BRIGHT_CYAN, " Terminal:",BRIGHT_WHITE, si.terminal);
    if (strcmp(si.de_wm,"N/A")!=0)
        irow(&info, BRIGHT_CYAN, " DE/WM:",   BRIGHT_WHITE, si.de_wm);
    if (strcmp(si.resolution,"N/A")!=0)
        irow(&info, BRIGHT_CYAN, " Res:",     BRIGHT_WHITE, si.resolution);

    //iblank(&info);

    /* Hardware */
    isection(&info, BRIGHT_YELLOW, "Hardware");

    char cpu_val[ILEN];
    if (si.cpu_freq[0])
        snprintf(cpu_val, ILEN, "%s  [%s × %s cores]", si.cpu_model, si.cpu_freq, si.cpu_cores);
    else
        snprintf(cpu_val, ILEN, "%s  [%s cores]", si.cpu_model, si.cpu_cores);
    irow(&info, BRIGHT_YELLOW, " CPU:", BRIGHT_WHITE, cpu_val);

    if (strcmp(si.gpu,"N/A")!=0) {
        char gc[512]; snprintf(gc,512,"%s",si.gpu);
        char *tok=strtok(gc,";"); int first=1;
        while(tok){ trim(tok);
            if(tok[0]) irow(&info, BRIGHT_YELLOW, first?" GPU:":"      ", BRIGHT_WHITE, tok);
            first=0; tok=strtok(NULL,";"); }
    }

    char mb[256];
    snprintf(mb,256,"%s  %s",si.mb_vendor,si.mb_name); trim(mb);
    irow(&info, BRIGHT_YELLOW, " Board:",    BRIGHT_WHITE, mb);

    char rv[128];
    if(si.ram_type[0])
        snprintf(rv,128,"%s / %s  (%s)",si.ram_used,si.ram_total,si.ram_type);
    else
        snprintf(rv,128,"%s / %s",si.ram_used,si.ram_total);
    irow(&info, BRIGHT_YELLOW, " RAM:",      BRIGHT_WHITE, rv);

    if(si.swap_total[0]){
        char sv[64]; snprintf(sv,64,"%s / %s",si.swap_used,si.swap_total);
        irow(&info, BRIGHT_YELLOW, " Swap:",  BRIGHT_WHITE, sv);
    }
    if(strcmp(si.battery,"N/A")!=0)
        irow(&info, BRIGHT_YELLOW, " Battery:", BRIGHT_WHITE, si.battery);

    //iblank(&info);

    /* Storage */
    isection(&info, BRIGHT_GREEN, "Storage");
    {
        char dc[1024]; snprintf(dc,1024,"%s",si.disks);
        char *ln=strtok(dc,"\n"); int first=1;
        while(ln){ trim(ln);
            if(ln[0]) irow(&info, BRIGHT_GREEN, first?" Disk:":"      ", BRIGHT_WHITE, ln);
            first=0; ln=strtok(NULL,"\n"); }
    }

    //iblank(&info);

    /* Network */
    //isection(&info, BRIGHT_MAGENTA, "Network");
    //irow(&info, BRIGHT_MAGENTA, " Interface:", BRIGHT_WHITE, si.net_iface);
   // irow(&info, BRIGHT_MAGENTA, " IPv4:",       BRIGHT_WHITE, si.net_ipv4);
   // if(si.net_ipv6[0]&&strcmp(si.net_ipv6,"::1")!=0)
   //     irow(&info, BRIGHT_MAGENTA, " IPv6:",   BRIGHT_WHITE, si.net_ipv6);
  //  irow(&info, BRIGHT_MAGENTA, " Speed:",      BRIGHT_WHITE, si.net_speed);

    //iblank(&info);

    /* Time */
    //isection(&info, BRIGHT_BLUE, "Time");
   // irow(&info, BRIGHT_BLUE, " Date/Time:", BRIGHT_WHITE, si.datetime);
   // irow(&info, BRIGHT_BLUE, " Timezone:",  BRIGHT_WHITE, si.timezone);

    //iblank(&info);

    /* ── Render ── */
    Logo logo = select_logo(si.os_id);

    putchar('\n');
    render(&logo, &info);
    palette(logo.width + 4);
    putchar('\n');

    return 0;
}