/****************************************************************************
 * NCSA HDF                                                                 *
 * Software Development Group                                               *
 * National Center for Supercomputing Applications                          *
 * University of Illinois at Urbana-Champaign                               *
 * 605 E. Springfield, Champaign IL 61820                                   *
 *                                                                          *
 * For conditions of distribution and use, see the accompanying             *
 * hdf/COPYING file.                                                        *
 *                                                                          *
 ****************************************************************************/

#ifdef RCSID
static char RcsId[] = "@(#)$Revision$";
#endif

/* $Id$ */

#include "mfhdf.h"
#include "hdp.h"
#include <ctype.h>

#define NUM_FIELD_WIDTH 5
#define TAGNAME_FIELD_WIDTH 20
#define TAG_FIELD_WIDTH 8
#define REF_FIELD_WIDTH 8
#define INDEX_FIELD_WIDTH 12
#define OFFSET_FIELD_WIDTH 12
#define LENGTH_FIELD_WIDTH 12
#define LABEL_FIELD_WIDTH 15
#define DESC_FIELD_WIDTH 15

static void 
list_usage(intn argc, 
           char *argv[])
{
    printf("Usage:\n");
    printf("%s list [-acensldg] [-o<f|g|t|n>] [-t tag] <filelist>\n", argv[0]);
    printf("\t-a\tPrint annotations of items (sets long output)\n");
    printf("\t-c\tPrint classes of items (sets long output)\n");
    printf("\t-n\tPrint names or labels of items (sets long output)\n");
    printf("\t-e\tPrint special element information of items (sets long output)\n");
    printf("\t-s\tShort output (default)\n");
    printf("\t-l\tLong output\n");
    printf("\t-d\tDebugging output\n");
    printf("\t-g\tPrint groups only\n");
    printf("\t-t <number>\tPrint items of with a given tag number\n");
    printf("\t-t <name>\tPrint items of with a given tag name\n");
    printf("\t-of\tPrint items in the order found in the file\n");
    printf("\t-og\tPrint items in group order\n");
    printf("\t-ot\tPrint items in tag order (default)\n");
#if 0 /* No longer possible since objects can have more than one label 
       * -GV 6/12/97 */
    printf("\t-on\tPrint items in name or label order\n");
#endif
    printf("\t<filelist>\tList of hdf file names, separated by spaces\n");
}	/* end list_usage() */

static void 
init_list_opts(list_info_t * list_opts)
{
    list_opts->order = OTAG;	/* default ordering is by tag */
    list_opts->verbosity = VSHORT;	/* default verbosity is a short list */
    list_opts->limit = LNONE;	/* default is all the tag/refs */
    list_opts->class = FALSE;	/* don't dump class information */
    list_opts->name = FALSE;	/* don't dump name information */
    list_opts->desc = FALSE;	/* don't dump annotation information */
    list_opts->spec = FALSE;	/* don't dump special element information */
    list_opts->group = FALSE;	/* don't dump group information */
    list_opts->limit_tag = 0;	/* initialize... */
}	/* end init_list_opts() */

static intn 
parse_list_opts(list_info_t * list_opts, 
                intn curr_arg, 
                intn argc, 
                char *argv[])
{
    intn        ret = SUCCEED;

    for (; curr_arg < argc; curr_arg++)
      {
#if defined(WIN386) || defined(DOS386)
          if (argv[curr_arg][0] == '-' || argv[curr_arg][0] == '/')
#else
          if (argv[curr_arg][0] == '-' )
#endif /* for DOS/WINDOWS */
            {
                ret++;
                switch (argv[curr_arg][1])
                  {
                  case 'a':	/* print annotations */
                      list_opts->desc = TRUE;	/* dump description information */
                      if (list_opts->verbosity == VSHORT)
                          list_opts->verbosity = VLONG;		/* verbosity is a long list */
                      break;

                  case 'c':	/* print classes */
                      list_opts->class = TRUE;	/* dump class information */
                      if (list_opts->verbosity == VSHORT)
                          list_opts->verbosity = VLONG;		/* verbosity is a long list */
                      break;

                  case 'n':	/* print names */
                      list_opts->name = TRUE;	/* dump name/label information */
                      if (list_opts->verbosity == VSHORT)
                          list_opts->verbosity = VLONG;		/* verbosity is a long list */
                      break;

                  case 'e':	/* print special element info */
                      list_opts->spec = TRUE;	/* dump special element information */
                      if (list_opts->verbosity == VSHORT)
                          list_opts->verbosity = VLONG;		/* verbosity is a long list */
                      break;

                  case 's':	/* short output */
                      list_opts->verbosity = VSHORT;	/* verbosity is short */
                      break;

                  case 'l':	/* long output */
                      list_opts->verbosity = VLONG;		/* verbosity is long */
                      break;

                  case 'd':	/* debugging output */
                      list_opts->verbosity = VDEBUG;	/* verbosity is debug */
                      break;

                  case 'g':	/* print only groups */
#ifdef LATER
                      list_opts->limit = LGROUP;	/* limit to group output */
#endif
                      list_opts->group = TRUE;	/* dump group info */
                      if (list_opts->verbosity == VSHORT)
                          list_opts->verbosity = VLONG;		/* verbosity is long */
                      break;

                  case 't':	/* print only items of one tag */
                      curr_arg++;
                      ret++;
                      if (isdigit(argv[curr_arg][0]))
                        {
                            list_opts->limit = LTAGNUM;		/* limit to tag name output */
                            list_opts->limit_tag = atoi(argv[curr_arg]);
                            list_opts->limit_name = tagnum_to_name(list_opts->limit_tag);
                        }	/* end if */
                      else
                        {	/* must be a tag name */
                            list_opts->limit = LTAGNAME;	/* limit to tag name output */
                            list_opts->limit_name = HDstrdup(argv[curr_arg]);
                            list_opts->limit_tag = tagname_to_num(list_opts->limit_name);
                            if (list_opts->limit_tag == DFTAG_NULL)
                              {
                                  printf("ERROR: invalid tag name: %s\n", list_opts->limit_name);
                                  return (FAIL);
                              }		/* end if */
                        }	/* end else */
                      break;

                  case 'o':	/* order the items in some way */
                      switch (argv[curr_arg][2])
                        {
                        case 'g':
                            list_opts->order = OGROUP;	/* ordering is by group */
                            break;

                        case 't':
                            list_opts->order = OTAG;	/* ordering is by tag */
                            break;

                        case 'f':
                            list_opts->order = OFILE;	/* ordering is by file */
                            break;

#if 0 /* No longer possible since objects can have more than one label 
       * -GV 6/12/97 */
                        case 'n':
                            list_opts->order = ONAME;	/* ordering is by name */
                            break;
#endif
                        default:
                            printf("ERROR: Invalid list ordering!\n");
                            return (FAIL);
                        }	/* end switch */
                      break;

                  default:		/* invalid list option */
                      printf("ERROR: Invalid list option!\n");
                      return (FAIL);
                  }		/* end switch */
            }	/* end if */
      }		/* end for */

    return (ret);
}	/* end parse_list_opts */

static void 
print_list_header(list_info_t * list_opts)
{
    switch (list_opts->verbosity)
      {
      case VSHORT:		/* short output */
              /* no header */
          break;

      case VLONG:	/* long output */
          printf("%*s%*s%*s%*s%*s\n",
                 NUM_FIELD_WIDTH, "no",
                 TAGNAME_FIELD_WIDTH, "tagname",
                 TAG_FIELD_WIDTH, "tag", REF_FIELD_WIDTH, "ref",
                 INDEX_FIELD_WIDTH, "  index_by_tag");
          break;

      case VDEBUG:		/* debugging output */
          printf("%*s%*s%*s%*s%*s%*s%*s\n",
                 NUM_FIELD_WIDTH, "no",
                 TAGNAME_FIELD_WIDTH, "tagname",
                 TAG_FIELD_WIDTH, "tag", REF_FIELD_WIDTH, "ref",
                 INDEX_FIELD_WIDTH, "  index_by_tag",
                 OFFSET_FIELD_WIDTH, "offset",
                 LENGTH_FIELD_WIDTH, "length");
          break;
      }		/* end switch() */
}	/* end print_list_header() */


/* Exported
 * print all data labels for object with tag/ref */
intn
print_data_labels(const char *fname,
                  int32 an_id, 
                  uint16 tag, 
                  uint16 ref)
{
    intn  i;
    char  *buf = NULL;
    int32  ann_num;
    int32  ann_length;
    int32  ann_id = FAIL;
    int32 *ann_list = NULL;
    intn   ret_value = SUCCEED;

    /* find number of labels for object with tag/ref */
    ann_num = ANnumann(an_id, AN_DATA_LABEL, tag, ref);
    if (FAIL == ann_num)
      {
          fprintf(stderr,"Failure in ANnumann for object tag=%d, ref=%d, in file %s\n",
                  tag,ref,fname);
          ret_value = FAIL;
          goto done;
      }

    if (ann_num > 0 )
      { /* print data labels */

          /* allocate space for all label id's for data object */
          ann_list = HDmalloc(ann_num*sizeof(int32));
          if (NULL == ann_list)
            {
                fprintf(stderr,"Failure to allocate space \n");
                ret_value = FAIL;
                goto done;
            }

          /* retrieve all the data objects label handles */
          if (FAIL == ANannlist(an_id,AN_DATA_LABEL,tag, 
                                ref, ann_list))
            {
                fprintf(stderr,"Failure in ANannlist for object tag=%d, ref=%d, in file %s\n",
                        tag,ref,fname);
                ret_value = FAIL;
                goto done;
            }

          /* for every data label */
          for(i = 0; i < ann_num; i++)
            {
                ann_id = ann_list[i];
                ann_length =  ANannlen(ann_id);
                if (FAIL == ann_length)
                  {
                      fprintf(stderr,"Failure in ANannlen for %d'th label for object tag=%d, ref=%d, in file %s\n",
                              i,tag,ref,fname);
                      ret_value = FAIL;
                      goto done;
                  }
                buf = HDcalloc((ann_length+1) * sizeof(char),1);
                if (buf != NULL)
                    buf[ann_length] = '\0';
                else
                  {
                      fprintf(stderr,"Failure to allocate space \n");
                      ret_value = FAIL;
                      goto done;
                  }

                if (FAIL == ANreadann(ann_id, buf, ann_length+1))
                  {
                      fprintf(stderr,"Failure in ANreadann for %d'th label for object tag=%d, ref=%d, in file %s\n",
                              i,tag,ref,fname);
                      ret_value = FAIL;
                      goto done;
                  }

                printf("%*s%s\n", LABEL_FIELD_WIDTH, " Name/Label=", buf);

                /* end access */
                if (FAIL == ANendaccess(ann_id))
                  {
                      fprintf(stderr,"ANendaccess failed for %d'th label for object tag=%d, ref=%d, in file %s\n",
                              i,tag,ref,fname);
                      ret_value = FAIL;
                      goto done;
                  }

                /* reset id and free space for data label */
                ann_id = FAIL;
                HDfree(buf);
                buf = NULL;
            } /* end for ever data label */
        
          /* cleanup */
          HDfree(ann_list);
          ann_list = NULL;
      }  /* end if num_ann */

  done:
    if (ret_value == FAIL)
      { /* Failure cleanup */
          if (ann_id != FAIL)
              ANendaccess(ann_id);        
          if (buf != NULL)
              HDfree(buf);
      }
    /* Normal cleanup */

    return ret_value;
} /* print_data_labels() */


/* Exported
 * print all data descriptions for object with tag/ref */
intn
print_data_descs(const char *fname,
                 int32 an_id, 
                 uint16 tag, 
                 uint16 ref)
{
    intn  i;
    char  *buf = NULL;
    int32  ann_num;
    int32  ann_length;
    int32  ann_id = FAIL;
    int32 *ann_list = NULL;
    intn   ret_value = SUCCEED;

    /* find number of data descriptions for object with tag/ref */
    ann_num = ANnumann(an_id, AN_DATA_DESC, tag, ref);
    if (FAIL == ann_num)
      {
          fprintf(stderr,"Failure in ANnumann for object tag=%d, ref=%d, in file %s\n",
                  tag,ref,fname);
          ret_value = FAIL;
          goto done;
      }

    if (ann_num > 0 )
      { /* print data descriptions */

          ann_list = HDmalloc(ann_num*sizeof(int32));
          if (NULL == ann_list)
            {
                fprintf(stderr,"Failure to allocate space \n");
                ret_value = FAIL;
                goto done;
            }

          /* retrieve all the list of data descs id's */
          if (FAIL == ANannlist(an_id,AN_DATA_DESC,tag, 
                                ref, ann_list))
            {
                fprintf(stderr,"Failure in ANannlist for object tag=%d, ref=%d, in file %s\n",
                        tag,ref,fname);
                ret_value = FAIL;
                goto done;
            }

          /* for all data descs */
          for(i = 0; i < ann_num; i++)
            {
                ann_id = ann_list[i];
                ann_length =  ANannlen(ann_id);
                if (FAIL == ann_length)
                  {
                      fprintf(stderr,"Failure in ANannlen for %d'th description for object tag=%d, ref=%d, in file %s\n",
                              i,tag,ref,fname);
                      ret_value = FAIL;
                      goto done;
                  }
                buf = HDcalloc((ann_length+1) * sizeof(char),1);
                if (buf != NULL)
                    buf[ann_length] = '\0';
                else
                  {
                      fprintf(stderr,"Failure to allocate space \n");
                      ret_value = FAIL;
                      goto done;
                  }

                if (FAIL == ANreadann(ann_id, buf, ann_length+1))
                  {
                      fprintf(stderr,"Failure in ANreadann for %d'th description for object tag=%d, ref=%d, in file %s\n",
                              i,tag,ref,fname);
                      ret_value = FAIL;
                      goto done;
                  }

                printf("%*s%s\n", LABEL_FIELD_WIDTH, "  Description=", buf);

                /* end access */
                if (FAIL == ANendaccess(ann_id))
                  {
                      fprintf(stderr,"ANendaccess failed for file for %d'th description for object tag=%d, ref=%d, in file %s\n",
                              i,tag,ref,fname);
                      ret_value = FAIL;
                      goto done;
                  }
                
                /* free buffer and reset id */
                ann_id = FAIL;
                HDfree(buf);
                buf = NULL;
            } /* end for ever data desc */
        
          /* cleanup */
          HDfree(ann_list);
          ann_list = NULL;
      } /* end if num_ann */

  done:
    if (ret_value == FAIL)
      { /* Failure cleanup */
          if (ann_id != FAIL)
              ANendaccess(ann_id);        
          if (buf != NULL)
              HDfree(buf);
      }
    /* Normal cleanup */

    return ret_value;
} /* print_data_descs() */

/* Exported
 * prints all data labels in file */
intn
print_all_data_labels(const char *fname, 
                      int32 an_id)
{
    intn i;
    int32 len;
    char *label = NULL;
    int32 ann_id = FAIL;
    int32 n_file_label;
    int32 n_file_desc;
    int32 n_data_label;
    int32 n_data_desc;
    intn  ret_value = SUCCEED;


    /* find out how many file labels/descs and data labels/descs in file */
    if (FAIL == ANfileinfo(an_id, &n_file_label, &n_file_desc, &n_data_label, 
                           &n_data_desc))
      {
          fprintf(stderr,"ANfileinfo failed for file %s\n", fname);
          ret_value = FAIL;
          goto done;
      }

    /* for all data labels */
    for(i = 0; i< n_data_label; i++) 
      {  
          /* select i'th data label */
          ann_id = ANselect(an_id, i, AN_DATA_LABEL);
          if (FAIL == ann_id)
            {
                fprintf(stderr, "ANselect failed for %d'th label for file %s\n", i, fname);
                ret_value = FAIL;
                goto done;
            }

          /* get length of i'th data label */
          len = ANannlen(ann_id);
          if (FAIL == len)
            {
                fprintf(stderr, "ANannlen failed for %d'th label for file %s\n", i, fname);
                ret_value = FAIL;
                goto done;
            }

          /* allocate room for a data label */
          if ((label = (char *) HDcalloc(len + 1,1)) == NULL)
            {
                fprintf(stderr,"Failed to allocate space for %d'th data annotation label for file %s\n",i, fname);
                ret_value = FAIL;
                goto done;
            }

          /* read in data label and print it */
          if(ANreadann(ann_id, label, len+1)!= FAIL)
              printf("Data ID Label #%ld: %s\n", (long)i, label);
          else
            {
                fprintf(stderr,"ANreadann failed for %d'th label for file %s\n",i, fname);
                ret_value = FAIL;
                goto done;
            }

          /* end access */
          if (FAIL == ANendaccess(ann_id))
            {
                fprintf(stderr,"ANendaccess failed for %d'th label for file %s\n",i, fname);
                ret_value = FAIL;
                goto done;
            }

          /* reset id and free space for data label */
          ann_id = FAIL;
          HDfree(label);
          label = NULL;
      } /* end for every data label */ 

  done:
    if (ret_value == FAIL)
      { /* Failure cleanup */
          if (ann_id != FAIL)
              ANendaccess(ann_id);        
          if (label != NULL)
              HDfree(label);
      }
    /* Normal cleanup */

    return ret_value;
} /* print_all_data_labels() */

/* Exported
 * prints all data descriptions in file */
intn
print_all_data_descs(const char *fname, 
                     int32 an_id)
{
    int32 len;
    char *desc = NULL;
    int32 ann_id = FAIL;
    intn i;
    int32 n_file_label;
    int32 n_file_desc;
    int32 n_data_label;
    int32 n_data_desc;
    intn  ret_value = SUCCEED;

    /* find out how many file labels/descs and data labels/descs in file */
    if (FAIL == ANfileinfo(an_id, &n_file_label, &n_file_desc, &n_data_label, 
                           &n_data_desc))
      {
          fprintf(stderr,"ANfileinfo failed for file %s\n", fname);
          ret_value = FAIL;
          goto done;
      }

    /* for all data descs */
    for(i = 0; i< n_data_desc; i++) 
      {  
          /* select i'th data desc */
          ann_id = ANselect(an_id, i, AN_DATA_DESC);
          if (FAIL == ann_id)
            {
                fprintf(stderr,"ANselect failed for %d'th data description in file %s\n", i, fname);
                ret_value = FAIL;
                goto done;
            }

          /* get length of i'th data desc */
          len = ANannlen(ann_id);
          if (FAIL == len)
            {
                fprintf(stderr,"ANannlen failed for %d'th data description in file %s\n", i, fname);
                ret_value = FAIL;
                goto done;
            }

          /* allocate room for a data desc */
          if ((desc = (char *) HDcalloc(len + 1,1)) == NULL)
            {
                fprintf(stderr,"Failed to allocate space for %d'th data annotation desc in file %s\n",i, fname);
                ret_value = FAIL;
                goto done;
            }

          /* read in data desc and print it */
          if(ANreadann(ann_id, desc, len+1)!= FAIL)
              printf("Data ID Annotation #%ld: %s\n", (long)i, desc);
          else
            {
                fprintf(stderr,"ANreadann failed for %d'th data description in file %s\n",i, fname);
                ret_value = FAIL;
                goto done;
            }

          /* end access */
          if (FAIL == ANendaccess(ann_id))
            {
                fprintf(stderr,"ANendaccess failed for %d'th data description in file %s\n",i, fname);
                ret_value = FAIL;
                goto done;
            }

          /* reset id and free space for desc */
          ann_id = FAIL;
          HDfree(desc);
          desc = NULL;
      } /* end for every data desc */ 

  done:
    if (ret_value == FAIL)
      { /* Failure cleanup */
      }
    /* Normal cleanup */
    if (ann_id != FAIL)
        ANendaccess(ann_id);        
    if (desc != NULL)
        HDfree(desc);

    return ret_value;
} /* print_all_data_descs() */

intn
print_all_file_labels(const char *fname, 
                      int32 an_id)
{
    int32 len;
    char *label = NULL;
    intn i;
    int32 ann_id = FAIL;
    int32 n_file_label;
    int32 n_file_desc;
    int32 n_data_label;
    int32 n_data_desc;
    intn  ret_value = SUCCEED;

    /* find out how many file labels/descs and data labels/descs in file */
    if (FAIL == ANfileinfo(an_id, &n_file_label, &n_file_desc, &n_data_label, 
                      &n_data_desc))
      {
          fprintf(stderr,"ANfileinfo failed for file %s\n", fname);
          ret_value = FAIL;
          goto done;
      }

    /* for all file labels */
    for(i = 0; i< n_file_label; i++) 
      {  
          /* select i'th file label */
          ann_id = ANselect(an_id, i, AN_FILE_LABEL);
          if (FAIL == ann_id)
            {
                fprintf(stderr,"ANselect failed for %d'th label for file %s\n", i, fname);
                ret_value = FAIL;
                goto done;
            }

          /* get length of i'th file label */
          len = ANannlen(ann_id);
          if (FAIL == len)
            {
                fprintf(stderr,"ANannlen failed for %d'th label for file %s\n",i, fname);
                ret_value = FAIL;
                goto done;
            }

          /* allocate room for the file label */
          if ((label = (char *) HDcalloc(len + 1,1)) == NULL)
            {
                fprintf(stderr,"Failed to allocate space for %d'th file annotation label in file %s\n",i, fname);
                ret_value = FAIL;
                goto done;
            }

          /* read in file label and print it */
          if(ANreadann(ann_id, label, len+1)!= FAIL)
              printf("File Label #%ld: %s\n", (long)i, label);
          else
            {
                fprintf(stderr,"ANreadann failed for %d'th label for file %s\n", i, fname);
                ret_value = FAIL;
                goto done;
            }

          /* end access */
          if (FAIL == ANendaccess(ann_id))
            {
                fprintf(stderr,"ANendaccess failed for %d'th label for file %s\n",i, fname);
                ret_value = FAIL;
                goto done;
            }

          /* reset id and free space for label */
          ann_id = FAIL;
          HDfree(label);
          label = NULL;
      } /* end for every file label */ 

  done:
    if (ret_value == FAIL)
      { /* Failure cleanup */
      }
    /* Normal cleanup */
    if (ann_id != FAIL)
        ANendaccess(ann_id);        
    if (label != NULL)
        HDfree(label);

    return ret_value;
}	/* end print_all_file_labels() */

intn 
print_all_file_descs(const char *fname, 
                     int32 an_id)
{
    /* file desc */
    int32       len; 
    char       *desc = NULL;
    int32       ann_id = FAIL;
    intn       i;
    int32       n_file_label;
    int32       n_file_desc;
    int32       n_data_label;
    int32       n_data_desc;
    file_type_t ft = DASCII;
    /* SDS */
    int32       sd_fid = FAIL;
    int32       ndsets, nattrs;
    char        name[MAXNAMELEN];
    int32       attr_nt;
    int32       attr_count;
    int32       attr_index;
    char       *attr_nt_desc = NULL;
    int32       attr_buf_size;
    VOIDP       attr_buf = NULL;
    intn        ret_value = SUCCEED;

    /* find out how many file labels/descs and data labels/descs in file */
    if (FAIL == ANfileinfo(an_id, &n_file_label, &n_file_desc, &n_data_label, 
                           &n_data_desc))
      {
          fprintf(stderr,"ANfileinfo failed for file %s \n",fname);
          ret_value = FAIL;
          goto done;
      }

    /* for all file descs */
    for(i = 0; i< n_file_desc; i++) 
      {  
          /* select i'th file desc */
          ann_id = ANselect(an_id, i, AN_FILE_DESC);
          if (FAIL == ann_id)
            {
                fprintf(stderr,"ANselect failed for %d'th desc for file %s \n",i, fname);
                ret_value = FAIL;
                goto done;
            }

          /* get length of i'th file desc */
          len = ANannlen(ann_id);
          if (FAIL == len)
            {
                fprintf(stderr,"ANannlen failed for %d'th desc for file %s \n",i,fname);
                ret_value = FAIL;
                goto done;
            }

          /* allocate room for the file desc */
          if ((desc = (char *) HDcalloc(len + 1,1)) == NULL)
            {
                fprintf(stderr,"Failed to allocate space for %d file annotation desc for file %s\n",i, fname);
                ret_value = FAIL;
                goto done;
            }

          /* read in file desc and print it */
          if(ANreadann(ann_id, desc, len+1)!= FAIL)
              printf("File description #%ld: %s\n", (long)i, desc);
          else
            {
                fprintf(stderr,"ANreadann failed for %d'th desc for file %s \n",i,fname);
                ret_value = FAIL;
                goto done;
            }

          /* end access */
          if (FAIL == ANendaccess(ann_id))
            {
                fprintf(stderr,"ANendaccess failed for %d'th desc for file %s \n",i,fname);
                ret_value = FAIL;
                goto done;
            }

          /* reset id and free space for label */
          ann_id = FAIL;
          HDfree(desc);
          desc = NULL;
      } /* end for every file desc */ 

    /* all SDS global attributes are considered file descriptions */
    if ((sd_fid = SDstart(fname, DFACC_READ)) != FAIL)
      { /* SD global attributes */
          if (SDfileinfo(sd_fid, &ndsets, &nattrs) != FAIL)
               print_SDattrs( sd_fid, stdout, nattrs, fname );
               /* temporary use stdout until fixing hdp_list to print
                  to a FILE *fp */
          else
            {
                fprintf(stderr,"Failure in SDfileinfo for file %s\n",
fname);
                ret_value = FAIL;
                goto done;
            }
          if (FAIL == SDend(sd_fid))
             fprintf(stderr, "SDend failed for the current file\n" );
          sd_fid = FAIL; /* reset */
      }         /* end if  SDstart */

  done:
    if (ret_value == FAIL)
      { /* Failure cleanup */
          if (ann_id != FAIL)
              ANendaccess(ann_id);        
          if (desc != NULL)
              HDfree(desc);
          if (attr_nt_desc != NULL)
              HDfree(attr_nt_desc);
          if (attr_buf != NULL)
              HDfree((VOIDP) attr_buf);
      }
    /* Normal cleanup */

    return ret_value;
}	/* end print_all_file_descs() */

/* BMR: use part of print_all_file_descs for this routine to just print
   the file annotations because print_all_file_descs also prints SD
   file attributes.  Probably will separate SD file attributes when
   adding GR file attributes */
intn 
print_file_descs(const char *f_name,
                 int32 an_id ) 
{
    /* file desc */
    int32       len; 
    char       *desc = NULL;
    int32       ann_id = FAIL;
    intn       i;
    int32       n_file_label;
    int32       n_file_desc;
    int32       n_data_label;
    int32       n_data_desc;
    file_type_t ft = DASCII;
    /* SDS */
    int32       sd_fid = FAIL;
    int32       ndsets, nattrs;
    char        name[MAXNAMELEN];
    int32       attr_nt;
    int32       attr_count;
    int32       attr_index;
    int32       attr_buf_size;
    intn        ret_value = SUCCEED;

    /* find out how many file labels/descs and data labels/descs in file */
    if (FAIL == ANfileinfo(an_id, &n_file_label, &n_file_desc, &n_data_label, 
                           &n_data_desc))
      {
          fprintf(stderr,"ANfileinfo failed for file %s \n",f_name);
          ret_value = FAIL;
          goto done;
      }

    /* for all file descs */
    for(i = 0; i< n_file_desc; i++) 
      {  
          /* select i'th file desc */
          ann_id = ANselect(an_id, i, AN_FILE_DESC);
          if (FAIL == ann_id)
            {
                fprintf(stderr,"ANselect failed for %d'th desc for file %s \n",i, f_name);
                ret_value = FAIL;
                goto done;
            }

          /* get length of i'th file desc */
          len = ANannlen(ann_id);
          if (FAIL == len)
            {
                fprintf(stderr,"ANannlen failed for %d'th desc for file %s \n",i,f_name);
                ret_value = FAIL;
                goto done;
            }

          /* allocate room for the file desc */
          if ((desc = (char *) HDcalloc(len + 1,1)) == NULL)
            {
                fprintf(stderr,"Failed to allocate space for %d file annotation desc for file %s\n",i, f_name);
                ret_value = FAIL;
                goto done;
            }

          /* read in file desc and print it */
          if(ANreadann(ann_id, desc, len+1)!= FAIL)
              printf("File description #%ld: %s\n", (long)i, desc);
          else
            {
                fprintf(stderr,"ANreadann failed for %d'th desc for file %s \n",i,f_name);
                ret_value = FAIL;
                goto done;
            }

          /* end access */
          if (FAIL == ANendaccess(ann_id))
            {
                fprintf(stderr,"ANendaccess failed for %d'th desc for file %s \n",i,f_name);
                ret_value = FAIL;
                goto done;
            }

          /* reset id and free space for label */
          ann_id = FAIL;
          HDfree(desc);
          desc = NULL;
      } /* end for every file desc */ 

  done:
    if (ret_value == FAIL)
      { /* Failure cleanup */
          if (ann_id != FAIL)
              ANendaccess(ann_id);        
          if (desc != NULL)
              HDfree(desc);
      }
    /* Normal cleanup */

    return ret_value;
}	/* end print_all_file_descs() */

/* prints all relevant information that an HDF object can have
   including annotations */
static intn
print_list_obj(const char *fname,
               list_info_t *l_opts, 
               objinfo_t * o_info, 
               intn o_num, 
               int32 an_id)
{
    int32  i;
    char  *s = NULL;
    char  *buf = NULL;
    intn   ret_value = SUCCEED;

    switch (l_opts->verbosity)
      {
      case VSHORT:		/* short output */
          /* handled elsewhere */
          break;

      case VLONG:	/* long output */
          printf("%*d%*s%*d%*d%*ld\n",
                 NUM_FIELD_WIDTH, o_num,
                 TAGNAME_FIELD_WIDTH,
                 ((s = HDgettagsname(o_info->tag)) == NULL ? HDstrdup("Unknown") : s),
                 TAG_FIELD_WIDTH, o_info->tag, REF_FIELD_WIDTH, o_info->ref,
                 INDEX_FIELD_WIDTH, (long) o_info->index);
          HDfree(s);	/* free tagname string */
          s = NULL;
          break;

      case VDEBUG:		/* debugging output */
          printf("%*d%*s%*d%*d%*ld%*ld%*ld\n",
                 NUM_FIELD_WIDTH, o_num,
                 TAGNAME_FIELD_WIDTH,
                 ((s = HDgettagsname(o_info->tag)) == NULL ? HDstrdup("Unknown") : s),
                 TAG_FIELD_WIDTH, o_info->tag, REF_FIELD_WIDTH, o_info->ref,
                 INDEX_FIELD_WIDTH, (long) o_info->index,
                 OFFSET_FIELD_WIDTH, (long) o_info->offset,
                 LENGTH_FIELD_WIDTH, (long) o_info->length);
          HDfree(s);	/* free tagname string */
          s = NULL;
          break;
      }		/* end switch */

    /* find data labels for object if any */
    if (l_opts->name == TRUE)
      {
          if (FAIL == print_data_labels(fname, an_id, o_info->tag, o_info->ref))
            {
                fprintf(stderr,"Failed to print data labels for object with tag=%d, ref=%d in file %s\n",
                        o_info->tag, o_info->ref,fname);
                ret_value = FAIL;
                goto done;
            }
      } /* end if printing data labels */

      /*  printf("%*s%s\n", LABEL_FIELD_WIDTH, "Name/Label=", o_info->lab_info);*/
    if (l_opts->class == TRUE)
      {
      }		/* end if */

    /* find data descs for object if any */
    if (l_opts->desc == TRUE )
      {
          if (FAIL == print_data_descs(fname, an_id, o_info->tag, o_info->ref))
            {
                fprintf(stderr,"Failed to print data descriptions for object with tag=%d, ref=%d in file %s\n",
                        o_info->tag, o_info->ref,fname);
                ret_value = FAIL;
                goto done;
            }
      } /* end if print descriptions */

    /* printf("%*s%s\n", DESC_FIELD_WIDTH, "Description=", o_info->desc_info); */
    if (l_opts->spec == TRUE && o_info->is_special)
      {
          switch (o_info->spec_info->key)
            {
            case SPECIAL_LINKED:
                printf("\tLinked Block: first %ld standard %ld per unit %ld\n",
                       (long) o_info->spec_info->first_len,
                       (long) o_info->spec_info->block_len,
                       (long) o_info->spec_info->nblocks);
                break;

            case SPECIAL_EXT:
                printf("\tExternal File: path %s  offset %ld\n",
                       o_info->spec_info->path, (long) o_info->spec_info->offset);
                break;

            case SPECIAL_COMP:
                printf("\tCompressed Element: compression type: %s  modeling type %s\n",
                       (o_info->spec_info->comp_type == COMP_CODE_NONE ? "None" :
                        (o_info->spec_info->comp_type == COMP_CODE_RLE ? "Run-Length" :
                         (o_info->spec_info->comp_type == COMP_CODE_NBIT ? "N-Bit" : "Unknown"))),
                       (o_info->spec_info->model_type == COMP_MODEL_STDIO ? "Standard" : "Unknown"));
                break;

            case SPECIAL_CHUNKED:
                printf("\tChunked element: chunk size %d, ndims %d, [",
                       (intn)o_info->spec_info->chunk_size,  (intn)o_info->spec_info->ndims);
                for (i = 0; i < o_info->spec_info->ndims; i++)
                  {
                      printf("%d",(intn)o_info->spec_info->cdims[i]);
                      if(i != (o_info->spec_info->ndims -1 ))
                         printf(",");
                  }
                printf("]\n");
                break;

            default:
                printf("\t Do not understand special element type %d \n",
                       o_info->spec_info->key);
                break;
            }	/* end switch */
      }		/* end if */

    if (l_opts->group == TRUE && o_info->is_group)
      {
          DFdi       *g_obj = NULL;
          int32       num;

          if ((num = get_group_max(o_info->group_info)) != FAIL)
            {
                printf("\tContents: (%ld objects)\n", (long) num);
                g_obj = get_next_group(o_info->group_info, 0);
                while (g_obj != NULL)
                  {
                      printf("\t\t%-30s: (tag=%6d) ref=%d\n",
                             ((s = HDgettagsname(g_obj->tag)) == NULL ? HDstrdup("Unknown") : s),
                             g_obj->tag, g_obj->ref);
                      HDfree(s);	/* free tagname string */
                      s = NULL;
                      g_obj = get_next_group(o_info->group_info, 1);
                  }		/* end while */
            }	/* end if */
      }		/* end if */

  done:
    if (ret_value == FAIL)
      { /* Failure cleanup */
          if (s != NULL)
              HDfree(s);
          if (buf != NULL)
              HDfree(buf);
      }
    /* Normal cleanup */

    return ret_value;
}/* print_list_obj() */


/* print the library version of the file */
static void 
printfilever(int32 file_id)
{
    uint32 major, minor, release;
    char string[LIBVSTR_LEN+1];

    if (Hgetfileversion(file_id, &major, &minor, &release, string) == SUCCEED)
      {
        string[LIBVSTR_LEN] = '\0';		/* make it a null terminated string */
        printf("\nFile library version: ");
        printf("Major= %u, Minor=%u, Release=%u\nString=%s\n",
	    (unsigned)major, (unsigned)minor, (unsigned)release, string);
      }
    else
        printf("(Does not have library version information)\n");
}

/* low level object listing routine for HDF file */
intn 
do_list(intn curr_arg, 
        intn argc, 
        char *argv[], 
        dump_opt_t * glob_opts)
{
    list_info_t list_opts;		/* list options */
    filelist_t *f_list = NULL;	/* list of files to dump */
    objlist_t  *o_list = NULL;	/* list of DD objects in a file */
    objinfo_t  *o_info = NULL;	/* pointer to a DD object */
    char       *f_name = NULL;	/* current file name to list */
    int32       fid = FAIL;		/* HDF file ID */
    intn        obj_num;		/* number of the object we are displaying */
    intn        status;			/* status from various function calls */
    char       *s = NULL;		/* temporary character pointer */
    int32       an_id = FAIL;   /* annotation interface handle */
    intn        ret_value = SUCCEED;

    if (glob_opts->help == TRUE || curr_arg >= argc)
      {
          list_usage(argc, argv);
          goto done;
      }		/* end if */

    init_list_opts(&list_opts);
    if ((status = parse_list_opts(&list_opts, curr_arg, argc, argv)) == FAIL)
      {
          list_usage(argc, argv);
          ret_value = FAIL;
          goto done;
      }		/* end if */

    curr_arg += status;
    if (curr_arg >= argc || (f_list = make_file_list(curr_arg, argc, argv)) == NULL)
      {
          fprintf(stderr,"ERROR: No files to dump!\n");
          list_usage(argc, argv);
          ret_value = FAIL;
          goto done;
      }		/* end if */

    /* process each file */
    f_name = get_next_file(f_list, 0);
    while (f_name != NULL)
      {
          vinit_done = FALSE; /* reset global Vset variable */
          obj_num = 0;	      /* number of the object we are displaying */
          fid = FAIL;
          an_id = FAIL;
/* BMR: the last parameter should be non-negative integer accord. to RM */
          if ((fid = Hopen(f_name, DFACC_READ, -1)) != FAIL)
            {
                an_id = ANstart(fid);
                if (FAIL == an_id)
                  {
                      fprintf(stderr,"ANstart failed for file %s \n",f_name);
                      ret_value = FAIL;
                      goto done; 
                  }

                /* make list of all objects in file */
                o_list = make_obj_list(fid, 
                                   (list_opts.name == TRUE ? CHECK_LABEL : 0) |
                                   (list_opts.desc == TRUE ? CHECK_DESC : 0) |
                                   CHECK_GROUP | CHECK_SPECIAL);
                if (o_list != NULL)
                  {

                      /* print out filename, etc. */
                      printf("File: %s\n", f_name);
                      printfilever(fid);

                      /* check for file labels */
                      if (list_opts.name == TRUE)
                        {
                            if (FAIL == print_all_file_labels(f_name, an_id))
                              {
                                  fprintf(stderr,"Failed to print file labels for file %s\n", f_name);
                                  ret_value = FAIL;
                                  goto done;
                              }
                        }

                      /* check for file descriptions */
                      if (list_opts.desc == TRUE)
                        {
                            if (FAIL == print_all_file_descs(f_name, an_id))
                              {
                                  fprintf(stderr,"Failed to print file descriptions for %s \n", f_name);
                                  ret_value = FAIL;
                                  goto done;
                              }
                        }

                      /* sort list */
                      sort_obj_list(o_list, list_opts.order);

                      /* print out list header according to options */
                      print_list_header(&list_opts);

                      /* Special case for short output */
                      if (list_opts.verbosity == VSHORT)
                        {
                            uint16      last_tag = 0;

                            o_info = get_next_obj(o_list, 0);	/* get first DD object */
                            while (o_info != NULL)
                              {
                                  if ((list_opts.limit == LGROUP 
                                       || list_opts.limit == LNONE) 
                                      || list_opts.limit_tag == o_info->tag)
                                    {
                                        if (o_info->tag != last_tag)
                                          {
                                              printf("%s%-*s: (tag %d)\n", (last_tag == 0 ? "" : "\n"),
                                                     TAGNAME_FIELD_WIDTH,
                                                     ((s = HDgettagsname(o_info->tag)) == NULL ? HDstrdup("Unknown") : s),
                                                     o_info->tag);
                                              last_tag = o_info->tag;
                                              printf("\tRef nos: ");
                                              HDfree(s);	/* free tagname string */
                                              s = NULL; /* reset */
                                          }		/* end if */
                                        printf("%d ", o_info->ref);
                                    }	/* end if */
                                  o_info = get_next_obj(o_list, 1);		/* advance to the next DD object */
                              }		/* end while */
                            printf("\n");
                        }	/* end if */
                      else /* must be verbose output */
                        {
                            /* Loop through all the objects in the file */
                            o_info = get_next_obj(o_list, 0);	/* get first DD object */
                            while (o_info != NULL)
                              {
                                  switch (list_opts.limit)
                                    {
                                    default:
                                    case LNONE:
                                    case LGROUP:
                                        if (FAIL == print_list_obj(f_name, &list_opts, o_info, obj_num,an_id))
                                          {
                                              fprintf(stderr,"Failed to print object list for file %s \n", f_name);
                                              ret_value = FAIL;
                                              goto done;
                                          }
                                        break;

                                    case LTAGNUM:
                                    case LTAGNAME:
                                        if (list_opts.limit_tag == o_info->tag)
                                          {
                                              if (FAIL == print_list_obj(f_name, &list_opts, o_info, obj_num, an_id))
                                                {
                                                    fprintf(stderr,"Failed to print list of objects for file %s \n", f_name);
                                                    ret_value = FAIL;
                                                    goto done;
                                                }
                                          }
                                        break;
                                    }	/* end switch */
                                  obj_num++;
                                  o_info = get_next_obj(o_list, 1);		/* advance to the next DD object */
                              }		/*end while */
                        }	/* end else */
/* BMR: moved this block inside the if (o_list != NULL ) */
                     /* free the object list */
                     free_obj_list(o_list);
                     o_list = NULL;
                  }		/* end if */

                /* cleanup section */
                if (vinit_done == TRUE)
                  {
                      if (FAIL == Vfinish(fid))
                        {
                            fprintf(stderr,"Vfinish failed for file %s\n", f_name);
                            ret_value = FAIL;
                            goto done;
                        }
                  }

                if (FAIL == ANend(an_id))
                  {
                      fprintf(stderr,"ANend failed for file %s\n", f_name);
                      ret_value = FAIL;
                      goto done;
                  }
                an_id = FAIL; /* reset */

                if (Hclose(fid) == FAIL)
                  {
                      fprintf(stderr,"Hclose failed for file %s\n", f_name);
                      ret_value = FAIL;
                      goto done;
                  }
                fid = FAIL; /* reset */

            }	/* end if */
          else
            {
                fprintf(stderr,"WARNING: Hopen() failed, Possible invalid file name: %s\n", f_name);
                ret_value = FAIL;
                goto done;
            }

          /* next file processing */
          /* get next file to process */
          f_name = get_next_file(f_list, 1);

      }		/* end while processing files*/

  done:
    if (ret_value == FAIL)
      { /* Failure cleanup */
          if (fid != FAIL) /* check if file open still */
            {
                Hclose(fid);
                fid = FAIL;
            }
          if (an_id != FAIL) /* check if annotation handle still open */
            {
                ANend(an_id);
                an_id = FAIL;
            }
          if (s != NULL)
              HDfree(s);
          if (o_list != NULL)
              free_obj_list(o_list);
      }
    /* Normal cleanup */
    if (f_list != NULL)
        free_file_list(f_list);

    return ret_value;
}	/* end do_list() */
