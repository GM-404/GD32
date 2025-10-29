#ifndef RADAR_PRIVATE_INCLUDE
#define RADAR_PRIVATE_INCLUDE

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/********************** 雷达数据 **********************/
#define RADAR_DATA_BYTE                     (1)         /* ADC --> 1字节; 时域伪浮点/1DFFT/2DFFT --> 4字节; CFAR/点云 --> 8字节 */
#define RADAR_ANT_COUNT                     (2)         /* 采样天线数量 */
#define RADAR_CHIRP_COUNT                   (64)        /* 采样chirp数 */
#define RADAR_CHIRP_POINTS                  (128)       /* 采样点数 */
#define DATA_FIELD_BYTES                    (RADAR_DATA_BYTE*RADAR_ANT_COUNT*RADAR_CHIRP_POINTS*RADAR_CHIRP_COUNT)

#define RANGE_BINS       (20)  

/* 雷达采样信息结构体 */
typedef struct {
    uint32_t frame_head;                    /* 帧头(0x55AA55BB) */

    uint32_t frame_index;                   /* 雷达推送帧索引 */

    uint16_t curr_pack;                     /* 当前包序号 */
    uint16_t total_pack;                    /* 总数据包数 */

    uint8_t  data_type;                     /* 数据帧类型(0:int8_t双极性ADC; 1:时域伪浮点; 2:1DFFT伪浮点; 3:2DFFT伪浮点; 4:CFAR; 5:点云) */
    uint8_t  samp_ants;                     /* 采样天线数 */
    uint16_t data_crc16;                    /* 数据源CRC16 */

    uint16_t samp_points;                   /* 采样点数据 */
    uint16_t samp_chirps;                   /* 采样chirp数 */

    uint32_t data_bytes;                    /* 采样数据源字节数 */

    uint8_t  data[DATA_FIELD_BYTES];        /* 数据 */

    uint32_t frame_tail;                    /* 帧尾(0x55CC55DD) */
} __attribute__((aligned(4))) sample_frame_t;

#define SAMPLE_FRAME_SIZE                   sizeof(sample_frame_t)
/******************************************************/

/********************** 雷达配置参数 **********************/
/** @brief 打包后的雷达数据大小 */
#define PACKED_FRAME_SIZE   (RADAR_DATA_BYTE*RADAR_ANT_COUNT*RADAR_CHIRP_POINTS*RADAR_CHIRP_COUNT) +  sizeof(sample_frame_t)

/** @brief 输入的数据包 */
extern const uint8_t packed_frame_data[PACKED_FRAME_SIZE];  

/* CFAR 配置 */

/** @brief CFAR 幅度阈值 */
#define CONFIG_CFAR_TH_AMP 2.8

/** @brief CFAR 保护单元大小-距离维度 */
#define CONFIG_CFAR_NUM_GUARD_RANGE 5

/** @brief CFAR 保护单元大小-速度维度 */
#define CONFIG_CFAR_NUM_GUARD_VEL 3

/** @brief CFAR 训练单元大小-距离维度 */
#define CONFIG_CFAR_NUM_TRAIN_RANGE 6

/** @brief CFAR 训练单元大小-速度维度 */
#define CONFIG_CFAR_NUM_TRAIN_VEL 6

/** @brief CFAR 阈值*/
#define CONFIG_CFAR_TH_OFFSET 3


#ifdef __cplusplus
}
#endif

#endif
