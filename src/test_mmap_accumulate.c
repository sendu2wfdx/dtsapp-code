#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#define DEFAULT_MAP_SIZE     (100 * 1024 * 1024)  // 映射 100MB，与你实际 buf 大小对应
#define PAGE_SIZE            4096
#define READ_WORDS           65535 * 2                // 打印前 2 个 Batch uint32_t 结果

int main(int argc, char *argv[])
{
    int fd,map_size;
    void *map_base;
    off_t vm_pgoff = 0; // 默认映射 buf0

    // 支持参数 --offset=1 切换到 buf1
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "--offset=1", 10) == 0)
            vm_pgoff = 1;
            map_size = 512 * 1024;
    }

    fd = open("/dev/accumulate", O_RDWR | O_SYNC);
    if (fd < 0) {
        perror("open /dev/accumulate");
        return 1;
    }

    map_base = mmap(NULL,
                    map_size,
                    PROT_READ | PROT_WRITE,
                    MAP_SHARED,
                    fd,
                    vm_pgoff * (PAGE_SIZE)); // 注意 offset 是页为单位

    if (map_base == MAP_FAILED) {
        perror("mmap failed");
        close(fd);
        return 1;
    }

    printf("成功映射 /dev/accumulate (buf%ld)，虚拟地址 = %p\n", vm_pgoff, map_base);

    uint32_t *data = (uint32_t *)map_base;

    printf("前 %d 个 uint32_t 累加结果如下：\n", READ_WORDS);
    for (int i = 0; i < READ_WORDS; ++i) {
        printf("  [%03d] = 0x%08X\n", i, data[i]);
    }

    munmap(map_base, DEFAULT_MAP_SIZE);
    close(fd);
    return 0;
}
