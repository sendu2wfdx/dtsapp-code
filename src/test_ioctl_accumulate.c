#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>

#define ACC_IOC_MAGIC   'A'
#define ACC_IOC_START   _IO(ACC_IOC_MAGIC, 1)   // 用户态可用的累加器启动命令
#define ACC_IOC_WRITE_REG _IOW(ACC_IOC_MAGIC, 2, struct acc_reg_cfg)    // 用户态可用的寄存器写入命令
#define ACC_IOC_READ_REG   _IOR(ACC_IOC_MAGIC, 3, struct acc_reg_cfg)   // 用户态可用的寄存器读取命令
#define ACC_IOC_CLEAR_BUF1 _IO(ACC_IOC_MAGIC, 4)    // 用户态可用的清除 buf1 命令

struct acc_reg_cfg {
    uint32_t offset;
    uint32_t value;
};

// 累加器配置结构体
struct acc_config {
    uint32_t data_pre_sample;
    uint32_t sample_pre_batch;
    uint32_t accumulator_batches;
    uint32_t trigger_delay;
    uint32_t data_delay_pre_slice;
    uint32_t data_delay;
    uint32_t ld_trigger_en;
    uint32_t ld_trigger_pulse_width;
};

// 写寄存器函数封装
int ioctl_write(int fd, uint32_t offset, uint32_t value) {
    struct acc_reg_cfg cfg = { .offset = offset, .value = value };
    return ioctl(fd, ACC_IOC_WRITE_REG, &cfg);
}

// 读寄存器函数封装
int ioctl_read(int fd, uint32_t offset, uint32_t *value) {
    struct acc_reg_cfg cfg = { .offset = offset };
    int ret = ioctl(fd, ACC_IOC_READ_REG, &cfg);
    if (ret == 0) *value = cfg.value;
    return ret;
}

// 打印寄存器
void dump_regs(int fd, uint32_t start, uint32_t end) {
    printf("==== Dump Registers 0x%02X ~ 0x%02X ====\n", start, end);
    for (uint32_t off = start; off <= end; off += 4) {
        uint32_t val = 0;
        if (ioctl_read(fd, off, &val) == 0)
            printf("  [0x%02X] = 0x%08X\n", off, val);
        else
            printf("  [0x%02X] = <Read Error>\n", off);
    }
    printf("========================================\n\n");
}

// 使用结构体配置所有寄存器
void configure_acc(int fd, const struct acc_config *cfg) {
    ioctl_write(fd, 0x04, cfg->data_pre_sample);
    ioctl_write(fd, 0x10, cfg->sample_pre_batch);
    ioctl_write(fd, 0x18, cfg->accumulator_batches);
    ioctl_write(fd, 0x1C, cfg->trigger_delay);
    ioctl_write(fd, 0x20, (cfg->data_delay_pre_slice << 24) | (cfg->data_delay & 0xFFFFFF));
    ioctl_write(fd, 0x24, (cfg->ld_trigger_en << 16) | (cfg->ld_trigger_pulse_width & 0xFFFF));
}

int main()
{
    int fd = open("/dev/accumulate", O_RDWR);
    if (fd < 0) {
        perror("open /dev/accumulate");
        return 1;
    }

    // 可调配置项
    struct acc_config cfg = {
        .data_pre_sample        = 50000 - 1,
        .sample_pre_batch       = 50 - 1,
        .accumulator_batches    = 200 - 1,
        .trigger_delay          = 10 - 1,
        .data_delay             = 10,
        .data_delay_pre_slice   = 0,
        .ld_trigger_en          = 1,
        .ld_trigger_pulse_width = 5,
    };

    printf("清空BUF1...\n");
    ioctl(fd, ACC_IOC_CLEAR_BUF1);

    printf("寄存器初始值\n");
    dump_regs(fd, 0x00, 0x30);

    printf("配置寄存器...\n");
    configure_acc(fd, &cfg);

    printf("寄存器当前值\n");
    dump_regs(fd, 0x00, 0x30);

    printf("启动累加器...\n");
    // ioctl_write(fd, 0x00, (1 << 1) | 1); // result_sum_en << 1 | start
    ioctl(fd, ACC_IOC_START);

    printf("等待累加完成...\n");
    uint32_t status;
    do {
        ioctl_read(fd, 0x00, &status);
        usleep(1000);
    } while (status & 0x01);

    printf("完成，最终状态寄存器 = 0x%08X\n", status);
    close(fd);
    return 0;
}
