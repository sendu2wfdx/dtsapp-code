# User Guide

### 调试环境：

VScode + MobaXterm

##### VScode：

SSH连接，编辑代码（需安装SSH插件）

https://code.visualstudio.com/Download

##### MobaXterm：

TTL / SSH连接，虚拟终端 + 文件传输

https://github.com/RipplePiam/MobaXterm-Chinese-Simplified/releases/tag/v0.24.3.10

用户名：

```scss
ubuntu
```

密码：

```scss
temppwd
```



### 软硬件参数：

| 项目      | 参数                           |
| --------- | ------------------------------ |
| CPU       | Cortex A9 *2 XC7Z020-2CLG400   |
| Memory    | **1GB** MT41K256M16 RE-125     |
| EMMC      | **8GB** XC7Z020-2CLG400        |
| QFLASH    | **16MB** N25Q128A13ESE40F      |
| Kernel    | Linux arm 6.1.5-xilinx-v2023.1 |
| ROOTFS    | Ubuntu 20.04.6 LTS             |
| Username  | **ubuntu**                     |
| Password  | **temppwd**                    |
| Baud rate | 115200                         |



### 驱动需求：

### 1、普通硬件设备驱动：

#### 1.1、串口

| 物理接口 | 对应设备文件 | FPGA PIN                                                |
| -------- | ------------ | ------------------------------------------------------- |
| USB UART | /dev/ttyPS0  | TX MIO48<br />RX MIO49<br />**连接到 PS UART**          |
| RS485 0  | /dev/ttyUL1  | TX J20<br />RX H20<br />DE G18<br />**连接到 RS485_0**  |
| RS485 1  | /dev/ttyUL2  | TX B19<br />RX A20<br />DE G19 <br />**连接到 RS485_1** |
| RS232 0  | /dev/ttyUL3  | TX J18<br />RX G17<br />**连接到 PL UART**              |
| RS232 1  | /dev/ttyUL4  | TX G20<br />RX H18<br />**连接到 普通IO**               |

> [!NOTE]
>
> RS485的 **DE** 引脚仅 MLK_F3 开发板需要，项目底板自带硬件流控，无需操作。

###### 使用方法：

打开对应设备文件操作即可，485需要手动流控，使用的GPIO接在：**/dev/gpiochip1**。



#### 1.2、GPIO

使用 CEP2 摄像头接口（J6）

> [!NOTE]
>
> 直接配置为10个 **GPIO**，输入输出均可。
>
> **sysfs** 接口:
>
> /sys/class/gpio/gpiochip1012（不推荐使用）
>
> **chardev** 接口：
>
> /dev/gpiochip1可用 gpiod 控制


| 物理接口 | FPGA PIN |
| -------- | -------- |
| GPIO 0   | U7       |
| GPIO 1   | V7       |
| GPIO 2   | V6       |
| GPIO 3   | W6       |
| GPIO 4   | V11      |
| GPIO 5   | V10      |
| GPIO 6   | T5       |
| GPIO 7   | U5       |
| GPIO 8   | T9       |
| GPIO 9   | U10      |

###### 使用方法：

安装库与依赖：

```scss
sudo apt install  libgpiod-dev gpiod
```

可用命令：

| 命令       | 作用                     | 使用举例           | 说明                         |
| ---------- | ------------------------ | ------------------ | ---------------------------- |
| gpiodetect | 列出所有的GPIO控制器     | sudo gpiodetect    | 列出所有的GPIO控制器         |
| gpioinfo   | 列出gpio控制器的引脚情况 | sudo gpioinfo 1    | 列出gpio1控制器引脚组情况    |
| gpioset    | 设置gpio                 | sudo gpioset 1 2=0 | 设置gpio1组编号2引脚为低电平 |
| gpioget    | 获取gpio引脚状态         | sudo gpioget 1 2   | 获取gpio1组编号2的引脚状态   |
| gpiomon    | 监控gpio的状态           | sudo gpiomon 1 2   | 监控gpio1组编号2的引脚状态   |

> [!TIP]
>
> DS18B20 暂时挂接在 GPIO0（U7），除该引脚外均可用。



#### 1.3、网口

##### PHY:YT8531C

**eth0**，连接到物理接口 **ETH**

###### 使用方法：

略



#### 1.4、传感器

##### TERM:DS18B20 

温度传感器，暂时连接到 PL **GPIO0** (U7)

###### 使用方法：


查看单总线设备：

```scss
ls /sys/bus/w1/devices
```

输出如下：

```scss
ubuntu@arm:/sys/bus/w1/devices$ ls
28-0000003a9f53  w1_bus_master1
```

其中， **28-0000003a9f53 **即为设备名称。

> [!NOTE]
>
> 设备名称根据 DS18B20 **唯一ID** 确定，并不唯一

查看温度：

```scss
cat /sys/bus/w1/devices/28-0000003a9f53/temperature
```

输出：

```scss
ubuntu@arm:/sys/bus/w1/devices/28-0000003a9f53$ cat /sys/bus/w1/devices/28-0000003a9f53/temperature
22125
```

表示温度为22.125℃



#### 1.5、IIC总线设备

##### 总线接口：

**SCL**：G15

**SDA**：H15

##### RTC:DS1337

###### 使用方法：

获取RTC：

```scss
sudo hwclock --show
```

输出：

```scss
ubuntu@arm:/dev$ sudo hwclock --show
2025-07-01 09:39:00.011041+00:00
```

获取网络时间：

```scss
sudo timedatectl set-ntp true
sudo systemctl restart systemd-timesyncd
```

将系统时间写入硬件时钟：

```scss
sudo hwclock --systohc
```

##### EEPROM:M24C02

###### 使用方法：

读 EEPROM：


```scss
sudo hexdump -C /sys/bus/i2c/devices/0-0051/eeprom
```

写 ASCII：

```scss
echo "EEPROM TEST." > /sys/bus/i2c/devices/0-0051/eeprom
```

写 二进制：

```scss
printf "\x48\x45\x4C\x4C\x4F" | sudo tee /sys/bus/i2c/devices/0-0051/eeprom > /dev/null
```

（写入 `HELLO`（对应 ASCII）。）



#### 1.6、QSPI FLASH

##### QFLASH:N25Q128A13

###### 使用方法：

检查分区：

```scss
cat /proc/mtd
```

输出：

```scss
ubuntu@arm:~$ cat /proc/mtd
dev:    size   erasesize  name
mtd0: 00a00000 00010000 "qspi-boot"
mtd1: 00600000 00010000 "qspi-kernel"
mtd2: 00000000 00010000 "qspi-bootenv"
```

> [!CAUTION]
>
> 用于引导操作系统，用户禁止修改



#### 1.7、SD

##### SD0

SD卡槽

##### SD1

核心板板载 EMMC


> [!NOTE]
>
> 所有 **PINOUT** 仅适用于 **MLK_F3**，且仅包含需求中的必要设备
> USB、CAN也可用，没有介绍



### 2、自定义设备驱动

#### 2.1、光路开关（用GPIO即可，暂时没写，省略）

**用户输入：**

选择的光路（0~16）

**驱动输出：**

开关状态

**驱动实现的功能：**

文件操作、光路控制逻辑

#### 2.2、ADS4229（仅实现所有基础功能）

**用户输入：**

光纤长度、采样间隔长度、累加次数

启动信号、初始化信号

**驱动输出：**

设备状态、累加结果

**驱动实现的功能：**

文件操作、内存管理、参数计算、启动采集、结束采集、初始化设备

以上设备需要BD配置、编译内核、修改设备树、编写驱动。



##### 累加器缓存区定义：

| BUF  | 大小      | 内容                                                         |
| ---- | --------- | ------------------------------------------------------------ |
| BUF0 | **100MB** | adc0，adc1 的各个 **Batch** 结果（共 200 +200 个Batch，自动覆盖） |
| BUF1 | **512KB** | adc0，adc1 的 **SUM** 结果（需手动清空）                     |

###### BUF0存储方式：

BUF0_Base 由驱动获取并配置，用户态 mmap page0 即可，无需关心

OFFSET = 0x40000

| 区块名         | 起始地址             | 大小 / KB | 大小 / 点 |
| -------------- | -------------------- | --------- | --------- |
| adc 0 batch 0  | BUF0_Base + 0x00000  | 256KB     | 65536     |
| adc 1 batch 0  | BUF0_Base + 0x40000  | 256KB     | 65536     |
| adc 0 batch 1  | BUF0_Base + 0x80000  | 256KB     | 65536     |
| adc 1 batch 1  | BUF0_Base + 0xC0000  | 256KB     | 65536     |
| adc 0 batch 2  | BUF0_Base + 0x100000 | 256KB     | 65536     |
| adc 1 batch 2  | BUF0_Base + 0x140000 | 256KB     | 65536     |
| ......         | ......               | ......    | ......    |
| adc 0 batch 19 | BUF0_Base + 0x80000  | 256KB     | 65536     |
| adc 1 batch 19 | BUF0_Base + 0xC0000  | 256KB     | 65536     |

###### BUF0存储方式：

BUF1_Base 由驱动获取并配置，用户态 mmap page1 即可，无需关心

OFFSET = 0x40000

| 区块名    | 起始地址            | 大小 / MB | 大小 / 点 |
| --------- | ------------------- | --------- | --------- |
| adc 0 Sum | BUF1_Base + 0x00000 | 256KB     | 65535     |
| adc 1 Sum | BUF1_Base + 0x40000 | 256KB     | 65535     |



##### 驱动完成情况：

###### DMA 内存分配：

从内核CMA池中分配两个连续的 DMA 缓冲区，供硬件累加器访问。

###### 寄存器映射：

将设备物理寄存器映射到内核虚拟地址，用于启动设备、参数配置等。

###### mmap 接口：

将 DMA 缓冲区映射到用户空间，实现用户态高速访问累加结果。Batch 与 Sum 经过分区处理，可分别挂载。

###### ioctl 接口：

实现启动累加器、读写寄存器、清除 SUM 区域等控制功能（**设备配置功能待简化**）。

###### platform_driver：

通过设备树自动绑定硬件设备

###### miscdevice：

维护了一个 misc 类型的 chardev，通过 `/dev/accumulate` 暴露字符设备接口给用户空间。



##### 驱动使用方法：

驱动调试阶段手动加载的方式，便于随时调整。

###### 手动加载卸载驱动：

**accumulate.ko** 为编译好的内核模块

加载驱动：

```scss
sudo insmod accumulate.ko
```

读取内核日志：

```scss
dmesg | grep accumulate
```

成功挂载之后内核打印：

```scss
ubuntu@arm:~$ dmesg | grep accumulate
[ 4384.432227] accumulate: loading out-of-tree module taints kernel.
[ 4384.884078] accumulate 60000000.sample_top: accumulate: Device probed successfully.
[ 4384.884109] accumulate 60000000.sample_top: ACC registers mapped at 4be81e81
[ 4384.884130] accumulate 60000000.sample_top: BUF0 virt=da6e830c phys=0x38100000 size=83886080
[ 4384.884151] accumulate 60000000.sample_top: BUF1 virt=5e3e604c phys=0x3d100000 size=4194304
```

卸载驱动：

```scss
sudo rmmod accumulate.ko
```

成功卸载之后内核打印：

```scss
ubuntu@arm:~$ dmesg | grep accumulate
......
[ 4473.442797] accumulate: Device removed Successfully.
```

> [!NOTE]
>
> 读取近几条内核日志：
>
> ```scss
> dmesg | grep accumulate | tail -n 5
> ```



##### 用户态代码示例：

###### 编译方法：

安装 **build-essential** 软件包组：

```scss
sudo apt update
sudo apt install build-essential
```

使用 gcc 编译源代码：

```scss
gcc test_ioctl_accumulate.c -o test_ioctl_accumulate
```



###### 读写寄存器示例:

```scss
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
        .data_pre_sample        = 500000 - 1,
        .sample_pre_batch       = 50 - 1,
        .accumulator_batches    = 200 - 1,
        .trigger_delay          = 50 - 1,
        .data_delay             = 50,
        .data_delay_pre_slice   = 0,
        .ld_trigger_en          = 1,
        .ld_trigger_pulse_width = 5,
    };

    // 读取初始寄存器值
    printf("寄存器初始值\n");
    dump_regs(fd, 0x00, 0x30);
    
    // 清空BUF1
    printf("清空 BUF1 ...\n");
    ioctl(fd, ACC_IOC_CLEAR_BUF1);

    // 写入配置
    printf("配置寄存器...\n");
    configure_acc(fd, &cfg);

    // 再次读取
    printf("寄存器当前值\n");
    dump_regs(fd, 0x00, 0x30);

    // 启动累加器
    printf("启动累加器...\n");
    // ioctl_write(fd, 0x00, (1 << 1) | 1); // result_sum_en << 1 | start
    ioctl(fd, ACC_IOC_START);

    // 等待完成
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

```

###### 读取BUF区域示例：

```scss
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

```

###### 导出BUF结果为csv文件示例：

```scss
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>

#define ACC_IOC_MAGIC   'A'
#define ACC_IOC_START   _IO(ACC_IOC_MAGIC, 1)   // 用户态可用的累加器启动命令
#define ACC_IOC_WRITE_REG _IOW(ACC_IOC_MAGIC, 2, struct acc_reg_cfg)    // 用户态可用的寄存器写入命令
#define ACC_IOC_READ_REG   _IOR(ACC_IOC_MAGIC, 3, struct acc_reg_cfg)   // 用户态可用的寄存器读取命令
#define ACC_IOC_CLEAR_BUF1 _IO(ACC_IOC_MAGIC, 4)    // 用户态可用的清除 buf1 命令

#define PAGE_SIZE         4096
#define BUF0_OFFSET       0
#define BUF1_OFFSET       1
#define MAP_SIZE_BUF0     (100 * 1024 * 1024)  // 100MB
#define MAP_SIZE_BUF1     (512 * 1024)   // 512KB

// 配置参数
#define DATA_PRE_SAMPLE         (50000 - 1)
#define SAMPLE_PRE_BATCH        (50 - 1)
#define ACCUMULATOR_BATCHES     (200 - 1)
#define TRIGGER_DELAY           (50 - 1)
#define DATA_DELAY_PRE_SLICE    0
#define DATA_DELAY              50
#define LD_TRIGGER_EN           1
#define LD_TRIGGER_PULSE_WIDTH  1
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

int save_to_csv(const char* filename, uint32_t* data, size_t count) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("fopen csv");
        return -1;
    }

    for (size_t i = 0; i < count; i++) {
        fprintf(fp, "%u\n", data[i]);
    }

    fclose(fp);
    printf("保存 %zu 个值到 %s 成功\n", count, filename);
    return 0;
}

int main(int argc, char *argv[])
{
    size_t dump_count_buf0 = 65535 * 400;
    size_t dump_count_buf1 = 65535 * 2;

    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "--buf0=", 7) == 0)
            dump_count_buf0 = atoi(argv[i] + 7);
        else if (strncmp(argv[i], "--buf1=", 7) == 0)
            dump_count_buf1 = atoi(argv[i] + 7);
    }

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

    // Step 2: mmap buf1 清空
    printf("清空 buf1 区域...\n");
    uint32_t* buf1 = map_dma_buffer(fd, BUF1_OFFSET, MAP_SIZE_BUF1);
    if (!buf1) { close(fd); return 1; }
    memset(buf1, 0, MAP_SIZE_BUF1);
    msync(buf1, MAP_SIZE_BUF1, MS_SYNC); // 确保写回
    munmap(buf1, MAP_SIZE_BUF1); // 可以不卸载，也可以保留

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

    // Step 5: 重新 mmap buf0/buf1 并保存
    uint32_t* buf0 = map_dma_buffer(fd, BUF0_OFFSET, MAP_SIZE_BUF0);
    buf1 = map_dma_buffer(fd, BUF1_OFFSET, MAP_SIZE_BUF1);
    if (!buf0 || !buf1) {
        close(fd);
        return 1;
    }

    save_to_csv("buf0.csv", buf0, dump_count_buf0);
    save_to_csv("buf1.csv", buf1, dump_count_buf1);

    // 清理
    munmap(buf0, MAP_SIZE_BUF0);
    munmap(buf1, MAP_SIZE_BUF1);
    close(fd);
    return 0;
}

```

