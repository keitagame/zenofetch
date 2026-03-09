/*
 * cfetch - A neofetch/fastfetch-like system information tool written in C
 * Displays system info alongside an ASCII art logo
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include <pwd.h>
#include <time.h>

/* ANSI Color Codes */
#define RESET       "\033[0m"
#define BOLD        "\033[1m"
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

/* BG Colors for color blocks */
#define BG_BLACK   "\033[40m"
#define BG_RED     "\033[41m"
#define BG_GREEN   "\033[42m"
#define BG_YELLOW  "\033[43m"
#define BG_BLUE    "\033[44m"
#define BG_MAGENTA "\033[45m"
#define BG_CYAN    "\033[46m"
#define BG_WHITE   "\033[47m"
#define BG_BRIGHT_BLACK   "\033[100m"
#define BG_BRIGHT_RED     "\033[101m"
#define BG_BRIGHT_GREEN   "\033[102m"
#define BG_BRIGHT_YELLOW  "\033[103m"
#define BG_BRIGHT_BLUE    "\033[104m"
#define BG_BRIGHT_MAGENTA "\033[105m"
#define BG_BRIGHT_CYAN    "\033[106m"
#define BG_BRIGHT_WHITE   "\033[107m"

#define MAX_INFO_LEN 256
#define MAX_LINES    20

typedef struct {
    char user[MAX_INFO_LEN];
    char hostname[MAX_INFO_LEN];
    char os[MAX_INFO_LEN];
    char kernel[MAX_INFO_LEN];
    char uptime[MAX_INFO_LEN];
    char shell[MAX_INFO_LEN];
    char cpu[MAX_INFO_LEN];
    char memory[MAX_INFO_LEN];
    char disk[MAX_INFO_LEN];
    char arch[MAX_INFO_LEN];
    char packages[MAX_INFO_LEN];
    char terminal[MAX_INFO_LEN];
} SysInfo;

/* Ubuntu ASCII art logo */
static const char *ubuntu_logo[] = {
    BRIGHT_RED  "            .-/+oossssoo+/-.",
    BRIGHT_RED  "        `:+ssssssssssssssssss+:`",
    BRIGHT_RED  "      -+ssssssssssssssssssyyssss+-",
    BRIGHT_RED  "    .ossssssssssssssssss" BRIGHT_WHITE "dMMMNy" BRIGHT_RED "sssso.",
    BRIGHT_RED  "   /sssssssssss" BRIGHT_WHITE "hdmmNNmmyNMMMMh" BRIGHT_RED "ssssss/",
    BRIGHT_RED  "  +sssssssss" BRIGHT_WHITE "hm" BRIGHT_RED "yd" BRIGHT_WHITE "MMMMMMMNddddy" BRIGHT_RED "ssssssss+",
    BRIGHT_RED  " /ssssssss" BRIGHT_WHITE "hNMMM" BRIGHT_RED "yh" BRIGHT_WHITE "hyyyyhmNMMMNh" BRIGHT_RED "ssssssss/",
    BRIGHT_RED  ".ssssssss" BRIGHT_WHITE "dMMMNh" BRIGHT_RED "ssssssssss" BRIGHT_WHITE "hNMMMd" BRIGHT_RED "ssssssss.",
    BRIGHT_RED  "+ssss" BRIGHT_WHITE "hhhyNMMNy" BRIGHT_RED "ssssssssssss" BRIGHT_WHITE "yNMMMy" BRIGHT_RED "sssssss+",
    BRIGHT_RED  "oss" BRIGHT_WHITE "yNMMMNyMMh" BRIGHT_RED "ssssssssssss" BRIGHT_WHITE "hmmmh" BRIGHT_RED "ssssssso",
    BRIGHT_RED  "oss" BRIGHT_WHITE "yNMMMNyMMh" BRIGHT_RED "ssssssssssss" BRIGHT_WHITE "hmmmh" BRIGHT_RED "ssssssso",
    BRIGHT_RED  "+ssss" BRIGHT_WHITE "hhhyNMMNy" BRIGHT_RED "ssssssssssss" BRIGHT_WHITE "yNMMMy" BRIGHT_RED "sssssss+",
    BRIGHT_RED  ".ssssssss" BRIGHT_WHITE "dMMMNh" BRIGHT_RED "ssssssssss" BRIGHT_WHITE "hNMMMd" BRIGHT_RED "ssssssss.",
    BRIGHT_RED  " /ssssssss" BRIGHT_WHITE "hNMMM" BRIGHT_RED "yh" BRIGHT_WHITE "hyyyyhmNMMMNh" BRIGHT_RED "ssssssss/",
    BRIGHT_RED  "  +sssssssss" BRIGHT_WHITE "dm" BRIGHT_RED "yd" BRIGHT_WHITE "MMMMMMMMddddy" BRIGHT_RED "ssssssss+",
    BRIGHT_RED  "   /sssssssssss" BRIGHT_WHITE "hdmNNNNmyNMMMMh" BRIGHT_RED "ssssss/",
    BRIGHT_RED  "    .ossssssssssssssssss" BRIGHT_WHITE "dMMMNy" BRIGHT_RED "sssso.",
    BRIGHT_RED  "      -+sssssssssssssssssss" BRIGHT_WHITE "yyy" BRIGHT_RED "ssss+-",
    BRIGHT_RED  "        `:+ssssssssssssssssss+:`",
    BRIGHT_RED  "            .-/+oossssoo+/-.",
};

static const int logo_lines = 20;

/* Read first line of a file */
static int read_first_line(const char *path, char *buf, size_t size) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;
    if (!fgets(buf, size, f)) { fclose(f); return -1; }
    fclose(f);
    /* strip newline */
    size_t len = strlen(buf);
    if (len > 0 && buf[len-1] == '\n') buf[len-1] = '\0';
    return 0;
}

/* Run a command and get first line of output */
static int run_cmd(const char *cmd, char *buf, size_t size) {
    FILE *f = popen(cmd, "r");
    if (!f) return -1;
    if (!fgets(buf, size, f)) { pclose(f); return -1; }
    pclose(f);
    size_t len = strlen(buf);
    if (len > 0 && buf[len-1] == '\n') buf[len-1] = '\0';
    return 0;
}

static void get_username(SysInfo *info) {
    struct passwd *pw = getpwuid(getuid());
    if (pw) strncpy(info->user, pw->pw_name, MAX_INFO_LEN - 1);
    else strncpy(info->user, "unknown", MAX_INFO_LEN - 1);
}

static void get_hostname(SysInfo *info) {
    if (gethostname(info->hostname, MAX_INFO_LEN - 1) != 0)
        strncpy(info->hostname, "unknown", MAX_INFO_LEN - 1);
}

static void get_os(SysInfo *info) {
    char line[MAX_INFO_LEN];
    FILE *f = fopen("/etc/os-release", "r");
    if (!f) { strncpy(info->os, "Unknown OS", MAX_INFO_LEN - 1); return; }
    char name[128] = "", version[128] = "";
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "NAME=", 5) == 0) {
            sscanf(line + 5, "\"%127[^\"]\"", name);
            if (name[0] == '\0') sscanf(line + 5, "%127[^\n]", name);
        } else if (strncmp(line, "VERSION=", 8) == 0) {
            sscanf(line + 8, "\"%127[^\"]\"", version);
            if (version[0] == '\0') sscanf(line + 8, "%127[^\n]", version);
        }
    }
    fclose(f);
    snprintf(info->os, MAX_INFO_LEN, "%s %s", name, version);
}

static void get_kernel(SysInfo *info) {
    struct utsname uts;
    if (uname(&uts) == 0)
        snprintf(info->kernel, MAX_INFO_LEN, "%s", uts.release);
    else
        strncpy(info->kernel, "unknown", MAX_INFO_LEN - 1);
}

static void get_arch(SysInfo *info) {
    struct utsname uts;
    if (uname(&uts) == 0)
        snprintf(info->arch, MAX_INFO_LEN, "%s", uts.machine);
    else
        strncpy(info->arch, "x86_64", MAX_INFO_LEN - 1);
}

static void get_uptime(SysInfo *info) {
    struct sysinfo si;
    if (sysinfo(&si) != 0) { strncpy(info->uptime, "unknown", MAX_INFO_LEN-1); return; }
    long up = si.uptime;
    int days    = up / 86400;
    int hours   = (up % 86400) / 3600;
    int minutes = (up % 3600) / 60;
    if (days > 0)
        snprintf(info->uptime, MAX_INFO_LEN, "%d days, %d hours, %d mins", days, hours, minutes);
    else if (hours > 0)
        snprintf(info->uptime, MAX_INFO_LEN, "%d hours, %d mins", hours, minutes);
    else
        snprintf(info->uptime, MAX_INFO_LEN, "%d mins", minutes);
}

static void get_shell(SysInfo *info) {
    const char *sh = getenv("SHELL");
    if (sh) {
        /* Get just the name and version */
        const char *base = strrchr(sh, '/');
        base = base ? base + 1 : sh;
        char ver[64] = "";
        char cmd[128];
        snprintf(cmd, sizeof(cmd), "%s --version 2>/dev/null | head -1", sh);
        char verline[256];
        if (run_cmd(cmd, verline, sizeof(verline)) == 0) {
            /* Extract version number */
            char *v = strstr(verline, " ");
            if (v) {
                char *v2 = strstr(v + 1, " ");
                if (v2) {
                    size_t len = v2 - (v + 1);
                    if (len < 32) { strncpy(ver, v + 1, len); ver[len] = '\0'; }
                }
            }
        }
        if (ver[0])
            snprintf(info->shell, MAX_INFO_LEN, "%s %s", base, ver);
        else
            snprintf(info->shell, MAX_INFO_LEN, "%s", base);
    } else {
        strncpy(info->shell, "unknown", MAX_INFO_LEN - 1);
    }
}

static void get_cpu(SysInfo *info) {
    char line[512];
    FILE *f = fopen("/proc/cpuinfo", "r");
    if (!f) { strncpy(info->cpu, "Unknown CPU", MAX_INFO_LEN-1); return; }
    char model[MAX_INFO_LEN] = "";
    int cores = 0;
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "model name", 10) == 0 && model[0] == '\0') {
            char *colon = strchr(line, ':');
            if (colon) {
                char *start = colon + 2;
                size_t len = strlen(start);
                if (len > 0 && start[len-1] == '\n') start[len-1] = '\0';
                strncpy(model, start, MAX_INFO_LEN - 1);
            }
        }
        if (strncmp(line, "processor", 9) == 0) cores++;
    }
    fclose(f);

    /* Clean up model name: remove extra spaces */
    if (model[0] == '\0') strncpy(model, "Unknown CPU", MAX_INFO_LEN - 1);

    /* Remove "(R)", "(TM)" clutter for display */
    char clean[MAX_INFO_LEN];
    char *src = model, *dst = clean;
    while (*src && (dst - clean) < MAX_INFO_LEN - 1) {
        if (strncmp(src, "(R)", 3) == 0) { src += 3; continue; }
        if (strncmp(src, "(TM)", 4) == 0) { src += 4; continue; }
        *dst++ = *src++;
    }
    *dst = '\0';

    if (cores > 1)
        snprintf(info->cpu, MAX_INFO_LEN, "%s (%d)", clean, cores);
    else
        snprintf(info->cpu, MAX_INFO_LEN, "%s", clean);
}

static void get_memory(SysInfo *info) {
    struct sysinfo si;
    if (sysinfo(&si) != 0) { strncpy(info->memory, "unknown", MAX_INFO_LEN-1); return; }
    long total_mb = (si.totalram * si.mem_unit) / (1024 * 1024);
    long used_mb  = ((si.totalram - si.freeram - si.bufferram) * si.mem_unit) / (1024 * 1024);
    /* Also account for cached */
    char memline[256];
    long cached_mb = 0;
    FILE *f = fopen("/proc/meminfo", "r");
    if (f) {
        while (fgets(memline, sizeof(memline), f)) {
            long val;
            if (sscanf(memline, "Cached: %ld", &val) == 1) { cached_mb = val / 1024; }
            if (sscanf(memline, "Buffers: %ld", &val) == 1) { /* already in si */ (void)val; }
        }
        fclose(f);
    }
    used_mb -= cached_mb;
    if (used_mb < 0) used_mb = 0;
    snprintf(info->memory, MAX_INFO_LEN, "%ldMiB / %ldMiB", used_mb, total_mb);
}

static void get_disk(SysInfo *info) {
    struct statvfs sv;
    if (statvfs("/", &sv) != 0) { strncpy(info->disk, "unknown", MAX_INFO_LEN-1); return; }
    unsigned long long total = (unsigned long long)sv.f_blocks * sv.f_frsize;
    unsigned long long free  = (unsigned long long)sv.f_bfree  * sv.f_frsize;
    unsigned long long used  = total - free;
    /* Convert to GiB */
    double used_g  = used  / (1024.0 * 1024.0 * 1024.0);
    double total_g = total / (1024.0 * 1024.0 * 1024.0);
    snprintf(info->disk, MAX_INFO_LEN, "%.1fGiB / %.1fGiB (%.0f%%)",
             used_g, total_g, (used_g / total_g) * 100.0);
}

static void get_packages(SysInfo *info) {
    char count[64] = "0";
    /* Try dpkg */
    if (run_cmd("dpkg -l 2>/dev/null | grep -c '^ii'", count, sizeof(count)) == 0 && atoi(count) > 0) {
        snprintf(info->packages, MAX_INFO_LEN, "%s (dpkg)", count);
        return;
    }
    /* Try rpm */
    if (run_cmd("rpm -qa 2>/dev/null | wc -l", count, sizeof(count)) == 0 && atoi(count) > 0) {
        snprintf(info->packages, MAX_INFO_LEN, "%s (rpm)", count);
        return;
    }
    /* Try pacman */
    if (run_cmd("pacman -Q 2>/dev/null | wc -l", count, sizeof(count)) == 0 && atoi(count) > 0) {
        snprintf(info->packages, MAX_INFO_LEN, "%s (pacman)", count);
        return;
    }
    strncpy(info->packages, "unknown", MAX_INFO_LEN - 1);
}

static void get_terminal(SysInfo *info) {
    const char *term = getenv("TERM_PROGRAM");
    if (!term) term = getenv("TERM");
    if (term)
        strncpy(info->terminal, term, MAX_INFO_LEN - 1);
    else
        strncpy(info->terminal, "unknown", MAX_INFO_LEN - 1);
}

/* Info lines to display (label + value pairs) */
typedef struct {
    const char *label;
    char value[MAX_INFO_LEN];
} InfoLine;

static void print_color_blocks(void) {
    /* Normal colors */
    printf("   ");
    printf("%s   " RESET, BG_BLACK);
    printf("%s   " RESET, BG_RED);
    printf("%s   " RESET, BG_GREEN);
    printf("%s   " RESET, BG_YELLOW);
    printf("%s   " RESET, BG_BLUE);
    printf("%s   " RESET, BG_MAGENTA);
    printf("%s   " RESET, BG_CYAN);
    printf("%s   " RESET, BG_WHITE);
    printf("\n   ");
    /* Bright colors */
    printf("%s   " RESET, BG_BRIGHT_BLACK);
    printf("%s   " RESET, BG_BRIGHT_RED);
    printf("%s   " RESET, BG_BRIGHT_GREEN);
    printf("%s   " RESET, BG_BRIGHT_YELLOW);
    printf("%s   " RESET, BG_BRIGHT_BLUE);
    printf("%s   " RESET, BG_BRIGHT_MAGENTA);
    printf("%s   " RESET, BG_BRIGHT_CYAN);
    printf("%s   " RESET, BG_BRIGHT_WHITE);
    printf(RESET "\n");
}

/* Visible length of a string (ignoring ANSI escape sequences) */
static int visible_len(const char *s) {
    int len = 0;
    while (*s) {
        if (*s == '\033') {
            while (*s && *s != 'm') s++;
            if (*s) s++;
        } else {
            len++;
            s++;
        }
    }
    return len;
}

int main(void) {
    SysInfo info;
    memset(&info, 0, sizeof(info));

    get_username(&info);
    get_hostname(&info);
    get_os(&info);
    get_kernel(&info);
    get_uptime(&info);
    get_shell(&info);
    get_cpu(&info);
    get_memory(&info);
    get_disk(&info);
    get_arch(&info);
    get_packages(&info);
    get_terminal(&info);

    /* Build info lines */
    InfoLine lines[MAX_LINES];
    int n = 0;

#define ADD(lbl, val) do { lines[n].label = (lbl); strncpy(lines[n].value, (val), MAX_INFO_LEN-1); n++; } while(0)

    /* user@host header */
    ADD(NULL, "");  /* placeholder for user@host */
    ADD(NULL, "");  /* placeholder for separator */
    ADD("OS",       info.os);
    ADD("Host",     info.hostname);
    ADD("Kernel",   info.kernel);
    ADD("Uptime",   info.uptime);
    ADD("Packages", info.packages);
    ADD("Shell",    info.shell);
    ADD("Terminal", info.terminal);
    ADD("CPU",      info.cpu);
    ADD("Arch",     info.arch);
    ADD("Memory",   info.memory);
    ADD("Disk (/)", info.disk);
    ADD(NULL, "");  /* blank */
    ADD(NULL, "");  /* color blocks row 1 */
    ADD(NULL, "");  /* color blocks row 2 */

#undef ADD

    /* Fill user@host */
    snprintf(lines[0].value, MAX_INFO_LEN, "%s@%s", info.user, info.hostname);

    /* Separator line (dashes matching user@host length) */
    int sep_len = (int)strlen(info.user) + 1 + (int)strlen(info.hostname);
    if (sep_len >= MAX_INFO_LEN) sep_len = MAX_INFO_LEN - 1;
    memset(lines[1].value, '-', sep_len);
    lines[1].value[sep_len] = '\0';

    int total = logo_lines > n ? logo_lines : n;

    printf("\n");
    for (int i = 0; i < total; i++) {
        /* Print logo line */
        if (i < logo_lines) {
            printf("   %s" RESET, ubuntu_logo[i]);
            /* Pad to 45 visible chars */
            int vl = visible_len(ubuntu_logo[i]);
            for (int p = vl; p < 46; p++) printf(" ");
        } else {
            /* Pad blank logo area */
            printf("   %*s", 46, "");
        }

        /* Print info line */
        if (i < n) {
            if (i == 0) {
                /* user@host */
                printf(BRIGHT_GREEN BOLD "%s" RESET "@" BRIGHT_GREEN BOLD "%s" RESET,
                       info.user, info.hostname);
            } else if (i == 1) {
                /* separator */
                printf(RESET "%s", lines[i].value);
            } else if (i == n - 2) {
                /* color blocks row 1 */
                printf("%s   %s   %s   %s   %s   %s   %s   %s   " RESET,
                       BG_BLACK, BG_RED, BG_GREEN, BG_YELLOW,
                       BG_BLUE, BG_MAGENTA, BG_CYAN, BG_WHITE);
            } else if (i == n - 1) {
                /* color blocks row 2 */
                printf("%s   %s   %s   %s   %s   %s   %s   %s   " RESET,
                       BG_BRIGHT_BLACK, BG_BRIGHT_RED, BG_BRIGHT_GREEN, BG_BRIGHT_YELLOW,
                       BG_BRIGHT_BLUE, BG_BRIGHT_MAGENTA, BG_BRIGHT_CYAN, BG_BRIGHT_WHITE);
            } else if (lines[i].label) {
                printf(BRIGHT_RED BOLD "%s" RESET ": %s",
                       lines[i].label, lines[i].value);
            }
        }
        printf("\n");
    }
    printf("\n");
    return 0;
}