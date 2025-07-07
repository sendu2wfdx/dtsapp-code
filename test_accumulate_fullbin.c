#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>

#define ACC_IOC_MAGIC     'A'
#define ACC_IOC_START     _IO(ACC_IOC_MAGIC, 1)
#define ACC_IOC_WRITE_REG _IOW(ACC_IOC_MAGIC, 2, struct acc_reg_cfg)
#define ACC_IOC_READ_REG  _IOR(ACC_IOC_MAGIC, 3, struct acc_reg_cfg)

#define PAGE_SIZE         4096
#define BUF0_OFFSET       0
#define BUF1_OFFSET       1
#define MAP_SIZE_BUF0     (100 * 1024 * 1024)  // 100MB
#define MAP_SIZE_BUF1     (512 * 1024)   // 512KB

// 配置参数
#define DATA_PRE_SAMPLE         (50000 - 1)
#define SAMPLE_PRE_BATCH        (50 - 1)
#define ACCUMULATOR_BATCHES     (200 - 1)
#define TRIGGER_DELAY           (10 - 1)
#define DATA_DELAY_PRE_SLICE    0
#define DATA_DELAY              10
#define LD_TRIGGER_EN           1
#define LD_TRIGGER_PULSE_WIDTH  5
#define RESULT_SUM_EN           1
#define ACCMULATOR_START        1

struct acc_reg_cfg {
    uint32_t offset;
    uint32_t value;
};

int ioctl_write(int fd, uint32_t offset, uint32_t value) {
    struct acc_reg_cfg cfg = { .offset = offset, .value = value };
    return ioctl(fd, ACC_IOC_WRITE_REG, &cfg);
}

int ioctl_read(int fd, uint32_t offset, uint32_t *value) {
    struct acc_reg_cfg cfg = { .offset = offset };
    int ret = ioctl(fd, ACC_IOC_READ_REG, &cfg);
    if (ret == 0) *value = cfg.value;
    return ret;
}

void* map_dma_buffer(int fd, off_t vm_pgoff, size_t size) {
    void *map_base = mmap(NULL,
                          size,
                          PROT_READ | PROT_WRITE,
                          MAP_SHARED,
                          fd,
                          vm_pgoff * PAGE_SIZE);
    if (map_base == MAP_FAILED) {
        perror("mmap failed");
        return NULL;
    }
    return map_base;
}

int save_to_bin_bytes(const char* filename, void* data, size_t bytes) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("fopen bin");
        return -1;
    }

    size_t written = fwrite(data, 1, bytes, fp);
    fclose(fp);

    if (written != bytes) {
        fprintf(stderr, "写入字节数不一致！预期 %zu，实际 %zu\n", bytes, written);
        return -1;
    }

    printf("保存 %zu 字节到 %s 成功\n", bytes, filename);
    return 0;
}

int main()
{
    int fd = open("/dev/accumulate", O_RDWR | O_SYNC);
    if (fd < 0) {
        perror("open /dev/accumulate");
        return 1;
    }

    // Step 1: 配置寄存器
    printf("配置累加器寄存器...\n");
    ioctl_write(fd, 0x04, DATA_PRE_SAMPLE);
    ioctl_write(fd, 0x10, SAMPLE_PRE_BATCH);
    ioctl_write(fd, 0x18, ACCUMULATOR_BATCHES);
    ioctl_write(fd, 0x1C, TRIGGER_DELAY);
    ioctl_write(fd, 0x20, (DATA_DELAY_PRE_SLICE << 24) | DATA_DELAY);
    ioctl_write(fd, 0x24, (LD_TRIGGER_EN << 16) | LD_TRIGGER_PULSE_WIDTH);

    // Step 2: 清空 buf1 区域
    printf("清空 buf1 区域...\n");
    uint32_t* buf1 = map_dma_buffer(fd, BUF1_OFFSET, MAP_SIZE_BUF1);
    if (!buf1) { close(fd); return 1; }
    memset(buf1, 0, MAP_SIZE_BUF1);
    msync(buf1, MAP_SIZE_BUF1, MS_SYNC);
    munmap(buf1, MAP_SIZE_BUF1);

    // Step 3: 启动累加器
    printf("启动累加器...\n");
    ioctl_write(fd, 0x00, (RESULT_SUM_EN << 1) | ACCMULATOR_START);

    // Step 4: 等待完成
    printf("等待累加完成...\n");
    uint32_t status;
    do {
        ioctl_read(fd, 0x00, &status);
        usleep(1000);
    } while (status & 0x01);
    printf("累加完成，状态寄存器 = 0x%08X\n", status);

    // Step 5: 映射 DMA buffer 并保存为二进制
    uint8_t* buf0 = map_dma_buffer(fd, BUF0_OFFSET, MAP_SIZE_BUF0);
    buf1 = map_dma_buffer(fd, BUF1_OFFSET, MAP_SIZE_BUF1);
    if (!buf0 || !buf1) {
        close(fd);
        return 1;
    }

    save_to_bin_bytes("buf0.bin", buf0, MAP_SIZE_BUF0);
    save_to_bin_bytes("buf1.bin", buf1, MAP_SIZE_BUF1);

    munmap(buf0, MAP_SIZE_BUF0);
    munmap(buf1, MAP_SIZE_BUF1);
    close(fd);
    return 0;
}
