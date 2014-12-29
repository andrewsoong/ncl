/* THIS WRAPPER IS NOT DONE!! */

#include <stdio.h>
#include "wrapper.h"

extern void NGCALLF(weibfit,WEIBFIT)(int *, double *, double *, int *, 
                                     double *, double *, int *);

NhlErrorTypes trend_manken_n_W( void )
{

/*
 * Input variables
 */
/*
 * Argument # 0
 */
  void *x;
  double *tmp_x;
  int       ndims_x;
  ng_size_t nx, dsizes_x[NCL_MAX_DIMENSIONS];
  int inx, has_missing_x;
  NclScalar missing_x, missing_flt_x, missing_dbl_x;
  NclBasicDataTypes type_x;

/*
 * Argument # 1
 */
  int *dims;
  ng_size_t ndims;
/*
 * Argument # 2
 */
  logical *opt;
/*
 * Return variable
 */
  void *wb;
  double tmp_wb[6];
  int       ndims_wb;
  ng_size_t *dsizes_wb;
  int has_missing_wb;
  NclScalar missing_wb, missing_flt_wb, missing_dbl_wb;
  NclBasicDataTypes type_wb;

/*
 * Variables for retrieving attributes from "opt".
 */
  NclAttList  *attr_list;
  NclAtt  attr_obj;
  NclStackEntry stack_entry;
  int nmin, set_nmin, set_confi;
  void *confi;
  double *tmp_confi;
  NclBasicDataTypes type_confi;

/*
 * Various
 */
  ng_size_t i, j, nrnx, total_nl, total_nr, total_elements;
  ng_size_t index_nrx, index_x, index_wb, index_nr;
  int ier, ret;

/*
 * Retrieve parameters.
 *
 * Note any of the pointer parameters can be set to NULL, which
 * implies you don't care about its value.
 */
/*
 * Get argument # 0
 */
  x = (void*)NclGetArgValue(
           0,
           3,
           &ndims_x,
           dsizes_x,
           &missing_x,
           &has_missing_x,
           &type_x,
           DONT_CARE);

/*
 * Coerce missing value to double if necessary.
 */
  coerce_missing(type_x,has_missing_x,&missing_x,
                 &missing_dbl_x,&missing_flt_x);

/*
 * Get argument # 1
 */
  opt = (logical*)NclGetArgValue(
           1,
           3,
           NULL,
           NULL,
           NULL,
           NULL,
           NULL,
           DONT_CARE);

/*
 * Get argument # 2
 */
  dims = (int *)NclGetArgValue(2,3,NULL,&ndims,NULL,NULL,NULL,0);
/*
 * Some error checking. Make sure input dimensions are valid.
 */
  for(i = 0; i < ndims; i++ ) {
    if(dims[i] < 0 || dims[i] >= ndims_x) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"trend_manken_n: Invalid dimension sizes to do calculation on, can't continue");
      return(NhlFATAL);
    }
    if(i > 0 && dims[i] != (dims[i-1]+1)) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"trend_manken_n: Input dimension sizes must be monotonically increasing, can't continue");
      return(NhlFATAL);
    }
  }

/*
 * Check for attributes attached to "opt"
 */
  set_nmin = set_confi = False;

  if(*opt) {
    stack_entry = _NclGetArg(2, 3, DONT_CARE);
    switch (stack_entry.kind) {
    case NclStk_VAR:
      if (stack_entry.u.data_var->var.att_id != -1) {
        attr_obj = (NclAtt) _NclGetObj(stack_entry.u.data_var->var.att_id);
        if (attr_obj == NULL) {
          break;
        }
      }
      else {
/*
 * att_id == -1 ==> no optional args given.
 */
        break;
      }
/* 
 * Get optional arguments.
 */
      if (attr_obj->att.n_atts > 0) {
/*
 * Get list of attributes.
 */
        attr_list = attr_obj->att.att_list;
/*
 * Loop through attributes and check them. We are looking for:
 *
 *  nmin or confi
 */
        while (attr_list != NULL) {
          if(!strcasecmp(attr_list->attname, "nmin")) {
            nmin      = *(int *) attr_list->attvalue->multidval.val;
            set_nmin  = True;
          }
          else if(!strcasecmp(attr_list->attname, "confi")) {
            confi      = attr_list->attvalue->multidval.val;
            type_confi = attr_list->attvalue->multidval.data_type;
            set_confi  = True;
          }
          attr_list = attr_list->next;
        }
      default:
        break;
      }
    }
  }
  if(!set_nmin) {
    nmin = inx;
  }
  if(set_confi) {
    tmp_confi = coerce_input_double(confi,type_confi,1,0,NULL,NULL);
  }
  else {
    type_confi = NCL_double;
    tmp_confi  = (double *)calloc(1,sizeof(double));
    *tmp_confi = 1.0;
  }

/* 
 * Allocate space for output dimension sizes and set them.
 */
  ndims_wb = ndims_x - ndims + 1;
  dsizes_wb = (ng_size_t*)calloc(ndims_wb,sizeof(ng_size_t));  
  if( dsizes_wb == NULL ) {
    NhlPError(NhlFATAL,NhlEUNKNOWN,"trend_manken_n: Unable to allocate memory for holding dimension sizes");
    return(NhlFATAL);
  }

  nx = total_nl = total_nr = total_elements = 1;
  for(i = 0; i < dims[0]; i++) {
    total_nl *= dsizes_x[i];
    dsizes_wb[i] = dsizes_x[i];
  }
  for(i = 0; i < ndims ; i++) {
    nx = nx*dsizes_x[dims[i]];
  }
  for(i = dims[ndims-1]+1; i < ndims_x; i++) {
    total_nr *= dsizes_x[i];
    dsizes_wb[i-ndims] = dsizes_x[i];
  }
  total_elements = total_nr * total_nl;
  if(set_confi) {
    dsizes_wb[ndims_wb-1] = 6;
  }
  else {
    dsizes_wb[ndims_wb-1] = 2;
  }
  if(nx > INT_MAX) {
    NhlPError(NhlFATAL,NhlEUNKNOWN,"trend_manken_n: nx = %ld is greater than INT_MAX", nx);
    return(NhlFATAL);
  }
  inx = (int) nx;

/*
 * Allocate space for tmp_x.
 */
  tmp_x = (double *)calloc(nx,sizeof(double));
  if(tmp_x == NULL) {
    NhlPError(NhlFATAL,NhlEUNKNOWN,"trend_manken_n: Unable to allocate memory for coercing input array to double");
    return(NhlFATAL);
  }

/* 
 * Allocate space for output array.
 */
  if(type_x != NCL_double) type_wb = NCL_float;
  else                     type_wb = NCL_double;
  if(type_wb != NCL_double) {
    if(set_confi) {
      wb = (void *)calloc(6*total_elements, sizeof(float));
    }
    else {
      wb = (void *)calloc(2*total_elements, sizeof(float));
    }
  }
  else {
    if(set_confi) {
      wb = (void *)calloc(6*total_elements, sizeof(double));
    }
    else {
      wb = (void *)calloc(2*total_elements, sizeof(double));
    }
  }
  if(wb == NULL) {
    NhlPError(NhlFATAL,NhlEUNKNOWN,"trend_manken_n: Unable to allocate memory for temporary output array");
    return(NhlFATAL);
  }

/*
 * Loop across leftmost dimensions and call the Fortran routine for each
 * subsection of the input arrays.
 */
  nrnx = total_nr * nx;
  for(i = 0; i < total_nl; i++) {
    index_nrx = i*nrnx;
    index_nr  = i*total_nr;
    for(j = 0; j < total_nr; j++) {
      index_x  = index_nrx + j;
      index_wb = index_nr + j;
/*
 * Coerce subsection of x (tmp_x) to double.
 */
      coerce_subset_input_double_step(x,tmp_x,index_x,total_nr,type_x,
                                      nx,0,NULL,NULL);

/*
 * Call the Fortran routine.
 */
      NGCALLF(weibfit,WEIBFIT)(&inx, tmp_x, &missing_dbl_x.doubleval, 
			       &nmin, tmp_confi, tmp_wb, &ier);

/*
 * Coerce output array to appropriate type
 */
      if(set_confi) {
	coerce_output_float_or_double(wb,tmp_wb,type_wb,6,index_wb);
      }
      else {
	coerce_output_float_or_double(wb,tmp_wb,type_wb,2,index_wb);
      }
    }
  }

/*
 * Free unneeded memory.
 */
  NclFree(tmp_x);
  if(type_confi != NCL_double) NclFree(tmp_confi);

/*
 * Return value back to NCL script.
 */
  if(type_wb != NCL_double) {
    ret = NclReturnValue(wb,ndims_wb,dsizes_wb,&missing_flt_wb,type_wb,0);
  }
  else {
    ret = NclReturnValue(wb,ndims_wb,dsizes_wb,&missing_dbl_wb,type_wb,0);
  }
  NclFree(dsizes_wb);
  return(ret);
}