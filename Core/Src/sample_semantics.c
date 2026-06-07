#include "sample_semantics.h"

extern __IO uint16_t g_adc1_dma_buffer[5];
extern __IO uint16_t g_adc2_dma_buffer[3];

volatile SampleSemanticRaw_t g_sample_semantic_raw = {0};

void SampleSemantic_UpdateFromBuffers(void)
{
  /*
   * 旧 F3 工程的语义映射来自 function.c::ADCSample()：
   * ADC1_RESULT[0] -> IL
   * ADC1_RESULT[1] -> Vy
   * ADC1_RESULT[2] -> Iy
   * ADC1_RESULT[3] -> Vx
   * ADC1_RESULT[4] -> Ix
   * ADC2_RESULT[0] -> Vadj
   * ADC2_RESULT[1] -> Iadj
   * ADC2_RESULT[2] -> Temp
   */
  g_sample_semantic_raw.il_raw = g_adc1_dma_buffer[0];
  g_sample_semantic_raw.vy_raw = g_adc1_dma_buffer[1];
  g_sample_semantic_raw.iy_raw = g_adc1_dma_buffer[2];
  g_sample_semantic_raw.vx_raw = g_adc1_dma_buffer[3];
  g_sample_semantic_raw.ix_raw = g_adc1_dma_buffer[4];

  g_sample_semantic_raw.vadj_raw = g_adc2_dma_buffer[0];
  g_sample_semantic_raw.iadj_raw = g_adc2_dma_buffer[1];
  g_sample_semantic_raw.temp_raw = g_adc2_dma_buffer[2];
}
