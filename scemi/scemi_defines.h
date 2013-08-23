
#ifndef _svc_defines_h
#define _svc_defines_h

#include "svdpi.h"

#define RWBUF_SIZE (1<<10)

#define SCMD_CONNECT_PATH ((uint32_t)0)
     /* SCMD_CONNECT_PATH NUM_ARGS #uid bytes_per_element direction*/
#define SCMD_DATA         ((uint32_t)1)
     /* SCMD_DATA NUM_ARGS #uid __VA_ARGS__ */
#define SCMD_DATA_EOM     ((uint32_t)2)
     /* SCMD_DATA NUM_ARGS #uid */
#define SCMD_FINISH       ((uint32_t)3)
     /* SCMD_DATA NUM_ARGS */

#if 0
#define DBG_PRINT(...) fprintf(stderr, __VA_ARGS__)
#else
#define DBG_PRINT(...)
#endif
#define ERROR(...) fprintf(stderr, __VA_ARGS__)

#ifdef  SVC_DUT
#	define SVC_FIFO   "./svc_dut.f"
#	define SVC_FIFO_R "./svc_tb.f"
#	define SVC_SV_SCOPE "scemi_pipes_dut"
#else
#	define SVC_FIFO_R "./svc_dut.f"
#	define SVC_FIFO   "./svc_tb.f"
#	define SVC_SV_SCOPE "scemi_pipes_tb"
#endif
#define SVC_SVP_SCOPE "scemi_utils"

#ifdef __cplusplus
extern "C" {
#endif

void scemi_data_avail_trigger(void);
uint32_t scemi_pipe_pending_data(void *pipe_handle);

void scemi_pipe_c_send_data(void *pipe_handle, uint32_t data);
void scemi_pipe_c_send_nodata( void *pipe_handle, int num_elements, svBit eom );
uint32_t scemi_pipe_c_receive_data(void *pipe_handle);
void scemi_pipe_c_receive_nodata( void *pipe_handle, int num_elements, int *num_elements_valid, svBit *eom );
char *hash2string(int unsigned hash);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

