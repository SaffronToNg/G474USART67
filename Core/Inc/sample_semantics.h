#ifndef __SAMPLE_SEMANTICS_H__
#define __SAMPLE_SEMANTICS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/*
 * 最小采样语义层：只建立“DMA 缓冲区索引 -> 物理含义”的对应关系，
 * 暂不引入旧工程的平均值、偏置、控制量和状态机。
 */

typedef struct
{
  uint16_t il_raw;
  uint16_t vy_raw;
  uint16_t iy_raw;
  uint16_t vx_raw;
  uint16_t ix_raw;
  uint16_t vadj_raw;
  uint16_t iadj_raw;
  uint16_t temp_raw;
} SampleSemanticRaw_t;

extern volatile SampleSemanticRaw_t g_sample_semantic_raw;

void SampleSemantic_UpdateFromBuffers(void);

#ifdef __cplusplus
}
#endif

#endif /* __SAMPLE_SEMANTICS_H__ */
