/*
 * cfetch - neofetch/fastfetch-like system info tool in C
 * Supports many Linux distributions with colored ASCII art logos
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
#include <ctype.h>

/* ── ANSI colors ─────────────────────────────────────────────────── */
#define R    "\033[0m"
#define BD   "\033[1m"
#define RED  "\033[31m"
#define GRN  "\033[32m"
#define YLW  "\033[33m"
#define BLU  "\033[34m"
#define MAG  "\033[35m"
#define CYN  "\033[36m"
#define WHT  "\033[37m"
#define BBLK "\033[90m"
#define BRED "\033[91m"
#define BGRN "\033[92m"
#define BYLW "\033[93m"
#define BBLU "\033[94m"
#define BMAG "\033[95m"
#define BCYN "\033[96m"
#define BWHT "\033[97m"

/* Background palette */
#define bg0  "\033[40m"
#define bg1  "\033[41m"
#define bg2  "\033[42m"
#define bg3  "\033[43m"
#define bg4  "\033[44m"
#define bg5  "\033[45m"
#define bg6  "\033[46m"
#define bg7  "\033[47m"
#define bg8  "\033[100m"
#define bg9  "\033[101m"
#define bg10 "\033[102m"
#define bg11 "\033[103m"
#define bg12 "\033[104m"
#define bg13 "\033[105m"
#define bg14 "\033[106m"
#define bg15 "\033[107m"

#define MAX_STR  256
#define MAX_INFO 18

/* ── Distro IDs ──────────────────────────────────────────────────── */
typedef enum {
    DISTRO_UNKNOWN = 0,
    DISTRO_UBUNTU,
    DISTRO_DEBIAN,
    DISTRO_ARCH,
    DISTRO_MANJARO,
    DISTRO_FEDORA,
    DISTRO_CENTOS,
    DISTRO_RHEL,
    DISTRO_OPENSUSE,
    DISTRO_ALPINE,
    DISTRO_GENTOO,
    DISTRO_VOID,
    DISTRO_MINT,
    DISTRO_POPOS,
    DISTRO_KALI,
    DISTRO_ENDEAVOUROS,
    DISTRO_GARUDA,
    DISTRO_NIXOS,
    DISTRO_RASPBIAN,
    DISTRO_SLACKWARE,
    DISTRO_MX,
    DISTRO_ZORIN,
    DISTRO_ELEMENTARY,
    DISTRO_COUNT
} DistroID;

typedef struct {
    const char **lines;
    int          count;
    int          width;
    const char  *label_color;
} Logo;

/* ══════════════════════════════════════════════════════════════════
   ASCII ART LOGOS
   ══════════════════════════════════════════════════════════════════ */

/* Ubuntu */
static const char *logo_ubuntu[] = {
    BRED "            .-/+oossssoo+/-.",
    BRED "        `:+ssssssssssssssssss+:`",
    BRED "      -+ssssssssssssssssssyyssss+-",
    BRED "    .ossssssssssssssssss" BWHT "dMMMNy" BRED "sssso.",
    BRED "   /sssssssssss" BWHT "hdmmNNmmyNMMMMh" BRED "ssssss/",
    BRED "  +sssssssss" BWHT "hm" BRED "yd" BWHT "MMMMMMMNddddy" BRED "ssssssss+",
    BRED "  /ssssssss" BWHT "hNMMM" BRED "yh" BWHT "hyyyyhmNMMMNh" BRED "ssssssss/",
    BRED " .ssssssss" BWHT "dMMMNh" BRED "ssssssssss" BWHT "hNMMMd" BRED "ssssssss.",
    BRED " +ssss" BWHT "hhhyNMMNy" BRED "ssssssssssss" BWHT "yNMMMy" BRED "sssssss+",
    BRED " oss" BWHT "yNMMMNyMMh" BRED "ssssssssssssss" BWHT "hmmmh" BRED "ssssssso",
    BRED " oss" BWHT "yNMMMNyMMh" BRED "ssssssssssssss" BWHT "hmmmh" BRED "ssssssso",
    BRED " +ssss" BWHT "hhhyNMMNy" BRED "ssssssssssss" BWHT "yNMMMy" BRED "sssssss+",
    BRED " .ssssssss" BWHT "dMMMNh" BRED "ssssssssss" BWHT "hNMMMd" BRED "ssssssss.",
    BRED "  /ssssssss" BWHT "hNMMM" BRED "yh" BWHT "hyyyyhmNMMMNh" BRED "ssssssss/",
    BRED "  +sssssssss" BWHT "dm" BRED "yd" BWHT "MMMMMMMMddddy" BRED "ssssssss+",
    BRED "   /sssssssssss" BWHT "hdmNNNNmyNMMMMh" BRED "ssssss/",
    BRED "    .ossssssssssssssssss" BWHT "dMMMNy" BRED "sssso.",
    BRED "      -+sssssssssssssssssss" BWHT "yyy" BRED "ssss+-",
    BRED "        `:+ssssssssssssssssss+:`",
    BRED "            .-/+oossssoo+/-.",
};

/* Debian */
static const char *logo_debian[] = {
    RED "                               _,met$$$$$gg.",
    RED "                           ,g$$$$$$$$$$$$$$$P.",
    RED "                         ,g$$P\"\"       \"\"\"Y$$.\".",
    RED "                       ,$$P'              `$$$.",
    RED "                     ',$$P       ,ggs.     `$$b:",
    RED "                     `d$$'     ,$P\"'   .    $$$",
    RED "                      $$P      d$'     ,    $$P",
    RED "                      $$:      $$.   -    ,d$$'",
    RED "                      $$;      Y$b._   _,d$P'",
    RED "                      Y$$.    `.`\"Y$$$$P\"'",
    RED "                      `$$b      \"-.__",
    RED "                       `Y$$b",
    RED "                        `Y$$.",
    RED "                          `$$b.",
    RED "                            `Y$$b.",
    RED "                              `\"Y$b._",
    RED "                                  `\"\"\"\"",
};


/* Arch Linux */
static const char *logo_arch[] = {
    BCYN "                   -`",
    BCYN "                  .o+`",
    BCYN "                 `ooo/",
    BCYN "                `+oooo:",
    BCYN "               `+oooooo:",
    BCYN "               -+oooooo+:",
    BCYN "             `/:-:++oooo+:",
    BCYN "            `/++++/+++++++:",
    BCYN "           `/++++++++++++++:",
    BCYN "          `/+++ooooooooooooo/`",
    BCYN "         ./ooosssso++osssssso+`",
    BCYN "        .oossssso-````/ossssss+`",
    BCYN "       -osssssso.      :ssssssso.",
    BCYN "      :osssssss/        osssso+++.",
    BCYN "     /ossssssss/        +ssssooo/-",
    BCYN "   `/ossssso+/:-        -:/+osssso+-",
    BCYN "  `+sso+:-`                 `.-/+oso:",
    BCYN " `++:.                           `-/+/",
    BCYN " .`                                 `",
};




/* Manjaro */
static const char *logo_manjaro[] = {
    BGRN "||||||||| ||||",
    BGRN "||||||||| ||||",
    BGRN "||||      ||||",
    BGRN "|||| |||| ||||",
    BGRN "|||| |||| ||||",
    BGRN "|||| |||| ||||",
    BGRN "||||      ||||",
    BGRN "|||| |||| ||||",
    BGRN "|||| |||| ||||",
    BGRN "|||| |||| ||||",
};

/* Fedora */
static const char *logo_fedora[] = {
    BBLU "          /:-------------:\\",
    BBLU "       :-------------------:::",
    BBLU "     :-----------" BWHT "/shhOHbmp" BBLU "---:\\",
    BBLU "   /-----------" BWHT "omMMMNNNMMD" BBLU "  ---:",
    BBLU "  :-----------" BWHT "sMMMMNMNMP" BBLU "        :\\",
    BBLU " :-----------" BWHT ":MMMdP" BBLU "------------:",
    BBLU ",------------" BWHT ":MMMd" BBLU "-------------, \\",
    BBLU ":---   " BWHT "mMMM" BBLU "-----------------------:",
    BBLU ":------ " BWHT "mMMM" BBLU "--------------------,",
    BBLU ":-------- " BWHT "MMMMm" BBLU "------------------:",
    BBLU ":---------- " BWHT "MMMMm" BBLU "----------------:",
    BBLU ":----------- " BWHT ":MMMMm" BBLU "-------------:",
    BBLU ":------------ " BWHT ":MMMMMm" BBLU "-----------:",
    BBLU ":--------------" BWHT ":MMMMMm" BBLU "---------:",
    BBLU ":--------------- " BWHT ":MMMMMm" BBLU "------:",
    BBLU " :--------------- " BWHT ":MMMMm" BBLU "----:",
    BBLU "  `:----- " BWHT "mMMMMm" BBLU "-" BWHT "mmMMMm" BBLU "---:'",
    BBLU "     `------" BWHT "mMMMMMMMMm" BBLU "------'",
    BBLU "        `------" BWHT "mMMm" BBLU "------'",
};

/* CentOS */
static const char *logo_centos[] = {
    BYLW "                 ____",
    BYLW "             .--" GRN "/////" BYLW "--.",
    BYLW "           ." GRN "////" BYLW "xx" GRN "////" BYLW ".",
    BYLW "          /" GRN "///" BYLW "xxxxxx" GRN "///" BYLW "/",
    BYLW "        /" GRN "///" BYLW "xx" MAG "/////" GRN "xx" BYLW "///\\",
    BYLW "       |" GRN "//" BYLW "xx" MAG "////////" GRN "xx" BYLW "//|",
    BYLW "       |" MAG "////" BYLW "xx" GRN "/////" MAG "xx////" BYLW "|",
    BYLW "       |" MAG "//////" BYLW "xx" GRN "//" MAG "xx//////" BYLW "|",
    BYLW "       |" MAG "////////" BYLW "xx" MAG "////////" BYLW "|",
    BYLW "        \\" MAG "///" BYLW "xx" GRN "/////" MAG "xx" BYLW "///",
    BYLW "         \\" MAG "//" BYLW "xxxxxx" GRN "////" BYLW "\\",
    BYLW "          \\." GRN "////" BYLW "xx" GRN "////" BYLW "./",
    BYLW "            `." GRN "/////" BYLW ".'",
    BYLW "               `---'",
};

/* openSUSE */
static const char *logo_opensuse[] = {
    BGRN "           .;ldkO0000Okdl;.",
    BGRN "       .;d00xl:^''''''^:ok00d;.",
    BGRN "     .d00l'                'o00d.",
    BGRN "   .d0Kd'  Okxol:;,.         :O0d.",
    BGRN "  .OKKKK0kOKKKKKKKKKKOxo:,    lKO.",
    BGRN " ,0KKKKKKKKKKKKKKKK0P^         ;00,",
    BGRN ".OKKKKKKKKKKKKKKK0P^           xKO,",
    BGRN ":KKKKKKKKKKKKKKKK0o;          OK0,",
    BGRN "dKKKKKKKKKKKOx0KKKKKKd,      .0K0d,",
    BGRN "dKKKKKKKKKd'  .;okOKK0kl.  ,0K0KK0:",
    BGRN "OKKKKKKKK0.       .;oO0K00KKKKKKKKKx",
    BGRN "xKKKKKKKKK0kl,.    .ldk0KKKKKKKKKK0:",
    BGRN ".kKKKKKKKKKKKK0kxoodkOK00KKKKKKKK0l",
    BGRN " 'OK0KKKKKKKKKKKKKKKKKK0000000Okl.",
    BGRN "   'O0KKKKKKKKKKKKKKKK0Kd^",
    BGRN "     .oOOKKKKKKKKKKKKKKKd.",
    BGRN "        .:ldxxddol:;.",
};

/* Alpine */
static const char *logo_alpine[] = {
    BBLU "       /\\ /\\",
    BBLU "      /  V  \\",
    BBLU "     /  (*)  \\",
    BBLU "    / /-   -\\ \\",
    BBLU "   /  |     |  \\",
    BBLU "  /   | /|\\ |   \\",
    BBLU " /    |/ | \\|    \\",
    BBLU "/_____|   |______\\",
};

/* Gentoo */
static const char *logo_gentoo[] = {
    BMAG "         -/oyddmdhs+:",
    BMAG "      -smMMMMMMMMMMMMMMys:",
    BMAG "   .omMMMMMMMMMMMMMMMMMMMMmo.",
    BMAG "  /MMMMMMMMMMMMMMMMMMMMMMMMMMm/",
    BMAG " oMMMMMMMMMMMMMMMMMMMMMMMMMMMMMo",
    BMAG ".MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM.",
    BMAG "OMMMMMMMM" BWHT "MMMMMMMM" BMAG "MMMMMMMMMMMMMMMO",
    BMAG "OMMMMMMMM" BWHT "MMMMMMMM" BMAG "MMMMMMMMMMMMMMMO",
    BMAG ".MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM.",
    BMAG " oMMMMMMMMMMMMMMMMMMMMMMMMMMMMMo",
    BMAG "  /MMMMMMMMMMMMMMMMMMMMMMMMMMm/",
    BMAG "   .omMMMMMMMMMMMMMMMMMMMMmo.",
    BMAG "      -smMMMMMMMMMMMMMMys:",
    BMAG "         -/oyddmdhs+:",
};

/* Void Linux */
static const char *logo_void[] = {
    BGRN "   _______",
    BGRN "   \\______ \\",
    BGRN "    |  |  \\ \\",
    BGRN "    | /" BWHT " \\ " BGRN "\\ \\",
    BGRN "    |/" BWHT "   \\" BGRN "\\ \\",
    BGRN "    |" BWHT "  ___" BGRN "\\ \\",
    BGRN "    |" BWHT " /    \\" BGRN "/ /",
    BGRN "    |" BWHT "/  /\\ " BGRN "/ /",
    BGRN "    |____/ / /",
    BGRN "    /_____/ /",
    BGRN "    \\_______/",
};

/* Linux Mint */
static const char *logo_mint[] = {
    BGRN " MMMMMMMMMMMMMMMMMMMMMMMMMmds+.",
    BGRN " MMm----::-://////////////oymNMd+`",
    BGRN " MMd      " BWHT "/++                " BGRN "-sNMd:",
    BGRN " MMNso/`  " BWHT "dMM    `.::-. .-::.`" BGRN " .hMN:",
    BGRN " ddddMMh  " BWHT "dMM   :hNMNMNhNMNMNh:" BGRN " `NMm",
    BGRN "     NMm  " BWHT "dMM  .NMN/-+MMM+-/NMN`" BGRN " dMM",
    BGRN "     NMm  " BWHT "dMM  -MMm  `MMM   dMM." BGRN " dMM",
    BGRN "     NMm  " BWHT "dMM  .mmd  `mmm   yMM." BGRN " dMM",
    BGRN "     NMm  " BWHT "dMM`  ..-   ...   ydm." BGRN " dMM",
    BGRN "     hMM- " BWHT "+MMd/-------...-:sdds  " BGRN "dMM",
    BGRN "     -NMm- " BWHT ":hNMNNNmdddddddddy/`  " BGRN "dMM",
    BGRN "      -dMNs-" BWHT "``-::::-------.``    " BGRN "dMM",
    BGRN "        `/dMNmy+/:-------------:/yMMM",
    BGRN "           ./ydNMMMMMMMMMMMMMMMMMMMMM",
    BGRN "              .MMMMMMMMMMMMMMMMMMM",
};

/* Pop!_OS */
static const char *logo_popos[] = {
    BCYN "             /////////////",
    BCYN "         /////////////////////",
    BCYN "      ///////" BWHT "*767" BCYN "///////////////",
    BCYN "    //////" BWHT "7676767676*" BCYN "//////////",
    BCYN "   /////" BWHT "76767" BCYN "/////(" BWHT "7676767" BCYN "////",
    BCYN "  /////" BWHT "767676" BCYN "///////////////////",
    BCYN " /////" BWHT "7676767676" BCYN "////////////////",
    BCYN " ////" BWHT "767676767676" BCYN "////" BWHT "76767" BCYN "///",
    BCYN " ///" BWHT "76767676767676" BCYN "///" BWHT "7676" BCYN "////",
    BCYN " ///" BWHT "76767676767676" BCYN "///" BWHT "7676" BCYN "////",
    BCYN " ////" BWHT "7676767676" BCYN "////////////",
    BCYN "  /////" BWHT "7676767" BCYN "/////////////",
    BCYN "   /////(" BWHT "7676" BCYN "//////////////",
    BCYN "    ///////" BWHT "7676" BCYN "/////////",
    BCYN "       ///////" BWHT "*" BCYN "////////",
    BCYN "         ///////////////",
    BCYN "            /////////",
};

/* Kali Linux */
static const char *logo_kali[] = {
    BBLU "...............",
    BBLU "..,;:ccccccc:;,.",
    BBLU ".,ccccccccccccccc:.",
    BBLU ".;ccccccccccccccccccc;.",
    BBLU ".:ccccccccccccc;" BWHT "oooooooc" BBLU ":.",
    BBLU ".:cccccccccccc;" BWHT "OMMMMMMMMo" BBLU ":.",
    BBLU ".;ccccccccccc;" BWHT "OMMMMMMMMMO" BBLU ";.",
    BBLU ".;cccccccccc;" BWHT "OMMMMMMMMMMMO" BBLU ";.",
    BBLU ".;ccccccccc;" BWHT "OMMMMMdMMMMMMMO" BBLU ";.",
    BBLU ".;cccccccc;" BWHT "oMMMMMd:dMMMMMMMo" BBLU ";.",
    BBLU ".;cccccc;.  " BWHT ":MMMMd:  :dMMMMMo" BBLU " .;.",
    BBLU ".:ccc:.     " BWHT " :d:.     :dMMMo" BBLU "   .:.",
    BBLU ".''.          '.       '.''",
};

/* EndeavourOS */
static const char *logo_endeavouros[] = {
    BMAG "                     ./{}\\.",
    BMAG "                    //{" BRED "|" BMAG "}\\\\",
    BMAG "                   //{" BRED "||" BMAG "}\\\\",
    BMAG "                  //{" BRED "|||" BMAG "}\\\\",
    BMAG "                 //{" BRED "||||" BMAG "}\\\\",
    BMAG "                //{" BRED "|||||" BMAG "}\\\\",
    BMAG "               //{" BRED "||||||" BMAG "}\\\\",
    BMAG "              //{" BRED "|||||||" BMAG "}\\\\",
    BMAG "             //{" BRED "||||||||" BMAG "}\\\\",
    BMAG "            //{" BRED "|||||||||" BMAG "}\\\\",
    BMAG "           //{" BRED "||||||||||" BMAG "}\\\\",
    BMAG "          //{" BRED "|||||||||||" BMAG "}\\\\",
    BMAG "         //{" BRED "||||||||||||" BMAG "}\\\\",
    BMAG "        //{" BRED "|||||||||||||" BMAG "}\\\\",
    BMAG "       //{" BRED "||||||||||||||" BMAG "}\\\\",
    BMAG "      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\\\\",
};

/* NixOS */
static const char *logo_nixos[] = {
    BBLU "          \\\\  \\\\ //",
    BBLU "         ==\\\\--\\\\/ //",
    BBLU "           //   \\\\//",
    BBLU "        ==//     //==",
    BBLU "         //\\\\   //",
    BBLU "        // \\\\  //==",
    BBLU "           \\\\ //",
    BBLU "         ==/\\\\ //\\\\",
    BBLU "            // //\\\\",
    BBLU "         ==// //==",
    BBLU "           ///",
    BBLU "        ==//",
};

/* Garuda */
static const char *logo_garuda[] = {
    BRED "                  .^.",
    BRED "                 /W{\\",
    BRED "                /WW{\\\\",
    BRED "               /WWW{\\\\\\",
    BRED "              /WWWW{\\\\\\\\",
    BRED "             /WWWWW{\\\\\\\\\\",
    BRED "            /WWWWWW{\\\\\\\\\\\\",
    BRED "           /WWWWWWW{\\\\\\\\\\\\\\",
    BRED "          /WWWWWWWW{\\\\\\\\\\\\\\\\",
    BRED "         /WWWWWWWWW{\\\\\\\\\\\\\\\\\\",
    BRED "        .WWWWWWWWWW{\\\\\\\\\\\\\\\\\\\\",
    BRED "        `----------'",
};

/* Raspbian */
static const char *logo_raspbian[] = {
    BRED "  .~~.   .~~.",
    BRED " '. \\ ' ' / .'",
    RED  "  .~ .~~~..~.",
    RED  " : .~.'~'.~. :",
    RED  "~ (   ) (   ) ~",
    RED  "( : '~'.~.'~' : )",
    RED  " ~ .~ (   ) ~. ~",
    RED  "  (  : '~' :  )",
    RED  "   '~ .~~~. ~'",
    RED  "       '~'",
};

/* MX Linux */
static const char *logo_mx[] = {
    BWHT "MMMMMMMMMMMMMMMMMMMMMMMMMMMMMNNMMMMM",
    BWHT "MMMMMMMMMMNs..yMMMMMMMMMMMMm: +MMMMM",
    BWHT "MMMMMMMMMM+   :NMMMMMMMMMMs  .MMMMM",
    BWHT "MMMMMMMMMm.    oMMMMMMMMMy.   oMMMM",
    BWHT "MMMMMMMMMy      sMMMMMMMo     -MMMM",
    BWHT "MMMMMMMMm.      .NMMMMMy       hMMM",
    BWHT "MMMMMMMMo         hMMMm.       /MMM",
    BWHT "MMMMMMMN.         .MMM-         NMM",
    BWHT "MMMMMMMy           :h.          oMM",
    BWHT "MMMMMMM`                         NM",
    BWHT "MMMMMMs             .            /M",
    BWHT "MMMMMN`            /o.            M",
    BWHT "MMMMm-           `oMs.           `N",
    BWHT "MMMMo          .yMMMMs            y",
    BWHT "MMMy          sMMMMMMd.           :",
    BWHT "MMN`        .mMMMMMMMMs            ",
};

/* Slackware */
static const char *logo_slackware[] = {
    BBLU "   ________  ___.",
    BBLU "  /        \\|   |",
    BBLU " /  ______ /|   |",
    BBLU " \\_____   \\/ |   |",
    BBLU "  \\     \\   |   |",
    BBLU "  |      \\  |   |",
    BBLU "  |_______|\\|___|",
    BBLU "  Slackware",
};

/* Zorin OS */
static const char *logo_zorin[] = {
    BBLU "        `osssssssssssssssssssso`",
    BBLU "       .osssssssssssssssssssssso.",
    BBLU "      .+yyyyyyyyyyyyyyyyyyyyyyyyo+.",
    BBLU "    `:+yyyyyyyyyyyyyyyyyyyyyyyyyyy+:`",
    BBLU "   :oyyy+" BWHT "hhhhhhhhhhhhhhhhhhhhh" BBLU "+yyyo:",
    BBLU "  :oyyy+" BWHT "hhhhhhhhhhhhhhhhhhhhhhh" BBLU "+yyyo:",
    BBLU " :oyyyyy+hhh" BWHT "hhhhhhhhhhhhhhhh" BBLU "hhh+yyyyy:",
    BBLU " :oyyyyy+hhh" BWHT "hhhhhhhhhhhhhhhh" BBLU "hhh+yyyyy:",
    BBLU "  :oyyyy+" BWHT "hhhhhhhhhhhhhhhhhhhhhh" BBLU "+yyyy:",
    BBLU "   `:+yyy+" BWHT "hhhhhhhhhhhhhhhhhhhh" BBLU "+yyy+:`",
    BBLU "     `:+yyyyyyyyyyyyyyyyyyyyyyy+:`",
    BBLU "       `.+yyyyyyyyyyyyyyyyyyy+.`",
    BBLU "          `osssssssssssssso`",
};

/* elementary OS */
static const char *logo_elementary[] = {
    BBLU "         eeeeeeeeeeeeeee",
    BBLU "      eeeeeeeeeeeeeeeeeeeee",
    BBLU "    eeeee  eeeeeeeeeeee   eeeee",
    BBLU "  eeee   eeeee       eeeee   eeee",
    BBLU " eeee   eeee           eeee   eeee",
    BBLU "eee    eee               eee    eee",
    BBLU "eee   eee                 eee   eee",
    BBLU "ee    eee                 eee    ee",
    BBLU "ee    eee                 eee    ee",
    BBLU "eee   eee                 eee   eee",
    BBLU "eee    eee               eee    eee",
    BBLU " eeee   eeee           eeee   eeee",
    BBLU "  eeee   eeeee       eeeee   eeee",
    BBLU "    eeeee  eeeeeeeeeeee  eeeee",
    BBLU "      eeeeeeeeeeeeeeeeeeeee",
    BBLU "         eeeeeeeeeeeeeee",
};

/* Unknown / Generic Linux */
static const char *logo_unknown[] = {
    BWHT "        #####",
    BWHT "       #######",
    BWHT "       ##" BBLK "O" BWHT "#" BBLK "O" BWHT "##",
    BWHT "       #" BYLW "#####" BWHT "#",
    BWHT "     ##" BWHT "##" BYLW "###" BWHT "##" BWHT "##",
    BWHT "    #" BWHT "##########" BWHT "##",
    BWHT "   #" BWHT "############" BWHT "##",
    BWHT "   #" BWHT "############" BWHT "###",
    BWHT "  ##" BYLW "#" BWHT "###########" BWHT "##" BYLW "#",
    BWHT "######" BYLW "#" BWHT "#######" BYLW "#" BWHT "######",
    BWHT " #" BYLW "####" BWHT "#######" BYLW "#####" BWHT "#",
    BWHT "  " BYLW "####" BWHT "#######" BYLW "#####",
    BWHT "    " BYLW "####" BWHT "#####" BYLW "###",
    BWHT "      " BYLW "##########",
    BWHT "       " BYLW "########",
};

/* ── Logo table ──────────────────────────────────────────────────── */
#define NLINES(a) ((int)(sizeof(a)/sizeof(a[0])))
#define LOGO(id, arr, w, col) \
    [id] = { (arr), NLINES(arr), (w), (col) }

static const Logo logos[DISTRO_COUNT] = {
    LOGO(DISTRO_UNKNOWN,     logo_unknown,     22, BWHT),
    LOGO(DISTRO_UBUNTU,      logo_ubuntu,      45, BRED),
    LOGO(DISTRO_DEBIAN,      logo_debian,      18, RED),
    LOGO(DISTRO_ARCH,        logo_arch,        36, BCYN),
    LOGO(DISTRO_MANJARO,     logo_manjaro,     14, BGRN),
    LOGO(DISTRO_FEDORA,      logo_fedora,      36, BBLU),
    LOGO(DISTRO_CENTOS,      logo_centos,      26, BYLW),
    LOGO(DISTRO_RHEL,        logo_centos,      26, BRED),
    LOGO(DISTRO_OPENSUSE,    logo_opensuse,    44, BGRN),
    LOGO(DISTRO_ALPINE,      logo_alpine,      18, BBLU),
    LOGO(DISTRO_GENTOO,      logo_gentoo,      36, BMAG),
    LOGO(DISTRO_VOID,        logo_void,        20, BGRN),
    LOGO(DISTRO_MINT,        logo_mint,        40, BGRN),
    LOGO(DISTRO_POPOS,       logo_popos,       29, BCYN),
    LOGO(DISTRO_KALI,        logo_kali,        34, BBLU),
    LOGO(DISTRO_ENDEAVOUROS, logo_endeavouros, 32, BMAG),
    LOGO(DISTRO_GARUDA,      logo_garuda,      28, BRED),
    LOGO(DISTRO_NIXOS,       logo_nixos,       22, BBLU),
    LOGO(DISTRO_RASPBIAN,    logo_raspbian,    18, BRED),
    LOGO(DISTRO_SLACKWARE,   logo_slackware,   18, BBLU),
    LOGO(DISTRO_MX,          logo_mx,          36, BWHT),
    LOGO(DISTRO_ZORIN,       logo_zorin,       36, BBLU),
    LOGO(DISTRO_ELEMENTARY,  logo_elementary,  25, BBLU),
};

/* ══════════════════════════════════════════════════════════════════
   DISTRO DETECTION
   ══════════════════════════════════════════════════════════════════ */

static void str_lower(char *s) {
    for (; *s; s++) *s = (char)tolower((unsigned char)*s);
}

static DistroID detect_distro(char *id_out, size_t id_len) {
    FILE *f = fopen("/etc/os-release", "r");
    if (!f) {
        strncpy(id_out, "unknown", id_len - 1);
        id_out[id_len-1] = '\0';
        return DISTRO_UNKNOWN;
    }

    char line[256], id_str[64]="", like_str[64]="";
    while (fgets(line, sizeof(line), f)) {
        size_t l = strlen(line);
        if (l > 0 && line[l-1] == '\n') line[l-1] = '\0';
        char tmp[64];
        if (strncmp(line, "ID=", 3) == 0) {
            char *v = line + 3;
            if (*v == '"') sscanf(v, "\"%63[^\"]\"", tmp);
            else           sscanf(v, "%63[^\n]", tmp);
            strncpy(id_str, tmp, 63);
        } else if (strncmp(line, "ID_LIKE=", 8) == 0) {
            char *v = line + 8;
            if (*v == '"') sscanf(v, "\"%63[^\"]\"", tmp);
            else           sscanf(v, "%63[^\n]", tmp);
            strncpy(like_str, tmp, 63);
        }
    }
    fclose(f);

    strncpy(id_out, id_str, id_len - 1);
    id_out[id_len-1] = '\0';

    char id_lc[64], like_lc[64];
    strncpy(id_lc,   id_str,   63);   id_lc[63]   = '\0';
    strncpy(like_lc, like_str, 63);   like_lc[63] = '\0';
    str_lower(id_lc);
    str_lower(like_lc);

    /* Primary ID match table */
    static const struct { const char *key; DistroID val; } tbl[] = {
        { "ubuntu",              DISTRO_UBUNTU      },
        { "debian",              DISTRO_DEBIAN      },
        { "arch",                DISTRO_ARCH        },
        { "manjaro",             DISTRO_MANJARO     },
        { "fedora",              DISTRO_FEDORA      },
        { "centos",              DISTRO_CENTOS      },
        { "rhel",                DISTRO_RHEL        },
        { "opensuse",            DISTRO_OPENSUSE    },
        { "opensuse-leap",       DISTRO_OPENSUSE    },
        { "opensuse-tumbleweed", DISTRO_OPENSUSE    },
        { "sles",                DISTRO_OPENSUSE    },
        { "alpine",              DISTRO_ALPINE      },
        { "gentoo",              DISTRO_GENTOO      },
        { "void",                DISTRO_VOID        },
        { "linuxmint",           DISTRO_MINT        },
        { "pop",                 DISTRO_POPOS       },
        { "kali",                DISTRO_KALI        },
        { "endeavouros",         DISTRO_ENDEAVOUROS },
        { "garuda",              DISTRO_GARUDA      },
        { "nixos",               DISTRO_NIXOS       },
        { "raspbian",            DISTRO_RASPBIAN    },
        { "slackware",           DISTRO_SLACKWARE   },
        { "mx",                  DISTRO_MX          },
        { "zorin",               DISTRO_ZORIN       },
        { "elementary",          DISTRO_ELEMENTARY  },
        /* common derivatives */
        { "kubuntu",             DISTRO_UBUNTU      },
        { "lubuntu",             DISTRO_UBUNTU      },
        { "xubuntu",             DISTRO_UBUNTU      },
        { "ubuntu-mate",         DISTRO_UBUNTU      },
        { "neon",                DISTRO_UBUNTU      },
        { "pureos",              DISTRO_DEBIAN      },
        { "lmde",                DISTRO_MINT        },
        { "antergos",            DISTRO_ARCH        },
        { "blackarch",           DISTRO_ARCH        },
        { "artix",               DISTRO_ARCH        },
        { "arcolinux",           DISTRO_ARCH        },
        { "scientific",          DISTRO_CENTOS      },
        { "ol",                  DISTRO_RHEL        },
        { "almalinux",           DISTRO_RHEL        },
        { "rocky",               DISTRO_RHEL        },
        { NULL, 0 }
    };

    for (int i = 0; tbl[i].key; i++)
        if (strcmp(id_lc, tbl[i].key) == 0)
            return tbl[i].val;

    /* ID_LIKE fallback */
    if (strstr(like_lc, "ubuntu"))  return DISTRO_UBUNTU;
    if (strstr(like_lc, "debian"))  return DISTRO_DEBIAN;
    if (strstr(like_lc, "arch"))    return DISTRO_ARCH;
    if (strstr(like_lc, "fedora"))  return DISTRO_FEDORA;
    if (strstr(like_lc, "rhel"))    return DISTRO_RHEL;
    if (strstr(like_lc, "suse"))    return DISTRO_OPENSUSE;

    return DISTRO_UNKNOWN;
}

/* ══════════════════════════════════════════════════════════════════
   SYSTEM INFO
   ══════════════════════════════════════════════════════════════════ */

typedef struct {
    char user[MAX_STR];
    char hostname[MAX_STR];
    char os[MAX_STR];
    char kernel[MAX_STR];
    char uptime[MAX_STR];
    char shell[MAX_STR];
    char cpu[MAX_STR];
    char memory[MAX_STR];
    char disk[MAX_STR];
    char arch[MAX_STR];
    char packages[MAX_STR];
    char terminal[MAX_STR];
    char distro_id[64];
    DistroID distro;
} SysInfo;

static int run_cmd(const char *cmd, char *buf, size_t size) {
    FILE *f = popen(cmd, "r");
    if (!f) return -1;
    int ok = (fgets(buf, (int)size, f) != NULL);
    pclose(f);
    if (!ok) return -1;
    size_t l = strlen(buf);
    if (l > 0 && buf[l-1] == '\n') buf[l-1] = '\0';
    return 0;
}

static void get_user(SysInfo *s) {
    struct passwd *pw = getpwuid(getuid());
    strncpy(s->user, pw ? pw->pw_name : "user", MAX_STR - 1);
    s->user[MAX_STR-1] = '\0';
}

static void get_hostname(SysInfo *s) {
    if (gethostname(s->hostname, MAX_STR - 1) != 0)
        strncpy(s->hostname, "localhost", MAX_STR - 1);
    s->hostname[MAX_STR-1] = '\0';
}

static void get_os(SysInfo *s) {
    FILE *f = fopen("/etc/os-release", "r");
    if (!f) { strncpy(s->os, "Linux", MAX_STR - 1); return; }
    char line[256], pretty[128]="";
    while (fgets(line, sizeof(line), f)) {
        char tmp[128];
        if (strncmp(line, "PRETTY_NAME=", 12) == 0) {
            if (sscanf(line+12, "\"%127[^\"]\"", tmp) == 1 ||
                sscanf(line+12, "%127[^\n]",    tmp) == 1)
                strncpy(pretty, tmp, 127);
            break;
        }
    }
    fclose(f);
    snprintf(s->os, MAX_STR, "%s", pretty[0] ? pretty : "Linux");
}

static void get_kernel(SysInfo *s) {
    struct utsname u;
    if (uname(&u) == 0) snprintf(s->kernel, MAX_STR, "%s", u.release);
    else strncpy(s->kernel, "unknown", MAX_STR - 1);
}

static void get_arch(SysInfo *s) {
    struct utsname u;
    if (uname(&u) == 0) snprintf(s->arch, MAX_STR, "%s", u.machine);
    else strncpy(s->arch, "x86_64", MAX_STR - 1);
}

static void get_uptime(SysInfo *s) {
    struct sysinfo si;
    if (sysinfo(&si) != 0) { strncpy(s->uptime, "unknown", MAX_STR-1); return; }
    long up = si.uptime;
    int d=(int)(up/86400), h=(int)((up%86400)/3600), m=(int)((up%3600)/60);
    if (d > 0)      snprintf(s->uptime, MAX_STR, "%dd %dh %dm", d, h, m);
    else if (h > 0) snprintf(s->uptime, MAX_STR, "%dh %dm", h, m);
    else            snprintf(s->uptime, MAX_STR, "%dm", m);
}

static void get_shell(SysInfo *s) {
    const char *sh = getenv("SHELL");
    if (!sh) { strncpy(s->shell, "unknown", MAX_STR-1); return; }
    const char *base = strrchr(sh, '/');
    base = base ? base + 1 : sh;
    char cmd[128], vline[256], ver[32]="";
    snprintf(cmd, sizeof(cmd), "%s --version 2>/dev/null | head -1", sh);
    if (run_cmd(cmd, vline, sizeof(vline)) == 0) {
        char *p = vline;
        while (*p && !isdigit((unsigned char)*p)) p++;
        if (*p) {
            char *e = p;
            while (*e && (isdigit((unsigned char)*e) || *e == '.')) e++;
            size_t l = (size_t)(e - p);
            if (l > 0 && l < 32) { memcpy(ver, p, l); ver[l] = '\0'; }
        }
    }
    if (ver[0]) snprintf(s->shell, MAX_STR, "%s %s", base, ver);
    else        snprintf(s->shell, MAX_STR, "%s", base);
}

static void get_cpu(SysInfo *s) {
    FILE *f = fopen("/proc/cpuinfo", "r");
    if (!f) { strncpy(s->cpu, "Unknown CPU", MAX_STR-1); return; }
    char line[512], model[MAX_STR]="";
    int cores = 0;
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "model name", 10) == 0 && model[0] == '\0') {
            char *c = strchr(line, ':');
            if (c) {
                char *p = c + 2;
                size_t l = strlen(p);
                if (l > 0 && p[l-1] == '\n') p[l-1] = '\0';
                strncpy(model, p, MAX_STR-1);
            }
        }
        if (strncmp(line, "processor", 9) == 0) cores++;
    }
    fclose(f);
    if (model[0] == '\0') strncpy(model, "Unknown CPU", MAX_STR-1);

    /* strip (R) (TM), collapse spaces */
    char tmp[MAX_STR]; char *d = tmp;
    for (char *p = model; *p && (d-tmp) < MAX_STR-1; ) {
        if (strncmp(p,"(R)",3)==0){ p+=3; continue; }
        if (strncmp(p,"(TM)",4)==0){ p+=4; continue; }
        *d++ = *p++;
    }
    *d = '\0';
    char out[MAX_STR]; d = out;
    int sp = 0;
    for (char *p=tmp; *p && (d-out)<MAX_STR-1; p++) {
        if (*p==' '){ if(!sp){*d++=' '; sp=1;} }
        else { *d++=*p; sp=0; }
    }
    *d = '\0';
    /* trim leading/trailing space */
    char *start = out;
    while (*start == ' ') start++;
    size_t slen = strlen(start);
    while (slen > 0 && start[slen-1] == ' ') slen--;
    start[slen] = '\0';

    snprintf(s->cpu, MAX_STR, "%s (%d)", start, cores);
}

static void get_memory(SysInfo *s) {
    long total_kb=0, avail_kb=0;
    FILE *f = fopen("/proc/meminfo", "r");
    if (!f) { strncpy(s->memory, "unknown", MAX_STR-1); return; }
    char line[128];
    while (fgets(line, sizeof(line), f)) {
        long v;
        if (sscanf(line, "MemTotal: %ld", &v) == 1)     total_kb = v;
        if (sscanf(line, "MemAvailable: %ld", &v) == 1) avail_kb = v;
    }
    fclose(f);
    long used_mb  = (total_kb - avail_kb) / 1024;
    long total_mb = total_kb / 1024;
    snprintf(s->memory, MAX_STR, "%ldMiB / %ldMiB", used_mb, total_mb);
}

static void get_disk(SysInfo *s) {
    struct statvfs sv;
    if (statvfs("/", &sv) != 0) { strncpy(s->disk, "unknown", MAX_STR-1); return; }
    unsigned long long tot = (unsigned long long)sv.f_blocks * sv.f_frsize;
    unsigned long long fre = (unsigned long long)sv.f_bfree  * sv.f_frsize;
    double used_g  = (tot - fre) / 1073741824.0;
    double total_g = tot / 1073741824.0;
    snprintf(s->disk, MAX_STR, "%.1fGiB / %.1fGiB (%.0f%%)",
             used_g, total_g, total_g > 0 ? (used_g/total_g)*100.0 : 0.0);
}

static void get_packages(SysInfo *s) {
    /* Each entry: shell command that prints a count, and the manager name */
    static const struct { const char *cmd; const char *label; } pms[] = {
        { "dpkg -l 2>/dev/null | grep -c '^ii'",               "dpkg"    },
        { "rpm -qa --queryformat='.' 2>/dev/null | wc -c",     "rpm"     },
        { "pacman -Qq 2>/dev/null | wc -l",                    "pacman"  },
        { "apk info 2>/dev/null | wc -l",                      "apk"     },
        { "xbps-query -l 2>/dev/null | wc -l",                 "xbps"    },
        { "eopkg list-installed 2>/dev/null | wc -l",          "eopkg"   },
        { "nix-env -q 2>/dev/null | wc -l",                    "nix"     },
        { "flatpak list 2>/dev/null | wc -l",                  "flatpak" },
        { "snap list 2>/dev/null | tail -n +2 | wc -l",        "snap"    },
        { "brew list 2>/dev/null | wc -l",                     "brew"    },
        { NULL, NULL }
    };

    char result[MAX_STR] = "", buf[64];
    int found = 0;
    for (int i = 0; pms[i].cmd; i++) {
        if (run_cmd(pms[i].cmd, buf, sizeof(buf)) == 0 && atoi(buf) > 0) {
            char part[80];
            snprintf(part, sizeof(part), "%s%d (%s)", found ? ", " : "", atoi(buf), pms[i].label);
            size_t rem = MAX_STR - strlen(result) - 1;
            if (rem > strlen(part)) strncat(result, part, rem);
            found = 1;
        }
    }
    strncpy(s->packages, found ? result : "unknown", MAX_STR-1);
    s->packages[MAX_STR-1] = '\0';
}

static void get_terminal(SysInfo *s) {
    const char *t = getenv("TERM_PROGRAM");
    if (!t) t = getenv("TERM");
    if (!t) t = "unknown";
    strncpy(s->terminal, t, MAX_STR-1);
    s->terminal[MAX_STR-1] = '\0';
}

/* ══════════════════════════════════════════════════════════════════
   RENDERING
   ══════════════════════════════════════════════════════════════════ */

/* Visible (printable) character width, skipping ANSI escapes */
static int visible_len(const char *s) {
    int len = 0;
    while (*s) {
        if (*s == '\033') {
            while (*s && *s != 'm') s++;
            if (*s) s++;
        } else {
            unsigned char c = (unsigned char)*s;
            if ((c & 0xC0) != 0x80) len++;  /* UTF-8 leading byte */
            s++;
        }
    }
    return len;
}

typedef struct { const char *label; char value[MAX_STR]; } InfoLine;

static void render(const SysInfo *si, const Logo *logo) {
    InfoLine info[MAX_INFO + 4];
    int n = 0;

#define ADD(lbl, val) do { \
    if (n < MAX_INFO+4) { \
        info[n].label = (lbl); \
        strncpy(info[n].value, (val), MAX_STR-1); \
        info[n].value[MAX_STR-1] = '\0'; \
        n++; \
    } \
} while (0)

    ADD(NULL, "");         /* user@host */
    ADD(NULL, "");         /* separator */
    ADD("OS",        si->os);
    ADD("Kernel",    si->kernel);
    ADD("Arch",      si->arch);
    ADD("Uptime",    si->uptime);
    ADD("Packages",  si->packages);
    ADD("Shell",     si->shell);
    ADD("Terminal",  si->terminal);
    ADD("CPU",       si->cpu);
    ADD("Memory",    si->memory);
    ADD("Disk (/)",  si->disk);
    ADD(NULL, "");         /* blank */
    ADD(NULL, "");         /* palette 1 */
    ADD(NULL, "");         /* palette 2 */
#undef ADD

    /* Fill user@host */
    snprintf(info[0].value, MAX_STR, "%s@%s", si->user, si->hostname);
    /* Fill separator */
    int seplen = (int)(strlen(si->user) + 1 + strlen(si->hostname));
    if (seplen >= MAX_STR) seplen = MAX_STR - 1;
    memset(info[1].value, '-', (size_t)seplen);
    info[1].value[seplen] = '\0';

    const char *col = logo->label_color;
    int logo_w = logo->width;
    int total  = logo->count > n ? logo->count : n;

    int palette1 = n - 2;
    int palette2 = n - 1;

    printf("\n");
    for (int i = 0; i < total; i++) {
        /* Logo */
        if (i < logo->count) {
            printf("   %s" R, logo->lines[i]);
            int pad = logo_w - visible_len(logo->lines[i]);
            while (pad-- > 0) printf(" ");
            printf("  ");
        } else {
            printf("   %*s  ", logo_w, "");
        }

        /* Info */
        if (i < n) {
            if (i == 0) {
                /* user@host */
                printf(BGRN BD "%s" R "@" BGRN BD "%s" R, si->user, si->hostname);
            } else if (i == 1) {
                /* separator */
                printf("%s", info[1].value);
            } else if (i == palette1) {
                printf(bg0"   "bg1"   "bg2"   "bg3"   "bg4"   "bg5"   "bg6"   "bg7"   "R);
            } else if (i == palette2) {
                printf(bg8"   "bg9"   "bg10"   "bg11"   "bg12"   "bg13"   "bg14"   "bg15"   "R);
            } else if (info[i].label) {
                printf(BD "%s%s" R ": %s", col, info[i].label, info[i].value);
            }
        }
        printf("\n");
    }
    printf("\n");
}

/* ══════════════════════════════════════════════════════════════════
   MAIN
   ══════════════════════════════════════════════════════════════════ */

int main(void) {
    SysInfo si;
    memset(&si, 0, sizeof(si));

    si.distro = detect_distro(si.distro_id, sizeof(si.distro_id));

    get_user(&si);
    get_hostname(&si);
    get_os(&si);
    get_kernel(&si);
    get_arch(&si);
    get_uptime(&si);
    get_shell(&si);
    get_cpu(&si);
    get_memory(&si);
    get_disk(&si);
    get_packages(&si);
    get_terminal(&si);

    const Logo *logo = &logos[si.distro];
    if (!logo->lines || logo->count == 0)
        logo = &logos[DISTRO_UNKNOWN];

    render(&si, logo);
    return 0;
}