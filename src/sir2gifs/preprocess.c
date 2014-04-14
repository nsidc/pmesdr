/* copyright 2000, 2003 by David Long, all rights reserved */
/*
   routines to enable input file preprocessing

   Written by DGL 1 April 2000
   Modified by DGL 1 March 2003 + added new options

*/


#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>


#include "preprocess.h"



int get_input_preprocess_lines(char **argv, int narg, char *command_lines[],
			       float out_anodata[], float anodata, int debug)
{
  FILE *fid;
  int i=1, n, ncnt=0;
  char line[200], line2[200], optname[200];
  float nodata;

  for (n=0; n<OMAX; n++)
    command_lines[n] = NULL;

  while (*(argv[i]) == '-' && i <= narg) {   /* optional argument */
    /* printf("preprocess: Checking input argument %d %s\n",i,argv[i]); */
    nodata = anodata;
    if (*(argv[i]+1) == 'f') {  /* check for file if optional argument is file input */
      sscanf(argv[i],"-f%s",optname);
      /* fprintf(stdout,"Opening options input file '%s'\n",optname); */
      fid=fopen(optname,"r");
      if (fid == NULL) {
	fprintf(stderr,"*** Could not open options file '%s' ***\n",optname);
	exit(-1);
      }
      while (fscanf(fid,"%s",line2) > 0) {
	if (debug) printf(" debug: options file line: %s\n",line2);
	if (line2[0] == '-' && line2[1] == 'P') {
	  if (line2[3] == '(') {  /* optional nodata value */
	    sscanf(line2,"-P	%d(%f)=%s",&n,&nodata,line);
	    if (strstr(line2,"=") == NULL) {
	      fprintf(stderr,"*** WARNING: no '=' in P command *** %s\n",line2);
	      sscanf(line2,"-P%d(%f)%s",&n,&nodata,line);
	    }
	  } else {  /* no optional nodata value */
	    sscanf(line2,"-P%d=%s",&n,line);
	    if (strstr(line2,"=") == NULL) {
	      fprintf(stderr,"*** WARNING: no '=' in P command *** %s\n",line2);
	      sscanf(line2,"-P%d%s",&n,line);
	    }
	  }
	  n--;
	  if (n < 0) n=0;
	  if (n > OMAX) n=OMAX-1;
	  if ((command_lines[n]=(char *) malloc(sizeof(char)*(6+strlen(line)))) == NULL) {
	    fprintf(stderr,"*** ERROR allocating input file name storage in get_input_preprocess_lines**\n");
	    exit(-1);
	  }
	  out_anodata[n] = nodata;
	  strcpy(command_lines[n],line);
	  ncnt = (n+1 > ncnt? n+1 : ncnt);
	}
      }
      /* if (debug) printf(" debug: reached end of input options file\n"); */
      fclose(fid);
    }
    
    if (*(argv[i]+1) == 'P') {  /* check for file if optional argument is file input */
      if (*(argv[i]+3) == '(') {  /* optional nodata value */
	sscanf(argv[i],"-P%d(%f)=%s",&n,&nodata,line);
	if (strstr(argv[i],"=") == NULL) {
	  fprintf(stderr,"*** WARNING: no '=' in P command *** %s\n",argv[i]);
	  sscanf(argv[i],"-P%d(%f)%s",&n,&nodata,line);
	}
      } else {  /* no optional nodata value */
	sscanf(argv[i],"-P%d=%s",&n,line);
	if (strstr(argv[i],"=") == NULL) {
	  fprintf(stderr,"*** WARNING: no '=' in P command *** %s\n",argv[i]);
	  sscanf(argv[i],"-P%d%s",&n,line);
	}
      }
      n--;
      if (n < 0) n=0;
      if (n > OMAX) n=OMAX-1;
      if ((command_lines[n]=(char *) malloc(sizeof(char)*(1+strlen(line)))) == NULL) {
	fprintf(stderr,"*** ERROR allocating input file name storage in get_input_files***\n");
	exit(-1);
      }
      out_anodata[n] = nodata;
      strcpy(command_lines[n],line);
      ncnt = (n+1 > ncnt? n+1 : ncnt);
    }
    i++;
  }
  if (debug) {
    printf(" debug: preprocess command lines: %d\n",ncnt);
    for (i=0;i<ncnt;i++)
      if (command_lines[i] != NULL) 
	printf("     %d: %s\n",i+1,command_lines[i]);
  }
  return(ncnt);
}


void pushop_rpn(rpn_entry rpn_list[], int op, int entry)
{
  /* printf("in pushop_rpn %d\n",op); */
  rpn_list[entry].command_code = op;
  rpn_list[entry].pointer = NULL;
  rpn_list[entry].nodata = 0.0;
  rpn_list[entry].in_nodata = 0.0;
}

void pushadd_rpn(rpn_entry rpn_list[], float *val, float out_nodata, float in_nodata, int entry)
{
  /* printf("in pushadd_rpn %d\n",entry); */
  rpn_list[entry].command_code = -1;
  rpn_list[entry].pointer = val;
  rpn_list[entry].nodata = out_nodata;
  rpn_list[entry].in_nodata = in_nodata;
  /* printf("done with pushadd_rpn %d %f\n", rpn_list[entry].pointer, rpn_list[entry].nodata); */
}

void pushval_rpn(rpn_entry rpn_list[], float value, int entry)
{
  /* printf("in pushval_rpn %f\n",value); */
  rpn_list[entry].command_code = -2;
  rpn_list[entry].pointer = NULL;
  rpn_list[entry].value = value;
  rpn_list[entry].in_nodata = 0.0;
}



int set_image_preprocess(char *command_lines[], int nchans, int rpn_depth[],
			 rpn_entry rpn_list[OMAX][DEPTH], 
			 int ninputs, float *in_stval[], float in_anodata[], 
			 float out_anodata[],int debug)
{
  int i, j, n, ncnt=nchans, m;
  char *c1, *c2, w[50];
  float val;

  /* printf("set_image_preprocess inputs: %d\n",ninputs);
  for (i=0; i < ninputs; i++)
    printf("  input %d of %d: %d %f\n",i,ninputs,in_stval[i],in_anodata[i]);
  */

  for (i=0; i < nchans; i++) {
    m=-1;
    rpn_depth[i] = m;
    
    if (command_lines[i] != NULL) {
      /* printf("preprocess command_line[%d]='%s'\n",i,command_lines[i]); */
      c1 = command_lines[i];
      n=strlen(c1);
      if (*(c1+n-1) != ',') {  /* insure command string is comma terminated */
	*(c1+n) = ',';
	*(c1+n+1) = '\0';
      }
      while ((c2 = strstr(c1,",")) != NULL) {  /* for each arg */
	n=c2-c1;
	n=(n>50?50:n);
	strncpy(w,c1,n);
	w[n]='\0';
	
	/* printf("preprocess '%s' command %d %d '%s'\n",c1,m+1,n,w);  */

	m++;
	if (m >= DEPTH) {
	  printf("*** preprocess stack depth of %d exceeded ***\n",DEPTH);
	  exit(-1);
	}

	/* decode command */

	switch (*w) {
	case '*':
	  pushop_rpn(rpn_list[i],3,m);
	  break;
	case '/':
	  pushop_rpn(rpn_list[i],4,m);
	  break;
	case '^':
	  pushop_rpn(rpn_list[i],5,m);
	  break;
	case 'd':  /* dB */
	case 'D':  /* dB */
	  pushop_rpn(rpn_list[i],8,m);
	  break;
	case 'a':  /* abs */
	case 'A':  /* abs */
	  pushop_rpn(rpn_list[i],9,m);
	  break;
	case ':':  /* push current value */
	  pushop_rpn(rpn_list[i],10,m);
	  break;
	case 'm':  /* mask */
	  pushop_rpn(rpn_list[i],11,m);
	  break;
	case '>':  /* greater thres */
	  pushop_rpn(rpn_list[i],12,m);
	  break;
	case '<':  /* greater thres */
	  pushop_rpn(rpn_list[i],13,m);
	  break;

	case 'I':
	  sscanf((w+1),"%d",&j);
	  if (j < 1 || j > ninputs) {
	    printf("*** preprocess specification error: invalid input file specification %d\n",j);
	  } else 
	    if (in_stval[j-1] == NULL)
	      printf("*** preprocess input error: no input %d defined\n",j);
	    else
	      pushadd_rpn(rpn_list[i],in_stval[j-1],out_anodata[i],in_anodata[j-1],m);
	  break;

	case '+':
	  if (*w == '+' && ( *(w+1) == ',' || *(w+1) == '\0')) {
	    pushop_rpn(rpn_list[i],1,m);  
	    break;
	  }
	case '-':
	  if (*w == '-' && ( *(w+1) == ',' || *(w+1) == '\0')) {
	    pushop_rpn(rpn_list[i],2,m);  
	    break;
	  }
	default:   /* value */
	  val=atof(w);
	  pushval_rpn(rpn_list[i],val,m);
	  break;
	}
	
	c1 = c2+1;
      }

      /* printf("setting rpn_depth %d %d\n",i,m); */      
      rpn_depth[i]=m;
    }      
  }
  return(ncnt);
}


#define MAX_STACK 50
float process_stack[MAX_STACK];   /* computational stack */
int stack_pointer;

float pop()
{
  float temp = process_stack[stack_pointer];
  stack_pointer--;
  if (stack_pointer < 0)
    stack_pointer = 0;
  return(temp);
}

void push(float value)
{
  stack_pointer++;
  if (stack_pointer > MAX_STACK)
    stack_pointer = MAX_STACK;
   process_stack[stack_pointer] = value;
}

void clear_stack(float value)
{
  stack_pointer = 0;
  process_stack[stack_pointer] = value;
  process_stack[stack_pointer+1] = value;
}



float *image_preprocess(rpn_entry rpn_list[], int rpn_depth, int n, 
			float pp_novalue, int debug)
{
  float *a, t, v1, v2;
  int i,m;
  char s[4];

  if (debug) {
    printf(" debug image_preprocessing: rpn_depth=%d  size=%d\n",rpn_depth,n);
    printf("                 node OP (#) Const    Out_nodata  In_nodata  <pointer>\n");

    for (m=0; m <= rpn_depth; m++) {

      switch(rpn_list[m].command_code) {
      case 1: /* + */
	strcpy(s,"+  ");
	break;

      case 2: /* - */
	strcpy(s,"-  ");
	break;
	
      case 3: /* * */
	strcpy(s,"*  ");
	break;
	
      case 4: /* / */
	strcpy(s,"/  ");
	break;
	
      case 5: /* ^ */
	strcpy(s,"^  ");
	break;
	
      case 8: /* dB */
	strcpy(s,"dB ");
	break;
	
      case 9: /* abs */
	strcpy(s,"abs");
	break;
	
      case 10: /* : (push) */
	strcpy(s,"pus");
	break;
	
      case 11: /* : (mask) */
	strcpy(s,"msk");
	break;
	
      case 12: /* : (greater thres) */
	strcpy(s,">  ");
	break;
	
      case 13: /* : (less thres) */
	strcpy(s,"<  ");
	break;
	
      case -1: /* array value */
	strcpy(s,"arr");
	break;
	
      case -2: /* constant */
	strcpy(s,"cst");
	break;

      default:
      case 0: /* no op */
	strcpy(s,"N/A");
	break;
      }
      printf(" debug rpn_list: %d %d %s (%d) %f %f %f %d\n",m,rpn_depth,s,rpn_list[m].command_code,
	     rpn_list[m].value, rpn_list[m].nodata, rpn_list[m].in_nodata, rpn_list[m].pointer);
    }
  }
    
  if (rpn_depth > 0) {
    
    a =(float *) malloc(sizeof(float) * n);
    if (a == NULL) {
      fprintf(stderr,"*** ERROR: preprocess image %d memory allocation failure...\n",n);
      exit(-1);
    }

    for (i=0; i < n; i++) {
      clear_stack(-1.e25);
      
      for (m=0; m <= rpn_depth; m++) {

	switch(rpn_list[m].command_code) {
	  
	case 1: /* + */
	  v1 = pop();
	  v2 = pop();
	  push(v1+v2);
	  break;

	case 2: /* - */
	  v1 = pop();
	  v2 = pop();
	  push(v2-v1);
	  break;

	case 3: /* * */
	  v1 = pop();
	  v2 = pop();
	  t = v1 * v2;
	  push(t);
	  break;

	case 4: /* / */
	  v1 = pop();
	  v2 = pop();
	  t = v2 / v1;
	  push(t);
	  break;

	case 5: /* ^ */
	  v1 = pop();
	  v2 = pop();
	  t = (float) pow( (double) v2, (double) v1);
	  push(t);
	  break;

	case 8: /* dB */
	  v1 = pop();
	  t = 10.0 * log10( (double) v1);
	  push(t);
	  break;

	case 9: /* abs */
	  v1 = pop();
	  t = (v1 < 0 ? -v1 : v1);
	  push(t);
	  break;

	case 10: /* : (push) */
	  t=pop();
	  push(t);
	  push(t);	  
	  break;

	case 11: /* : (mask) */
	  v1=pop();
	  v2=pop();
	  if (v1 > 0.0)
	    t=v2;
	  else
	    t=-1.e25;
	  push(t);
	  break;

	case 12: /* : (greater threshold) */
	  v1=pop();
	  v2=pop();
	  if (v2 > v1)
	    t=v1;
	  else
	    t=v2;
	  push(t);
	  break;

	case 13: /* : (less threshold) */
	  v1=pop();
	  v2=pop();
	  if (v2 < v1)
	    t=v1;
	  else
	    t=v2;
	  push(t);
	  break;

	case -1: /* array value */
	  t=*(rpn_list[m].pointer + i);
	  if (t <= rpn_list[m].in_nodata) {
	    push(rpn_list[m].nodata);
	    goto endit;
	  } else
	  push(t);
	  break;

	case -2: /* constant */
	  t=rpn_list[m].value;
	  push(t);
	  break;

	default:
	case 0: /* no op */
	  break;
	}
      }

    endit:  
      t = pop();
      a[i] = t;
    }
    
    return(a);
  } else 
    return(NULL);
}


  
void free_input_arrays(float *stval[], int nchans, float *in_stval[], int ninputs)
{
  int i,j,k;
  
  for (i=0; i < ninputs; i++) {
    k=0;
    for (j=0; j < nchans; j++)
      if (stval[j] == in_stval[i]) k=1;
    if (k == 0) {
      free(in_stval[i]);
      in_stval[i] = NULL;
    }
  }
}





