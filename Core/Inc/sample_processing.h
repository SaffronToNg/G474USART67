#ifndef __SAMPLE_PROCESSING_H__
#define __SAMPLE_PROCESSING_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "sample_semantics.h"

typedef struct
{
  int32_t vx_raw;
  int32_t vx_avg;
  int32_t ix_raw;
  int32_t ix_avg;
  int32_t ix_offset;
  int32_t ix_zero_based;
  int32_t vy_raw;
  int32_t vy_avg;
  int32_t iy_raw;
  int32_t iy_avg;
  int32_t iy_offset;
  int32_t iy_zero_based;
  int32_t il_raw;
  int32_t il_avg;
  int32_t il_offset;
  int32_t il_zero_based;
  int32_t vadj_raw;
  int32_t vadj_avg;
  int32_t iadj_raw;
  int32_t iadj_avg;
  int32_t temp_raw;
  int32_t temp_avg;
  uint16_t offset_sample_count;
  uint8_t offset_ready;
} SampleProcessed_t;

extern volatile SampleProcessed_t g_sample_processed;

void SampleProcessing_Update(void);

#ifdef __cplusplus
}
#endif

#endif /* __SAMPLE_PROCESSING_H__ */
