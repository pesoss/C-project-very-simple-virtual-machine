#include <stdio.h>
#include <err.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <stdbool.h>
#include <errno.h>

const size_t NUMBER_OF_ARGS = 2;
// I find this function on the link bellow, witch sleep the program in milliseconds
// https://stackoverflow.com/questions/1157209/is-there-an-alternative-sleep-function-in-c-to-milliseconds
// int msleep(long msec);
// With this function, I check ram_size, because ram size should be number of power of 2
bool is_pow_of_two(uint32_t n);
// https://stackoverflow.com/questions/7684359/how-to-use-nanosleep-in-c-what-are-tim-tv-sec-and-tim-tv-nsec
int mssleep(uint64_t miliseconds);

int main(const int argc, const char *argv[])
{
	// check number of arguments
    if (argc != NUMBER_OF_ARGS)
    {
        errx(1, "Enter only one argument!");
    }

    // check if file is .orc
    int st_size_of_argv1 = strlen(argv[1]);
    for (int i = 0; i < st_size_of_argv1; i++)
    {
        if (argv[1][i] == '.')
        {
            char letter_O = argv[1][i + 1];
            char letter_R = argv[1][i + 2];
            char letter_C = argv[1][i + 3];
            if (letter_O != 'o' || letter_R != 'r' || letter_C != 'c')
            {
                err(2, "File format is wrong!");
            }
        }
    }

    int fd;
    if ((fd = open(argv[1], O_RDONLY)) == -1)
    {
        err(3, "Open failed!");
    }

    char ORC[3];
    int ORC_read_stat;
    if ((ORC_read_stat = read(fd, &ORC, sizeof(ORC))) != sizeof(ORC))
    {
        err(5, "Read failed!");
    }
    if (ORC_read_stat == -1)
    {
        err(5, "Read failed!");
    }
    // check first 3 bytes of the program
    char ORC_CHECK[3] = {'O', 'R', 'C'};
    if (ORC[0] != ORC_CHECK[0] || ORC[1] != ORC_CHECK[1] || ORC[2] != ORC_CHECK[2])
    {
        err(66, "First 3 bytes of the program must bre O,R,C!");
    }

    uint32_t ram_size;
    int ram_size_read_stat;
    if ((ram_size_read_stat = read(fd, &ram_size, sizeof(ram_size))) != sizeof(ram_size))
    {
        err(5, "Read failed!");
    }
    if (ram_size_read_stat == -1)
    {
        err(5, "Read failed!");
    }
    if (!is_pow_of_two(ram_size))
    {
        err(42, "Ram size is not power by 2!");
    }
    struct stat st;
    if (fstat(fd, &st) == -1)
    {
        err(4, "Stat failed!");
    }
    // check file format
    if ((st.st_size - sizeof(ram_size) - sizeof(ORC)) % 25 != 0)
    {
        err(42, "File format is wrong!");
    }

    // here is the link, where i find how to check free ram size in bytes
    // https://lynxbee.com/c-program-to-check-total-and-free-ram-memory-in-linux/#.YpC-PchBy3B
    struct sysinfo info;
    if (sysinfo(&info) == -1)
    {
        err(99, "There isn't enough free ram size!");
    }
    // check is ram enough
    if (info.freeram < ram_size)
    {
        err(73, "There isn't enough free ram size to start the progrtam!");
    }
    int ins_read_stat;
    bool flag = false;
    int write_stat;
    char symbol;
    uint64_t tar, val, to, from_ptr, to_ptr, from, idx, v, res, v1, v2;
	int lseek_pos;
	// make struct for reading the file
    struct instructions
    {
        uint8_t opcode;
        uint64_t op1;
        uint64_t op2;
        uint64_t op3;
    } __attribute__((packed));
    struct instructions ins;
	// create an array with size = ram_size
    uint64_t *arr = (uint64_t *)malloc(ram_size * sizeof(uint64_t));
    if (arr == NULL)
    {
        err(11, "Memory not allocated!");
    }
	// start reading the file
    while ((ins_read_stat = read(fd, &ins, sizeof(ins)) > 0))
    {
        if (flag == true)
        {
            flag = false;
            continue;
        }

        if (ins.opcode == 0x00) // nop
        {
            continue;
        }
        else if (ins.opcode == 0x95) // set
        {
            tar = ins.op1;
            val = ins.op2;
            if (tar >= ram_size)
            {
                err(33, "Invalid address!");
            }
            arr[tar] = val;
        }
        else if (ins.opcode == 0x5d) // load
        {
            to = ins.op1;
            from_ptr = ins.op2;
            if (to >= ram_size || from_ptr >= ram_size || arr[from_ptr] >= ram_size)
            {
                err(33, "Invalid address!");
            }
            arr[to] = arr[arr[from_ptr]];
        }
        else if (ins.opcode == 0x63) // store
        {
            to_ptr = ins.op1;
            from = ins.op2;
            if (to_ptr >= ram_size || from >= ram_size || arr[to_ptr] >= ram_size)
            {
                err(33, "Invalid address!");
            }
            arr[arr[to_ptr]] = arr[from];
        }
        else if (ins.opcode == 0x91) // jmp
        {
            idx = ins.op1;
            if (idx >= ram_size || arr[idx] >= ram_size)
            {
                err(33, "Invalid address!");
            }
            lseek_pos = (arr[idx] * sizeof(ins)) + sizeof(ORC) + sizeof(ram_size);
            if ((lseek(fd, lseek_pos, SEEK_SET)) == -1)
            {
                err(88, "Lseek failed!");
            }
        }
        else if (ins.opcode == 0x25) // sgz
        {
            v = ins.op1;
            if (v >= ram_size)
            {
                err(33, "Invalid address!");
            }

            if (arr[v] > 0)
            {
                flag = true;
            }
        }
        else if (ins.opcode == 0xAD) // add
        {
            res = ins.op1;
            v1 = ins.op2;
            v2 = ins.op3;
            if (res >= ram_size || v1 >= ram_size || v2 >= ram_size)
            {
                err(33, "Invalid address!");
            }
            arr[res] = arr[v1] + arr[v2];
        }
        else if (ins.opcode == 0x33) // mul
        {
            res = ins.op1;
            v1 = ins.op2;
            v2 = ins.op3;
            if (res >= ram_size || v1 >= ram_size || v2 >= ram_size)
            {
                err(33, "Invalid address!");
            }
            arr[res] = arr[v1] * arr[v2];
        }
        else if (ins.opcode == 0x04) // div
        {
            res = ins.op1;
            v1 = ins.op2;
            v2 = ins.op3;
            if (res >= ram_size || v1 >= ram_size || v2 >= ram_size)
            {
                err(33, "Invalid address!");
            }
            if (arr[v2] == 0)
            {
                err(77, "Can't divide by 0!");
            }
            arr[res] = arr[v1] / arr[v2];
        }
        else if (ins.opcode == 0xB5) // mod
        {
            res = ins.op1;
            v1 = ins.op2;
            v2 = ins.op3;
            if (res >= ram_size || v1 >= ram_size || v2 >= ram_size)
            {
                err(33, "Invalid address!");
            }
            if (arr[v2] == 0)
            {
                err(77, "Can't divide by 0!");
            }
            arr[res] = arr[v1] % arr[v2];
        }
        else if (ins.opcode == 0xC1) // out
        {
            v = ins.op1;
            if (v >= ram_size)
            {
                err(55, "Out failed!");
            }
            symbol = arr[v];

            if ((write_stat = write(1, &symbol, sizeof(symbol))) != sizeof(symbol))
            {
                err(44, "Write of symbol failed!");
            }
            if (write_stat == -1)
            {
                err(44, "Write of symbol failed!");
            }
        }
        else if (ins.opcode == 0xBF) // sleep
        {
            v = ins.op1;
            if (mssleep(arr[v]) == -1)
            {
                err(200, "Sleep failed!");
            }
        }
        else // wrong instruction
        {
            err(22, "Wrong instruction!");
        }
    }
    if (ins_read_stat == -1)
    {
        err(5, "Read failed!");
    }
    close(fd);
    free(arr);
    exit(0);
}
/*
int msleep(long msec)
{
    struct timespec ts;
    int res;

    if (msec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do
    {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}*/
int mssleep(uint64_t miliseconds)
{
   struct timespec rem;
   struct timespec req= {
       (int)(miliseconds / 1000),     /* secs (Must be Non-Negative) */
       (miliseconds % 1000) * 1000000 /* nano (Must be in range of 0 to 999999999) */
   };

   return nanosleep(&req , &rem);
}
bool is_pow_of_two(uint32_t n)
{
    if (n == 0)
        return 0;
    while (n != 1)
    {
        if (n % 2 != 0)
            return 0;
        n = n / 2;
    }
    return 1;
}
