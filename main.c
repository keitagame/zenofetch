/*
 * sysinfo - A neofetch/fastfetch-like system information tool
 * Written in C, Linux-native
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

/* ─── ANSI Color codes ─── */
#define RESET       "\033[0m"
#define BOLD        "\033[1m"
#define DIM         "\033[2m"

#define BLACK       "\033[30m"
#define RED         "\033[31m"
#define GREEN       "\033[32m"
#define YELLOW      "\033[33m"
#define BLUE        "\033[34m"
#define MAGENTA     "\033[35m"
#define CYAN        "\033[36m"
#define WHITE       "\033[37m"

#define BRIGHT_BLACK   "\033[90m"
#define BRIGHT_RED     "\033[91m"
#define BRIGHT_GREEN   "\033[92m"
#define BRIGHT_YELLOW  "\033[93m"
#define BRIGHT_BLUE    "\033[94m"
#define BRIGHT_MAGENTA "\033[95m"
#define BRIGHT_CYAN    "\033[96m"
#define BRIGHT_WHITE   "\033[97m"

#define BG_RED      "\033[41m"
#define BG_GREEN    "\033[42m"
#define BG_YELLOW   "\033[43m"
#define BG_BLUE     "\033[44m"
#define BG_MAGENTA  "\033[45m"
#define BG_CYAN     "\033[46m"

/* ─── Distro ASCII Art definitions ─── */
typedef struct {
    const char *id;
    const char *color;
    const char **lines;
    int line_count;
} DistroArt;

static const char *ubuntu_art[] = {
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

static const char *arch_art[] = {
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

static const char *debian_art[] = {
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

static const char *fedora_art[] = {
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

static const char *manjaro_art[] = {
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

static const char *opensuse_art[] = {
    BRIGHT_GREEN "           .;ldkO0000Okdl;.",
    BRIGHT_GREEN "       .;d00xl:^''''''^:ok00d;.",
    BRIGHT_GREEN "     .d00l'                'o00d.",
    BRIGHT_GREEN "   .d0Kd'  Okxol:;,.          :O0d.",
    BRIGHT_GREEN "  .OK" BRIGHT_WHITE "KKK0kOKKKKKKKKKKKKKKKKKKOKOKKKd" BRIGHT_GREEN ".",
    BRIGHT_GREEN " ,0KKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK0o,",
    BRIGHT_GREEN ".kKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKd.",
    BRIGHT_GREEN ",OKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKO,",
    BRIGHT_GREEN ":KKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK:",
    BRIGHT_GREEN "dKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKd",
    BRIGHT_GREEN "dKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKd",
    BRIGHT_GREEN ":KKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK:",
    BRIGHT_GREEN "'OKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKO'",
    BRIGHT_GREEN " .kKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKd.",
    BRIGHT_GREEN "  ,0KKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK0,",
    BRIGHT_GREEN "   .dKKKKKKKKKKKKKKKKKKKKKKKKKKKKKd.",
    BRIGHT_GREEN "     .d00l.                    .o00d.",
    BRIGHT_GREEN "       .;d00xl:^'''^:ok00d;.",
    BRIGHT_GREEN "           .;ldkO0000Okdl;.",
};

static const char *generic_art[] = {
    BRIGHT_WHITE "        #####",
    BRIGHT_WHITE "       #######",
    BRIGHT_WHITE "       ##" BRIGHT_BLACK "O" BRIGHT_WHITE "#" BRIGHT_BLACK "O" BRIGHT_WHITE "##",
    BRIGHT_WHITE "       #" BRIGHT_YELLOW "#####" BRIGHT_WHITE "#",
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

static const char *raspbian_art[] = {
    BRIGHT_RED "  .~~.   .~~.",
    BRIGHT_RED " '. \\ ' ' / .'",
    BRIGHT_RED "  .~ .~~~..~.",
    BRIGHT_RED " : .~.'~'.~. :",
    BRIGHT_RED "~ (   ) (   ) ~",
    BRIGHT_RED "( : '~'.~.'~' : )",
    BRIGHT_RED " ~ .~  (   )  ~. ~",
    BRIGHT_RED "  (   ) (   ) (   )",
    BRIGHT_RED "   '~'   '~'   '~'",
};

static const char *kali_art[] = {
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
    BRIGHT_BLUE "                       ,d0Odlc;,..                 ..",
    BRIGHT_BLUE "                           ..',;:cdOOd::,'.         .",
    BRIGHT_BLUE "                                    .:d;.'' .c0OOOo",
    BRIGHT_BLUE "                                        'OOo. 'oOO;",
    BRIGHT_BLUE "                                         ,0Oo.",
    BRIGHT_BLUE "                                          .o+  .",
};

/* ─── Data structures ─── */
typedef struct {
    char username[64];
    char hostname[256];
    char os_name[256];
    char os_id[64];
    char kernel[256];
    char uptime[64];
    char shell[128];
    char terminal[128];
    char cpu_model[256];
    char cpu_cores[32];
    char cpu_freq[64];
    char gpu[512];
    char motherboard_vendor[128];
    char motherboard_name[128];
    char ram_total[32];
    char ram_used[32];
    char ram_type[64];
    char disks[1024];
    char net_interface[128];
    char net_ipv4[64];
    char net_ipv6[128];
    char net_speed[64];
    char datetime[64];
    char timezone[64];
    char locale[64];
    char packages[128];
    char de_wm[128];
    char resolution[128];
    char swap_total[32];
    char swap_used[32];
    char battery[64];
} SysInfo;

/* ─── Helper: trim whitespace ─── */
static void trim(char *s) {
    if (!s) return;
    char *end;
    while (isspace((unsigned char)*s)) memmove(s, s+1, strlen(s));
    end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) { *end = '\0'; end--; }
}

/* ─── Helper: read first line of file ─── */
static int read_first_line(const char *path, char *buf, size_t sz) {
    FILE *f = fopen(path, "r");
    if (!f) return 0;
    if (!fgets(buf, sz, f)) { fclose(f); return 0; }
    fclose(f);
    trim(buf);
    return 1;
}

/* ─── Helper: run command and capture output ─── */
static int run_cmd(const char *cmd, char *buf, size_t sz) {
    FILE *p = popen(cmd, "r");
    if (!p) return 0;
    if (!fgets(buf, sz, p)) { pclose(p); buf[0]='\0'; return 0; }
    pclose(p);
    trim(buf);
    return (buf[0] != '\0');
}

/* ─── OS Detection ─── */
static void get_os_info(SysInfo *si) {
    FILE *f = fopen("/etc/os-release", "r");
    if (!f) f = fopen("/usr/lib/os-release", "r");
    
    si->os_name[0] = '\0';
    si->os_id[0] = '\0';
    
    if (f) {
        char line[512];
        while (fgets(line, sizeof(line), f)) {
            if (strncmp(line, "PRETTY_NAME=", 12) == 0) {
                char *v = line + 12;
                if (*v == '"') v++;
                strncpy(si->os_name, v, sizeof(si->os_name)-1);
                size_t l = strlen(si->os_name);
                if (l > 0 && si->os_name[l-1] == '"') si->os_name[l-1] = '\0';
                trim(si->os_name);
            }
            if (strncmp(line, "ID=", 3) == 0) {
                char *v = line + 3;
                if (*v == '"') v++;
                strncpy(si->os_id, v, sizeof(si->os_id)-1);
                size_t l = strlen(si->os_id);
                if (l > 0 && si->os_id[l-1] == '"') si->os_id[l-1] = '\0';
                trim(si->os_id);
                /* lowercase */
                for (int i = 0; si->os_id[i]; i++) si->os_id[i] = tolower(si->os_id[i]);
            }
        }
        fclose(f);
    }
    
    if (si->os_name[0] == '\0') {
        struct utsname u;
        uname(&u);
        snprintf(si->os_name, sizeof(si->os_name), "%s", u.sysname);
        snprintf(si->os_id, sizeof(si->os_id), "linux");
    }
    
    /* Detect if running on Raspberry Pi */
    char model[256] = {0};
    if (read_first_line("/proc/device-tree/model", model, sizeof(model))) {
        if (strstr(model, "Raspberry Pi")) {
            snprintf(si->os_id, sizeof(si->os_id), "raspbian");
        }
    }
}

/* ─── Kernel ─── */
static void get_kernel(SysInfo *si) {
    struct utsname u;
    uname(&u);
    snprintf(si->kernel, sizeof(si->kernel), "%s %s", u.sysname, u.release);
}

/* ─── Uptime ─── */
static void get_uptime(SysInfo *si) {
    struct sysinfo s;
    sysinfo(&s);
    long up = s.uptime;
    int days = up / 86400;
    int hours = (up % 86400) / 3600;
    int mins = (up % 3600) / 60;
    if (days > 0)
        snprintf(si->uptime, sizeof(si->uptime), "%dd %dh %dm", days, hours, mins);
    else if (hours > 0)
        snprintf(si->uptime, sizeof(si->uptime), "%dh %dm", hours, mins);
    else
        snprintf(si->uptime, sizeof(si->uptime), "%dm", mins);
}

/* ─── Shell ─── */
static void get_shell(SysInfo *si) {
    const char *sh = getenv("SHELL");
    if (sh) {
        const char *base = strrchr(sh, '/');
        snprintf(si->shell, sizeof(si->shell), "%s", base ? base+1 : sh);
    } else {
        snprintf(si->shell, sizeof(si->shell), "unknown");
    }
}

/* ─── Terminal ─── */
static void get_terminal(SysInfo *si) {
    const char *t = getenv("TERM");
    if (t) snprintf(si->terminal, sizeof(si->terminal), "%s", t);
    else snprintf(si->terminal, sizeof(si->terminal), "unknown");
}

/* ─── DE/WM ─── */
static void get_de_wm(SysInfo *si) {
    const char *de = getenv("XDG_CURRENT_DESKTOP");
    const char *wm = getenv("DESKTOP_SESSION");
    const char *wayland = getenv("WAYLAND_DISPLAY");
    
    if (de && wm)
        snprintf(si->de_wm, sizeof(si->de_wm), "%s (%s)", de, wayland ? "Wayland" : "X11");
    else if (de)
        snprintf(si->de_wm, sizeof(si->de_wm), "%s", de);
    else if (wm)
        snprintf(si->de_wm, sizeof(si->de_wm), "%s", wm);
    else
        snprintf(si->de_wm, sizeof(si->de_wm), "N/A");
}

/* ─── CPU ─── */
static void get_cpu(SysInfo *si) {
    FILE *f = fopen("/proc/cpuinfo", "r");
    si->cpu_model[0] = '\0';
    si->cpu_cores[0] = '\0';
    si->cpu_freq[0] = '\0';
    
    int cores = 0;
    double max_freq = 0.0;
    
    if (f) {
        char line[512];
        while (fgets(line, sizeof(line), f)) {
            if (strncmp(line, "model name", 10) == 0 && si->cpu_model[0] == '\0') {
                char *colon = strchr(line, ':');
                if (colon) { 
                    strncpy(si->cpu_model, colon+2, sizeof(si->cpu_model)-1);
                    trim(si->cpu_model);
                }
            }
            if (strncmp(line, "processor", 9) == 0) cores++;
            if (strncmp(line, "cpu MHz", 7) == 0) {
                char *colon = strchr(line, ':');
                if (colon) {
                    double freq = atof(colon+2);
                    if (freq > max_freq) max_freq = freq;
                }
            }
        }
        fclose(f);
    }
    
    snprintf(si->cpu_cores, sizeof(si->cpu_cores), "%d", cores);
    
    /* Try to get max freq from cpufreq */
    char freq_buf[64] = {0};
    if (read_first_line("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq", freq_buf, sizeof(freq_buf))) {
        long hz = atol(freq_buf);
        if (hz > 0) max_freq = hz / 1000.0;
    }
    
    if (max_freq > 1000)
        snprintf(si->cpu_freq, sizeof(si->cpu_freq), "%.2f GHz", max_freq / 1000.0);
    else if (max_freq > 0)
        snprintf(si->cpu_freq, sizeof(si->cpu_freq), "%.0f MHz", max_freq);
    
    if (si->cpu_model[0] == '\0')
        snprintf(si->cpu_model, sizeof(si->cpu_model), "Unknown CPU");
    
    /* Shorten common CPU strings */
    char *p;
    if ((p = strstr(si->cpu_model, "(R)"))) memmove(p, p+3, strlen(p+3)+1);
    if ((p = strstr(si->cpu_model, "(TM)"))) memmove(p, p+4, strlen(p+4)+1);
    trim(si->cpu_model);
}

/* ─── GPU ─── */
static void get_gpu(SysInfo *si) {
    si->gpu[0] = '\0';
    
    /* Try lspci first */
    if (!run_cmd("lspci 2>/dev/null | grep -i 'vga\\|3d\\|display' | sed 's/.*: //' | head -3 | tr '\\n' '|'",
                 si->gpu, sizeof(si->gpu))) {
        /* Fallback: try /sys DRM */
        DIR *d = opendir("/sys/class/drm");
        if (d) {
            struct dirent *ent;
            while ((ent = readdir(d)) != NULL) {
                if (strncmp(ent->d_name, "card", 4) == 0 && !strchr(ent->d_name, '-')) {
                    char path[512];
                    snprintf(path, sizeof(path), "/sys/class/drm/%s/device/vendor", ent->d_name);
                    char vendor[16] = {0};
                    char device[16] = {0};
                    char vpath[512], dpath[512];
                    snprintf(vpath, sizeof(vpath), "/sys/class/drm/%s/device/vendor", ent->d_name);
                    snprintf(dpath, sizeof(dpath), "/sys/class/drm/%s/device/device", ent->d_name);
                    read_first_line(vpath, vendor, sizeof(vendor));
                    read_first_line(dpath, device, sizeof(device));
                    if (vendor[0]) {
                        char entry[128];
                        const char *vname = "Unknown GPU";
                        if (strstr(vendor, "1002")) vname = "AMD GPU";
                        else if (strstr(vendor, "10de")) vname = "NVIDIA GPU";
                        else if (strstr(vendor, "8086")) vname = "Intel GPU";
                        snprintf(entry, sizeof(entry), "%s (%s)|", vname, device);
                        strncat(si->gpu, entry, sizeof(si->gpu) - strlen(si->gpu) - 1);
                    }
                }
            }
            closedir(d);
        }
    }
    
    /* Remove trailing | */
    size_t l = strlen(si->gpu);
    if (l > 0 && si->gpu[l-1] == '|') si->gpu[l-1] = '\0';
    
    /* Replace | with newline marker */
    for (char *p = si->gpu; *p; p++) if (*p == '|') *p = ';';
    
    if (si->gpu[0] == '\0') snprintf(si->gpu, sizeof(si->gpu), "N/A");
}

/* ─── Motherboard ─── */
static void get_motherboard(SysInfo *si) {
    si->motherboard_vendor[0] = '\0';
    si->motherboard_name[0] = '\0';
    
    read_first_line("/sys/class/dmi/id/board_vendor", si->motherboard_vendor, sizeof(si->motherboard_vendor));
    read_first_line("/sys/class/dmi/id/board_name", si->motherboard_name, sizeof(si->motherboard_name));
    
    if (si->motherboard_vendor[0] == '\0') {
        /* Try product_name (for laptops/VMs) */
        char prod[128] = {0};
        read_first_line("/sys/class/dmi/id/product_name", prod, sizeof(prod));
        if (prod[0]) snprintf(si->motherboard_name, sizeof(si->motherboard_name), "%s", prod);
        
        char sys_vendor[128] = {0};
        read_first_line("/sys/class/dmi/id/sys_vendor", sys_vendor, sizeof(sys_vendor));
        if (sys_vendor[0]) snprintf(si->motherboard_vendor, sizeof(si->motherboard_vendor), "%s", sys_vendor);
    }
    
    if (si->motherboard_vendor[0] == '\0') snprintf(si->motherboard_vendor, sizeof(si->motherboard_vendor), "Unknown");
    if (si->motherboard_name[0] == '\0') snprintf(si->motherboard_name, sizeof(si->motherboard_name), "Unknown");
}

/* ─── RAM ─── */
static void get_ram(SysInfo *si) {
    si->ram_total[0] = '\0';
    si->ram_used[0] = '\0';
    si->ram_type[0] = '\0';
    si->swap_total[0] = '\0';
    si->swap_used[0] = '\0';
    
    FILE *f = fopen("/proc/meminfo", "r");
    if (!f) return;
    
    long mem_total = 0, mem_free = 0, mem_buffers = 0, mem_cached = 0, mem_sreclaimable = 0;
    long swap_total = 0, swap_free = 0;
    char line[256];
    
    while (fgets(line, sizeof(line), f)) {
        if      (sscanf(line, "MemTotal: %ld", &mem_total)) {}
        else if (sscanf(line, "MemFree: %ld", &mem_free)) {}
        else if (sscanf(line, "Buffers: %ld", &mem_buffers)) {}
        else if (sscanf(line, "Cached: %ld", &mem_cached)) {}
        else if (sscanf(line, "SReclaimable: %ld", &mem_sreclaimable)) {}
        else if (sscanf(line, "SwapTotal: %ld", &swap_total)) {}
        else if (sscanf(line, "SwapFree: %ld", &swap_free)) {}
    }
    fclose(f);
    
    long used = mem_total - mem_free - mem_buffers - mem_cached - mem_sreclaimable;
    if (used < 0) used = mem_total - mem_free;
    
    if (mem_total > 1024*1024)
        snprintf(si->ram_total, sizeof(si->ram_total), "%.1f GiB", mem_total / 1024.0 / 1024.0);
    else
        snprintf(si->ram_total, sizeof(si->ram_total), "%ld MiB", mem_total / 1024);
    
    if (used > 1024*1024)
        snprintf(si->ram_used, sizeof(si->ram_used), "%.1f GiB", used / 1024.0 / 1024.0);
    else
        snprintf(si->ram_used, sizeof(si->ram_used), "%ld MiB", used / 1024);
    
    if (swap_total > 0) {
        long swap_used = swap_total - swap_free;
        snprintf(si->swap_total, sizeof(si->swap_total), "%.1f GiB", swap_total / 1024.0 / 1024.0);
        snprintf(si->swap_used, sizeof(si->swap_used), "%.1f GiB", swap_used / 1024.0 / 1024.0);
    }
    
    /* Try to get RAM type from dmidecode or /sys */
    char ram_type_buf[64] = {0};
    if (run_cmd("dmidecode -t 17 2>/dev/null | grep -m1 'Type:' | grep -v 'Error' | awk '{print $2}'",
                ram_type_buf, sizeof(ram_type_buf))) {
        snprintf(si->ram_type, sizeof(si->ram_type), "%s", ram_type_buf);
    }
}

/* ─── Disk ─── */
static void get_disks(SysInfo *si) {
    si->disks[0] = '\0';
    
    FILE *f = fopen("/proc/mounts", "r");
    if (!f) return;
    
    char line[512];
    char processed[32][64];
    int nproc = 0;
    
    while (fgets(line, sizeof(line), f)) {
        char dev[256], mnt[256], fs[64], opts[256];
        int dump, pass;
        
        if (sscanf(line, "%255s %255s %63s %255s %d %d", dev, mnt, fs, opts, &dump, &pass) < 4) continue;
        
        /* Only real filesystems */
        if (strncmp(dev, "/dev/", 5) != 0) continue;
        if (strstr(fs, "tmpfs") || strstr(fs, "devtmpfs") || strstr(fs, "proc") || 
            strstr(fs, "sysfs") || strstr(fs, "cgroup")) continue;
        
        /* Avoid duplicates */
        int dup = 0;
        for (int i = 0; i < nproc; i++) if (strcmp(processed[i], dev) == 0) { dup = 1; break; }
        if (dup || nproc >= 32) continue;
        strncpy(processed[nproc++], dev, 63);
        
        struct statvfs st;
        if (statvfs(mnt, &st) != 0) continue;
        
        unsigned long long total = (unsigned long long)st.f_blocks * st.f_frsize;
        unsigned long long avail = (unsigned long long)st.f_bavail * st.f_frsize;
        unsigned long long used  = total - avail;
        
        if (total < 1024*1024) continue; /* skip tiny */
        
        char entry[256];
        const char *devname = strrchr(dev, '/');
        devname = devname ? devname+1 : dev;
        
        char model[256] = {0};
        char model_path[256];
        /* Try block device model */
        /* Strip partition number to get disk name */
        char disk_base[64] = {0};
        strncpy(disk_base, devname, sizeof(disk_base)-1);
        /* remove trailing digits for partition */
        int dl = strlen(disk_base);
        while (dl > 0 && isdigit((unsigned char)disk_base[dl-1])) { disk_base[--dl] = '\0'; }
        /* for nvme: strip trailing 'p' */
        if (dl > 0 && disk_base[dl-1] == 'p' && strstr(disk_base, "nvme")) disk_base[--dl] = '\0';
        
        snprintf(model_path, sizeof(model_path), "/sys/block/%s/device/model", disk_base);
        read_first_line(model_path, model, sizeof(model));
        if (model[0] == '\0') snprintf(model, sizeof(model), "%s", disk_base);
        trim(model);
        
        if (total >= (unsigned long long)1024*1024*1024)
            snprintf(entry, sizeof(entry), "%s [%s] %.1f/%.1f GiB (%s)",
                     devname, model,
                     used / 1024.0 / 1024.0 / 1024.0,
                     total / 1024.0 / 1024.0 / 1024.0,
                     mnt);
        else
            snprintf(entry, sizeof(entry), "%s [%s] %llu/%llu MiB (%s)",
                     devname, model,
                     (unsigned long long)(used / 1024 / 1024),
                     (unsigned long long)(total / 1024 / 1024),
                     mnt);
        
        if (si->disks[0] != '\0') strncat(si->disks, "\n", sizeof(si->disks)-strlen(si->disks)-1);
        strncat(si->disks, entry, sizeof(si->disks)-strlen(si->disks)-1);
    }
    fclose(f);
    
    if (si->disks[0] == '\0') snprintf(si->disks, sizeof(si->disks), "N/A");
}

/* ─── Network ─── */
static void get_network(SysInfo *si) {
    si->net_interface[0] = '\0';
    si->net_ipv4[0] = '\0';
    si->net_ipv6[0] = '\0';
    si->net_speed[0] = '\0';
    
    struct ifaddrs *ifaddr, *ifa;
    if (getifaddrs(&ifaddr) == -1) return;
    
    /* Find first non-loopback UP interface with IPv4 */
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) continue;
        if (ifa->ifa_flags & IFF_LOOPBACK) continue;
        if (!(ifa->ifa_flags & IFF_UP)) continue;
        
        if (ifa->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in *sa = (struct sockaddr_in *)ifa->ifa_addr;
            inet_ntop(AF_INET, &sa->sin_addr, si->net_ipv4, sizeof(si->net_ipv4));
            strncpy(si->net_interface, ifa->ifa_name, sizeof(si->net_interface)-1);
            break;
        }
    }
    
    /* Find IPv6 for same interface */
    if (si->net_interface[0]) {
        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
            if (!ifa->ifa_addr) continue;
            if (strcmp(ifa->ifa_name, si->net_interface) != 0) continue;
            if (ifa->ifa_addr->sa_family == AF_INET6) {
                struct sockaddr_in6 *sa6 = (struct sockaddr_in6 *)ifa->ifa_addr;
                inet_ntop(AF_INET6, &sa6->sin6_addr, si->net_ipv6, sizeof(si->net_ipv6));
                break;
            }
        }
    }
    
    freeifaddrs(ifaddr);
    
    /* Network speed from sysfs */
    if (si->net_interface[0]) {
        char speed_path[256];
        snprintf(speed_path, sizeof(speed_path), "/sys/class/net/%s/speed", si->net_interface);
        char speed_buf[32] = {0};
        if (read_first_line(speed_path, speed_buf, sizeof(speed_buf))) {
            int sp = atoi(speed_buf);
            if (sp > 0) {
                if (sp >= 1000) snprintf(si->net_speed, sizeof(si->net_speed), "%d Gbps", sp/1000);
                else snprintf(si->net_speed, sizeof(si->net_speed), "%d Mbps", sp);
            }
        }
        
        /* Try wireless info */
        char wireless_path[256];
        snprintf(wireless_path, sizeof(wireless_path), "/sys/class/net/%s/wireless", si->net_interface);
        DIR *wd = opendir(wireless_path);
        if (wd) {
            closedir(wd);
            char ssid_buf[128] = {0};
            char cmd[256];
            snprintf(cmd, sizeof(cmd), "iw dev %s info 2>/dev/null | grep ssid | awk '{print $2}'", si->net_interface);
            if (run_cmd(cmd, ssid_buf, sizeof(ssid_buf))) {
                strncat(si->net_speed, " WiFi (", sizeof(si->net_speed)-strlen(si->net_speed)-1);
                strncat(si->net_speed, ssid_buf, sizeof(si->net_speed)-strlen(si->net_speed)-1);
                strncat(si->net_speed, ")", sizeof(si->net_speed)-strlen(si->net_speed)-1);
            } else {
                strncat(si->net_speed, " WiFi", sizeof(si->net_speed)-strlen(si->net_speed)-1);
            }
        }
        
        if (si->net_speed[0] == '\0') snprintf(si->net_speed, sizeof(si->net_speed), "N/A");
    } else {
        snprintf(si->net_interface, sizeof(si->net_interface), "none");
        snprintf(si->net_ipv4, sizeof(si->net_ipv4), "N/A");
        snprintf(si->net_speed, sizeof(si->net_speed), "Disconnected");
    }
}

/* ─── Date/Time ─── */
static void get_datetime(SysInfo *si) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    
    strftime(si->datetime, sizeof(si->datetime), "%Y-%m-%d %H:%M:%S", tm_info);
    
    const char *tz = getenv("TZ");
    if (!tz) {
        read_first_line("/etc/timezone", si->timezone, sizeof(si->timezone));
        if (si->timezone[0] == '\0') {
            char tzbuf[128] = {0};
            char tzlink[512] = {0};
            ssize_t len = readlink("/etc/localtime", tzlink, sizeof(tzlink)-1);
            if (len > 0) {
                tzlink[len] = '\0';
                char *p = strstr(tzlink, "zoneinfo/");
                if (p) snprintf(si->timezone, sizeof(si->timezone), "%s", p+9);
                else snprintf(si->timezone, sizeof(si->timezone), "%s", tzlink);
            }
        }
    } else {
        snprintf(si->timezone, sizeof(si->timezone), "%s", tz);
    }
    if (si->timezone[0] == '\0') strftime(si->timezone, sizeof(si->timezone), "%Z", tm_info);
}

/* ─── Packages ─── */
static void get_packages(SysInfo *si) {
    si->packages[0] = '\0';
    
    char buf[64] = {0};
    if (run_cmd("dpkg --get-selections 2>/dev/null | wc -l", buf, sizeof(buf))) {
        int n = atoi(buf);
        if (n > 0) { snprintf(si->packages, sizeof(si->packages), "%d (dpkg)", n); return; }
    }
    if (run_cmd("rpm -qa 2>/dev/null | wc -l", buf, sizeof(buf))) {
        int n = atoi(buf);
        if (n > 0) { snprintf(si->packages, sizeof(si->packages), "%d (rpm)", n); return; }
    }
    if (run_cmd("pacman -Q 2>/dev/null | wc -l", buf, sizeof(buf))) {
        int n = atoi(buf);
        if (n > 0) { snprintf(si->packages, sizeof(si->packages), "%d (pacman)", n); return; }
    }
    if (run_cmd("apk info 2>/dev/null | wc -l", buf, sizeof(buf))) {
        int n = atoi(buf);
        if (n > 0) { snprintf(si->packages, sizeof(si->packages), "%d (apk)", n); return; }
    }
    if (si->packages[0] == '\0') snprintf(si->packages, sizeof(si->packages), "N/A");
}

/* ─── Resolution ─── */
static void get_resolution(SysInfo *si) {
    si->resolution[0] = '\0';
    if (!run_cmd("xrandr 2>/dev/null | grep ' connected' | grep -o '[0-9]*x[0-9]*+0+0' | head -1",
                 si->resolution, sizeof(si->resolution))) {
        /* Wayland */
        if (!run_cmd("wlr-randr 2>/dev/null | grep -o '[0-9]*x[0-9]*' | head -1",
                     si->resolution, sizeof(si->resolution))) {
            snprintf(si->resolution, sizeof(si->resolution), "N/A");
        }
    }
}

/* ─── Battery ─── */
static void get_battery(SysInfo *si) {
    si->battery[0] = '\0';
    
    glob_t g;
    if (glob("/sys/class/power_supply/BAT*/capacity", 0, NULL, &g) == 0 && g.gl_pathc > 0) {
        char cap[16] = {0};
        if (read_first_line(g.gl_pathv[0], cap, sizeof(cap))) {
            char status_path[256];
            strncpy(status_path, g.gl_pathv[0], sizeof(status_path)-1);
            char *p = strstr(status_path, "capacity");
            if (p) { memcpy(p, "status\0", 7); }
            char status[32] = {0};
            read_first_line(status_path, status, sizeof(status));
            snprintf(si->battery, sizeof(si->battery), "%s%% (%s)", cap, status[0] ? status : "Unknown");
        }
    }
    globfree(&g);
    
    if (si->battery[0] == '\0') snprintf(si->battery, sizeof(si->battery), "N/A");
}

/* ─── Username / Hostname ─── */
static void get_user_host(SysInfo *si) {
    const char *user = getenv("USER");
    if (!user) user = getenv("LOGNAME");
    if (user) strncpy(si->username, user, sizeof(si->username)-1);
    else snprintf(si->username, sizeof(si->username), "user");
    
    gethostname(si->hostname, sizeof(si->hostname));
}

/* ─── Art selection ─── */
static void print_distro_logo(const char *os_id, int *nlines_out) {
    const char **lines = NULL;
    int count = 0;
    const char *id = os_id;
    
    if (strstr(id, "ubuntu") || strstr(id, "mint") || strstr(id, "pop")) {
        lines = ubuntu_art; count = sizeof(ubuntu_art)/sizeof(*ubuntu_art);
    } else if (strstr(id, "arch") || strstr(id, "endeavour") || strstr(id, "garuda")) {
        lines = arch_art; count = sizeof(arch_art)/sizeof(*arch_art);
    } else if (strstr(id, "debian")) {
        lines = debian_art; count = sizeof(debian_art)/sizeof(*debian_art);
    } else if (strstr(id, "fedora")) {
        lines = fedora_art; count = sizeof(fedora_art)/sizeof(*fedora_art);
    } else if (strstr(id, "manjaro")) {
        lines = manjaro_art; count = sizeof(manjaro_art)/sizeof(*manjaro_art);
    } else if (strstr(id, "opensuse") || strstr(id, "suse")) {
        lines = opensuse_art; count = sizeof(opensuse_art)/sizeof(*opensuse_art);
    } else if (strstr(id, "raspbian") || strstr(id, "raspberry")) {
        lines = raspbian_art; count = sizeof(raspbian_art)/sizeof(*raspbian_art);
    } else if (strstr(id, "kali")) {
        lines = kali_art; count = sizeof(kali_art)/sizeof(*kali_art);
    } else {
        lines = generic_art; count = sizeof(generic_art)/sizeof(*generic_art);
    }
    
    for (int i = 0; i < count; i++) {
        printf("%s" RESET "\n", lines[i]);
    }
    if (nlines_out) *nlines_out = count;
}

/* ─── Color bar ─── */
static void print_color_bar(void) {
    printf("\n  ");
    const char *colors[] = {
        BG_RED, BG_GREEN, BG_YELLOW, BG_BLUE, BG_MAGENTA, BG_CYAN,
        "\033[0;40m", "\033[0;41m", "\033[0;42m", "\033[0;43m",
        "\033[0;44m", "\033[0;45m", "\033[0;46m", "\033[0;47m",
    };
    for (int i = 0; i < 14; i++) printf("%s   " RESET, colors[i]);
    printf("\n");
}

/* ─── Field printer ─── */
#define LABEL_W 20
static void print_field(const char *color, const char *label, const char *value) {
    if (!value || value[0] == '\0') return;
    printf("  %s%-*s" RESET "%s\n", color, LABEL_W, label, value);
}

static void print_separator(const char *color) {
    printf("  %s", color);
    for (int i = 0; i < 55; i++) printf("─");
    printf(RESET "\n");
}

/* ─── Main ─── */
int main(void) {
    SysInfo si = {0};
    
    /* Gather info */
    get_user_host(&si);
    get_os_info(&si);
    get_kernel(&si);
    get_uptime(&si);
    get_shell(&si);
    get_terminal(&si);
    get_de_wm(&si);
    get_cpu(&si);
    get_gpu(&si);
    get_motherboard(&si);
    get_ram(&si);
    get_disks(&si);
    get_network(&si);
    get_datetime(&si);
    get_packages(&si);
    get_resolution(&si);
    get_battery(&si);
    
    /* ─── Output ─── */
    printf("\n");
    
    /* Header: user@host */
    printf("  " BOLD BRIGHT_CYAN "%s" RESET BOLD "@" BOLD BRIGHT_CYAN "%s" RESET "\n", 
           si.username, si.hostname);
    
    /* Separator */
    printf("  " BRIGHT_BLACK);
    for (int i = 0; i < (int)(strlen(si.username) + strlen(si.hostname) + 1); i++) printf("─");
    printf(RESET "\n");
    
    /* Logo */
    print_distro_logo(si.os_id, NULL);
    printf("\n");
    
    /* ─── System ─── */
    print_separator(BRIGHT_BLACK);
    printf("  " BOLD BRIGHT_WHITE "  System" RESET "\n");
    print_separator(BRIGHT_BLACK);
    
    char os_line[512];
    snprintf(os_line, sizeof(os_line), "%s", si.os_name);
    print_field(BOLD BRIGHT_CYAN, " OS:", os_line);
    print_field(BOLD BRIGHT_CYAN, " Kernel:", si.kernel);
    print_field(BOLD BRIGHT_CYAN, " Uptime:", si.uptime);
    print_field(BOLD BRIGHT_CYAN, " Packages:", si.packages);
    print_field(BOLD BRIGHT_CYAN, " Shell:", si.shell);
    print_field(BOLD BRIGHT_CYAN, " Terminal:", si.terminal);
    if (strcmp(si.de_wm, "N/A") != 0)
        print_field(BOLD BRIGHT_CYAN, " DE/WM:", si.de_wm);
    if (strcmp(si.resolution, "N/A") != 0)
        print_field(BOLD BRIGHT_CYAN, " Resolution:", si.resolution);
    
    /* ─── Hardware ─── */
    printf("\n");
    print_separator(BRIGHT_BLACK);
    printf("  " BOLD BRIGHT_WHITE "  Hardware" RESET "\n");
    print_separator(BRIGHT_BLACK);
    
    /* CPU */
    char cpu_line[512];
    if (si.cpu_freq[0])
        snprintf(cpu_line, sizeof(cpu_line), "%s (%s cores @ %s)", si.cpu_model, si.cpu_cores, si.cpu_freq);
    else
        snprintf(cpu_line, sizeof(cpu_line), "%s (%s cores)", si.cpu_model, si.cpu_cores);
    print_field(BOLD BRIGHT_YELLOW, " CPU:", cpu_line);
    
    /* GPU (may be multiple) */
    if (strcmp(si.gpu, "N/A") != 0) {
        char gpu_copy[512];
        strncpy(gpu_copy, si.gpu, sizeof(gpu_copy)-1);
        char *token = strtok(gpu_copy, ";");
        int first = 1;
        while (token) {
            trim(token);
            if (token[0]) {
                if (first) print_field(BOLD BRIGHT_YELLOW, " GPU:", token);
                else        print_field(BOLD BRIGHT_YELLOW, "     ", token);
                first = 0;
            }
            token = strtok(NULL, ";");
        }
    }
    
    /* Motherboard */
    char mb_line[256];
    snprintf(mb_line, sizeof(mb_line), "%s %s", si.motherboard_vendor, si.motherboard_name);
    trim(mb_line);
    print_field(BOLD BRIGHT_YELLOW, " Motherboard:", mb_line);
    
    /* RAM */
    char ram_line[128];
    if (si.ram_type[0] && strcmp(si.ram_type, "Unknown") != 0)
        snprintf(ram_line, sizeof(ram_line), "%s used / %s total (%s)", si.ram_used, si.ram_total, si.ram_type);
    else
        snprintf(ram_line, sizeof(ram_line), "%s used / %s total", si.ram_used, si.ram_total);
    print_field(BOLD BRIGHT_YELLOW, " RAM:", ram_line);
    
    /* Swap */
    if (si.swap_total[0]) {
        char swap_line[128];
        snprintf(swap_line, sizeof(swap_line), "%s used / %s total", si.swap_used, si.swap_total);
        print_field(BOLD BRIGHT_YELLOW, " Swap:", swap_line);
    }
    
    /* Battery */
    if (strcmp(si.battery, "N/A") != 0)
        print_field(BOLD BRIGHT_YELLOW, " Battery:", si.battery);
    
    /* ─── Storage ─── */
    printf("\n");
    print_separator(BRIGHT_BLACK);
    printf("  " BOLD BRIGHT_WHITE "  Storage" RESET "\n");
    print_separator(BRIGHT_BLACK);
    
    {
        char disks_copy[1024];
        strncpy(disks_copy, si.disks, sizeof(disks_copy)-1);
        char *line = strtok(disks_copy, "\n");
        int first = 1;
        while (line) {
            trim(line);
            if (line[0]) {
                if (first) print_field(BOLD BRIGHT_GREEN, " Disk:", line);
                else        print_field(BOLD BRIGHT_GREEN, "      ", line);
                first = 0;
            }
            line = strtok(NULL, "\n");
        }
    }
    
    /* ─── Network ─── */
    printf("\n");
    print_separator(BRIGHT_BLACK);
    printf("  " BOLD BRIGHT_WHITE "  Network" RESET "\n");
    print_separator(BRIGHT_BLACK);
    
    print_field(BOLD BRIGHT_MAGENTA, " Interface:", si.net_interface);
    print_field(BOLD BRIGHT_MAGENTA, " IPv4:", si.net_ipv4);
    if (si.net_ipv6[0] && strcmp(si.net_ipv6, "::1") != 0)
        print_field(BOLD BRIGHT_MAGENTA, " IPv6:", si.net_ipv6);
    print_field(BOLD BRIGHT_MAGENTA, " Speed:", si.net_speed);
    
    /* ─── Time ─── */
    printf("\n");
    print_separator(BRIGHT_BLACK);
    printf("  " BOLD BRIGHT_WHITE "  Time & Locale" RESET "\n");
    print_separator(BRIGHT_BLACK);
    
    print_field(BOLD BRIGHT_BLUE, " Date/Time:", si.datetime);
    print_field(BOLD BRIGHT_BLUE, " Timezone:", si.timezone);
    
    /* Color palette */
    print_color_bar();
    printf("\n");
    
    return 0;
}
