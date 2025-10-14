#include "private.h"
#include "radar_algo.h"

static uint8_t *radar_frame[2] = {NULL, NULL};

/**
 * @brief 雷达数据帧解析
 **/
static int frame_prepare_parse(sample_frame_t *frame)
{
    if ((frame != NULL)
        && (frame->frame_head == 0x55AA55BB)
        && (frame->frame_tail == 0x55CC55DD)
        && (frame->data_bytes <= DATA_FIELD_BYTES))   
    {
        return 0;

    }

    return -1;
}

/**
 * @brief 雷达数据接收回调
*/
static void radar_frame_callback(const uint8_t buf_id, const void *frame, const uint32_t bytes)
{
    if (frame_prepare_parse((sample_frame_t *)frame) == 0) {
        bsp_comm_frame((const uint8_t *)frame, bytes);

        //sample_frame_t *dframe = (sample_frame_t *)frame;
        //bsp_log_info("index: %d, head: %X, tail: %X, dbytes: %d, chirps: %d, points: %d", dframe->frame_index, dframe->frame_head, dframe->frame_tail, dframe->data_bytes, dframe->samp_chirps, dframe->samp_points);
    }
}

/**
 * 函数名称: radar_algo_probe
 * 功能描述: 雷达算法注册 公共接口函数。 用于初始化雷达算法模块所需资源并注册回调函数。
 * 输入参数: 无
 * 输出参数: 无
 * 返回说明: 无
 */
void radar_algo_probe(void)
{
    radar_frame[0] = bsp_mem_malloc(SAMPLE_FRAME_SIZE);
    if (radar_frame[0] == NULL) {
        bsp_log_error("radar frame ping malloc failed");
    }

    radar_frame[1] = bsp_mem_malloc(SAMPLE_FRAME_SIZE);
    if (radar_frame[1] == NULL) {
        bsp_log_error("radar frame pong malloc failed");
    }
    
    bsp_radar_frame_init((const void *)radar_frame[0], (const void *)radar_frame[1], SAMPLE_FRAME_SIZE, radar_frame_callback);
}

/**
 * 函数名称: radar_algo_manager
 * 功能描述: 雷达算法管理，处理上一帧的 RDM 数据，并触发下一帧采集
 * 输入参数: 无
 * 输出参数: 无
 * 返回说明: 无
 */
void radar_algo_manager(void)
{
    
}

