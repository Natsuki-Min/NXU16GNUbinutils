
echo -e "                   .-::::::-.                 "
echo -e "                .-'          `-.              "
echo -e "             .-'                `-.           "
echo -e "          .-'                      `-.        "
echo -e "       .-'                            `-.     "
echo -e "    .-'                                  `-.  "
echo -e " .-'        .-::::::-.      .::::::-.        `-."
echo -e " |        .'          `-.  .'          `-.      |"
echo -e " |       /                \/                \     |"
echo -e " |      ;                  ;                  ;    |"
echo -e " |      ;                  ;                  ;    |"
echo -e "  \     ;                  ;                  ;   /"
echo -e "   `.   `-.              .-'              .-'  .' "
echo -e "     `-.   `-.        .-'        .-'    .-'    "
echo -e "        `-.   `-.__.-'    __.-'    .-'         "
echo -e "           `-.          .-'         .-'         "
echo -e "              `-.    .-'        .-'            "
echo -e "                 `-.`-._____.-'              "

ARCH=avr:2
MACHINE=
SCRIPT_NAME=avr
OUTPUT_FORMAT="elf32-avr"
MAXPAGESIZE=1
EMBEDDED=yes
TEMPLATE_NAME=elf

TEXT_LENGTH=8K
DATA_ORIGIN=0x800060
DATA_LENGTH=0xffa0
EXTRA_EM_FILE=avrelf

FUSE_NAME=fuse

EEPROM_LENGTH=64K
FUSE_LENGTH=1K
LOCK_LENGTH=1K
SIGNATURE_LENGTH=1K
USER_SIGNATURE_LENGTH=1K
