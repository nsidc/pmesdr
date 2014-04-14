#define OMAX 3  /* max number of output channels */
#define DEPTH 20 /* max depth of processing stack */

typedef struct {
  int command_code;
  float *pointer;
  float value;
  float nodata;
  float in_nodata;
} rpn_entry;

int get_input_preprocess_lines(char **argv, int narg, char *command_lines[], 
			       float out_anodata[], float anodata, int debug);

int set_image_preprocess(char *command_lines[], int nchans, int rpn_depth[],
			 rpn_entry rpn_list[][DEPTH], 
			 int ninputs, float *in_stval[], float in_anodata[], 
			 float out_anodata[], int debug);

float *image_preprocess(rpn_entry rpn_list[], int rmp_depth, int n, float anodata, int debug);

void free_input_arrays(float *stval[], int nchans, float *in_stval[], int ninputs);

