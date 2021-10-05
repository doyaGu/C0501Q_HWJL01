#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

/* Reverse the bytes in a 32-bit value */
#define SWAP32(q) ((((q) >> 24) & 0xff) + (((q) >> 8) & 0xff00) + \
                    (((q) & 0xff00) << 8) + (((q) & 0xff) << 24))

#define MAX_LABEL 128
#define MAX_OPERANDS 4
#define MAX_FILENAME 2048
#define MAX_CMD_SIZE 0x1000000

static const char *PROGRAM_NAME = "dumpscript";

typedef enum ds_error {
    ERROR_NONE = 0,
    ERROR_UNKNOWN_OPCODE,
    ERROR_FILE_OPEN,
    ERROR_FILE_READ,
    ERROR_EOF,
} ds_error_t;

typedef enum opcode_type {
    OPCODE_BRANCH = 0,
    OPCODE_CONTROL,
    OPCODE_IO,
} opcode_type_t;

typedef enum operand_type {
    OPERAND_NONE = 0,
    OPERAND_ADDRESS,
    OPERAND_DATA,
    OPERAND_MASK,
    OPERAND_VAL,
    OPERAND_OFFSET,
    OPERAND_LENGTH,
    OPERAND_WIDTH,
    OPERAND_PITCH,
} operand_type_t;

typedef struct opcode {
    char *op;
    uint32_t hex;
    int operands;
    operand_type_t operand_types[MAX_OPERANDS];
    opcode_type_t type;
} opcode_t;

typedef uint32_t operand_t;

typedef struct cmd {
    int id;
    size_t offset;
    const opcode_t *opcode;
    operand_t operand[MAX_OPERANDS];
    char label[MAX_LABEL];
    char data_name[MAX_LABEL]; // only used by DATA
    uint8_t *data; // only used by DATA
} cmd_t;

static const opcode_t opcodes[] = {
        {"beq",        0XFFFFFFE0,              2,                  {OPERAND_VAL,     OPERAND_OFFSET},                               OPCODE_BRANCH},
        {"bne",        0XFFFFFFE1,              2,                  {OPERAND_VAL,     OPERAND_OFFSET},                               OPCODE_BRANCH},
        {"bgt",        0XFFFFFFE2,              2,                  {OPERAND_VAL,     OPERAND_OFFSET},                               OPCODE_BRANCH},
        {"bgte",       0XFFFFFFE3,              2,                  {OPERAND_VAL,     OPERAND_OFFSET},                               OPCODE_BRANCH},
        {"blt",        0XFFFFFFE4,              2,                  {OPERAND_VAL,     OPERAND_OFFSET},                               OPCODE_BRANCH},
        {"blte",       0XFFFFFFE5,              2,                  {OPERAND_VAL,     OPERAND_OFFSET},                               OPCODE_BRANCH},
        {"skip",       0XFFFFFFF7,              1,                  {OPERAND_OFFSET},                                                OPCODE_CONTROL},
        {"read_mask",  0XFFFFFFF8,              2,                  {OPERAND_ADDRESS, OPERAND_MASK},                                 OPCODE_IO},
        {"goto",       0XFFFFFFF9,              1,                  {OPERAND_ADDRESS},                                               OPCODE_CONTROL},
        {"write_mask", 0XFFFFFFFA,              2,                  {OPERAND_ADDRESS, OPERAND_MASK},                                 OPCODE_IO},
        {"call",       0XFFFFFFFB,              1,                  {OPERAND_ADDRESS},                                               OPCODE_CONTROL},
        {"data_wait1", 0XFFFFFFFC,              2,                  {OPERAND_ADDRESS, OPERAND_MASK},                                 OPCODE_CONTROL},
        {"data_wait0", 0XFFFFFFFD,              2,                  {OPERAND_ADDRESS, OPERAND_MASK},                                 OPCODE_CONTROL},
        {"data",       0XFFFFFFFE,              4 /* Actually 5 */, {OPERAND_ADDRESS, OPERAND_LENGTH, OPERAND_WIDTH, OPERAND_PITCH}, OPCODE_IO},
        {"wait",       0XFFFFFFFF,              1,                  {OPERAND_DATA},                                                  OPCODE_CONTROL},
        {"write",      0, /* No opcode */       2,                  {OPERAND_DATA,    OPERAND_DATA},                                 OPCODE_IO}
};

static const int OPCODE_NUM = sizeof opcodes / sizeof(opcode_t);

static inline bool is_cmd_data(cmd_t *cmd) {
    return cmd->opcode->hex == 0XFFFFFFFE;
}

typedef struct arguments {
    bool verbose;
    char infile[MAX_FILENAME];
    char outfile[MAX_FILENAME];
} arguments_t;

static struct ds_state {
    size_t offset;
    int id;
    ds_error_t error;
} state = {
        .offset = 0,
        .id = 0,
        .error = ERROR_NONE
};

static void usage() {
    fprintf(stderr, "Usage:\n\t%s [options] [infile] [outfile]\n", PROGRAM_NAME);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "\t-v\toutput verbose information");
}

static void die(const char *msg, int ret_code) {
    fprintf(stderr, "%s: %s\n", PROGRAM_NAME, msg);
    exit(ret_code);
}

static inline bool next_hex(FILE *infile, uint32_t *hex) {
    if (fread(hex, 4, 1, infile) != 1) {
        if (feof(infile)) {
            state.error = ERROR_EOF;
        } else if (ferror(infile)) {
            state.error = ERROR_FILE_READ;
        }
        return false;
    }
    *hex = SWAP32(*hex);
    state.offset += 4;
    return true;
}

static inline void retract_hex(FILE *infile) {
    fseek(infile, -4, SEEK_CUR);
    state.offset -= 4;
}

static inline const opcode_t *match_opcode(uint32_t hex) {
    for (int i = 0; i < (sizeof opcodes / sizeof(opcode_t)); ++i) {
        if (hex == opcodes[i].hex) {
            return &opcodes[i];
        }
    }
    return &opcodes[OPCODE_NUM - 1];
}

static bool fill_operands(FILE *infile, cmd_t *cmd) {
    for (int i = 0; i < cmd->opcode->operands; ++i) {
        next_hex(infile, &cmd->operand[i]);
    }
}

static void get_data(FILE *infile, cmd_t *cmd) {
    const uint32_t length = cmd->operand[1];
    const uint32_t width = cmd->operand[2];
    const size_t filesize = length * width;
    cmd->data = malloc(filesize);
    if (!cmd->data) {
        die("No enough memory!", -1);
    }

    if (fread(cmd->data, sizeof(uint8_t), filesize, infile) != filesize) {
        die("Reading data failed!", -1);
    }

    state.offset += filesize;
}

static bool decode(FILE *infile, cmd_t *cmd) {
    cmd->id = state.id++;
    cmd->offset = state.offset;

    uint32_t hex = 0xFFFFFEF;
    if (!next_hex(infile, &hex)) {
        return false;
    }

    cmd->opcode = match_opcode(hex);
    if (cmd->opcode->hex == 0) {
        retract_hex(infile);
    };
    fill_operands(infile, cmd);
    switch (cmd->opcode->operands) {
        case 1: {
            if (cmd->opcode->operand_types[0] == OPERAND_ADDRESS) {
                sprintf(cmd->label, "%s(0x%.4x)", cmd->opcode->op, cmd->operand[0]);
            } else {
                sprintf(cmd->label, "%s(%u)", cmd->opcode->op, cmd->operand[0]);
            }
        }
            break;
        case 2:
            sprintf(cmd->label, "%s(0x%.4x, 0x%.4x)", cmd->opcode->op, cmd->operand[0], cmd->operand[1]);
            break;
        case 4: {
            const uint32_t base = cmd->operand[0];
            const uint32_t length = cmd->operand[1];
            const uint32_t width = cmd->operand[2];
            const uint32_t pitch = cmd->operand[3];
            snprintf(cmd->data_name, MAX_LABEL, "data_0x%.8x_%ux%u_%u", base, length, width, pitch);
            sprintf(cmd->label, "%s(0x%.8x, %.4u, %.4u, %.4u, \"%s\")", cmd->opcode->op, cmd->operand[0], cmd->operand[1],
                    cmd->operand[2], cmd->operand[3], cmd->data_name);
            get_data(infile, cmd);
        }
            break;
        default:
            die("Abnormal error!", -2);
            break;
    }
    return true;
}

static void output_cmd(FILE *outfile, cmd_t *cmd) {
    fprintf(outfile, "%s;\n", cmd->label);
}

static void output_data(cmd_t *cmd) {
    const char *filename = cmd->data_name;
    FILE *outfile = fopen(filename, "wb");
    if (!outfile) {
        perror("Error");
        die("Unable to open the output data file!", -1);
    }

    const uint32_t length = cmd->operand[1];
    const uint32_t width = cmd->operand[2];
    const size_t filesize = length * width;
    if (fwrite(cmd->data, sizeof(uint8_t), filesize, outfile) != filesize) {
        die("Writing data failed!", -1);
    }
    free(cmd->data);
}

static void parse_args(int argc, char *argv[], arguments_t *args) {
    int n = argc - 2;
    if (n < 1) {
        usage();
        exit(0);
    }

    for (int i = 1; i < n; ++i) {
        const char *option = argv[i];
        if (*option == '-') {
            switch (*(option + 1)) {
                case 'v':
                    args->verbose = true;
                    break;
                default:
                    die("Unknown option!", -1);
            }
        } else {
            die("Wrong argument order!", -1);
        }
    }

    strncpy(args->infile, argv[argc - 2], MAX_FILENAME);
    strncpy(args->outfile, argv[argc - 1], MAX_FILENAME);
}

void decompile(FILE *infile, FILE *outfile) {
    cmd_t cmd = {0};
    while (state.error == ERROR_NONE && decode(infile, &cmd)) {
        if (is_cmd_data(&cmd)) {
            output_data(&cmd);
        }
        output_cmd(outfile, &cmd);
    }
}

int main(int argc, char *argv[]) {
    arguments_t args = {
            .verbose = false,
    };
    parse_args(argc, argv, &args);

    FILE *infile = fopen(args.infile, "rb");
    if (!infile) {
        perror("Error");
        die("Unable to open the input file!", -1);
    }

    FILE *outfile = fopen(args.outfile, "w");
    if (!outfile) {
        perror("Error");
        die("Unable to open the output file!", -1);
    }

    decompile(infile, outfile);
}