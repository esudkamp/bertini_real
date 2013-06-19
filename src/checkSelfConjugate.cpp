#include "checkSelfConjugate.hpp"







int checkSelfConjugate(witness_set W,
                       int           num_vars,
                       boost::filesystem::path input_file,
											 std::string stifle_text)
/***************************************************************\
 * USAGE: check if component is self conjugate                  *
 * ARGUMENTS: witness set, # of variables and name of input file*
 * RETURN VALUES: 1-self conjugate; 0-non self conjugate   *
 * NOTES:                                                        *
 \***************************************************************/
{
  int strLength = 0, digits = 15, *declarations = NULL;
  char *fmt = NULL;
	std::string bertini_command="bertini input_membership_test ";
	bertini_command.append(stifle_text);
  FILE *IN = NULL;
	

	
	
	
	// make the format to write the member_points file
  strLength = 1 + snprintf(NULL, 0, "%%.%dle %%.%dle\n", digits, digits);
  // allocate size
  fmt = (char *)bmalloc(strLength * sizeof(char));
  // setup fmt
  sprintf(fmt, "%%.%dle %%.%dle\n", digits, digits);
	
	
  // setup input file
  partition_parse(&declarations, input_file, "func_input_real", "config_real",0); // the 0 means not self conjugate

	
	
	//check existence of the required witness_data file.
	IN = safe_fopen_read("witness_data");
	fclose(IN);
	
	
	
	
	
	

	//only need to do this once.  we put the point and its conjugate into the same member points file and run the membership test simultaneously with one bertini call.
  membership_test_input_file("input_membership_test", "func_input_real", "config_real",3);
	
	
	//setup  member_points file, including both the first witness point, and its complex-conjugate
	write_member_points_sc(W.pts_d[0],fmt);
	
	// Do membership test
	std::cout << "*\n" << bertini_command << "\n*" << std::endl;
  system(bertini_command.c_str());
	
	int *component_numbers;
	component_numbers = (int *)bmalloc(2*sizeof(int));
	
	read_incidence_matrix(component_numbers);

  free(declarations);
	
	
	
  // delete temporary files

	
	if (component_numbers[0]==component_numbers[1]) {
		printf("component IS self conjugate\n");
		free(component_numbers);
		return 1;
	}
	else
	{
		printf("component is NOT self conjugate\n");
		free(component_numbers);
		return 0;
	}

	
}





int get_incidence_number(witness_set W,
												 int           num_vars,
												 boost::filesystem::path input_file,
												 std::string stifle_text)
/***************************************************************\
 * USAGE: check if component is self conjugate                  *
 * ARGUMENTS: witness set, # of variables and name of input file*
 * RETURN VALUES: 1-self conjugate; 0-non self conjugate   *
 * NOTES:                                                        *
 \***************************************************************/
{
  int strLength = 0, digits = 15, *declarations = NULL;
  char *fmt = NULL;
	std::string bertini_command="bertini input_membership_test ";
	bertini_command.append(stifle_text);
  
	FILE *IN = NULL;
	

	
	// make the format to write the member_points file
  strLength = 1 + snprintf(NULL, 0, "%%.%dle %%.%dle\n", digits, digits);
  // allocate size
  fmt = (char *)bmalloc(strLength * sizeof(char));
  // setup fmt
  sprintf(fmt, "%%.%dle %%.%dle\n", digits, digits);
	
	
  // setup input file
  partition_parse(&declarations, input_file, "func_input_real", "config_real",0); // the 0 means not self conjugate
	
	
	
	//check existence of the required witness_data file.
	IN = safe_fopen_read("witness_data"); fclose(IN);
	
	//perhaps this could be used to get the witness_data file, but there would be problems
	//  membership_test_input_file("input_membership_test", "func_input_real", "config_real",1);
	
	
	
	
	
	//only need to do this once.  we put the point and its conjugate into the same member points file and run the membership test simultaneously with one bertini call.
  membership_test_input_file("input_membership_test", "func_input_real", "config_real",3);
	
	
	//setup  member_points file, including both the first witness point, and its complex-conjugate
	write_member_points_singlept(W.pts_d[0],fmt);
	
	// Do membership test
	std::cout << "*\n" << bertini_command << "\n*" << std::endl;
  system(bertini_command.c_str());
	
	int component_number;
	
	read_incidence_matrix(&component_number);
	
  free(declarations);
	
	return component_number;
}



int write_member_points_singlept(point_d point_to_write, char * fmt){
	FILE *OUT = NULL;
	int ii;
	
	
	remove("member_points");
	OUT = safe_fopen_write("member_points");

	
	vec_d result;
	init_vec_d(result,0);
	dehomogenize_d(&result,point_to_write);
	
	fprintf(OUT,"1\n\n");
	for(ii=0;ii<result->size;ii++)
		fprintf(OUT, fmt, result->coord[ii].r, result->coord[ii].i);
	
	
	fclose(OUT);
	
	
	clear_vec_d(result);
	
	return 0;
}



int write_member_points_sc(point_d point_to_write, char * fmt){
	FILE *OUT = NULL;
	int ii;
	
	
	remove("member_points");
	OUT = safe_fopen_write("member_points");

	
	vec_d result;
	init_vec_d(result,0);
	dehomogenize_d(&result,point_to_write);
	
	fprintf(OUT,"2\n\n");
	for(ii=0;ii<result->size;ii++)
	{
		fprintf(OUT, fmt, result->coord[ii].r, result->coord[ii].i);
	}
	
	fprintf(OUT,"\n");
	for(ii=0;ii<result->size;ii++)
	{
		fprintf(OUT, fmt, result->coord[ii].r, -result->coord[ii].i);
	}
	fclose(OUT);
	
	clear_vec_d(result);
	
	return 0;
}




void read_incidence_matrix(int *component_numbers){
	FILE *IN = NULL;
	int ii,jj;
	int num_nonempty_codims, num_components, num_pts, codim;
	int component_indicator, component_number,total_num_components=0;

  //read incidence_matrix and see if it is self-conjugate
  IN = fopen("incidence_matrix", "r");
  if (IN == NULL)
  {
    printf("\n\nERROR: indicence_matrix file not produced.\n");
    bexit(ERROR_FILE_NOT_EXIST);
  }
	
	
  fscanf(IN, "%d",&num_nonempty_codims);      // number of nonempty codimensions
	
	for (ii = 0; ii<num_nonempty_codims; ii++) {
		fscanf(IN, "%d",&codim);      // codimension  (iterated for each codimension?)
		fscanf(IN, "%d",&num_components);  // number of components (is whatever)
		total_num_components = total_num_components+num_components;
	}

	
  fscanf(IN, "%d",&num_pts);    // number of points (should be 1)
//	printf("reading incidence for %d pts\n",num_pts);

	//and then a binary matrix indicating membership on which component
	//from the appendices of the book:
	//	% Binary matrix with one row per point, columns corresponding to components.
	//	%                0 if given point is not on given component;
	//	%                1 else .
	for (jj=0; jj<num_pts; jj++) {
		component_number=-10;
		for(ii=0;ii<total_num_components;ii++)  // i don't this is correct if there is more than one nonempty codimension.
			//todo: check this iterator limit  !!!
		{
			fscanf(IN, "%d", &component_indicator);
			if (component_indicator==1) {  //then is on this component
				component_number = ii;
			}
		}
		
		if (component_number==-10) {
			printf("it appears the candidate point lies on NO COMPONENT.\n");
		}
		component_numbers[jj]=component_number;
	}
	
	fclose(IN);


	
	return;
}



void read_incidence_matrix_wrt_number(int *component_numbers, int given_incidence_number){
	FILE *IN = NULL;
	int ii,jj;
	int num_nonempty_codims, num_components, num_pts, codim;
	int component_indicator, component_number,total_num_components=0;
	
  //read incidence_matrix and see if it is self-conjugate
  IN = fopen("incidence_matrix", "r");
  if (IN == NULL)
  {
    printf("\n\nERROR: indicence_matrix file not produced.\n");
    bexit(ERROR_FILE_NOT_EXIST);
  }
	
	
  fscanf(IN, "%d",&num_nonempty_codims);      // number of nonempty codimensions
	
	for (ii = 0; ii<num_nonempty_codims; ii++) {
		fscanf(IN, "%d",&codim);      // codimension  (iterated for each codimension?)
		fscanf(IN, "%d",&num_components);  // number of components (is whatever)
		total_num_components = total_num_components+num_components;
	}
	
	
  fscanf(IN, "%d",&num_pts);    // number of points (should be 1)
	
	//and then a binary matrix indicating membership on which component
	//from the appendices of the book:
	//	% Binary matrix with one row per point, columns corresponding to components.
	//	%                0 if given point is not on given component;
	//	%                1 else .
	for (jj=0; jj<num_pts; jj++) {
		component_number=0;
		for(ii=0;ii<total_num_components;ii++)  // i don't know if this is correct if there is more than one nonempty codimension.
		{
			fscanf(IN, "%d", &component_indicator);
			if (component_indicator==1 && given_incidence_number==ii) {  //then is on this component
				component_number++;
			}
		}
		if (component_number==0) {
//			printf("test point did not lie on any component at all.\n");
			//			exit(-1);
		}
		component_numbers[jj]=component_number;
	}
	
	fclose(IN);

	
	return;
}





void membership_test_input_file(boost::filesystem::path outputFile,
                                boost::filesystem::path funcInput,
                                boost::filesystem::path configInput,
                                int  tracktype)
/***************************************************************\
 * USAGE: setup input file to membership test                    *
 * ARGUMENTS: name of output file, function & configuration input*
 * RETURN VALUES: none                                           *
 * NOTES:                                                        *
 \***************************************************************/
{
  char ch;
  FILE *OUT = safe_fopen_write(outputFile), *IN = NULL;

	
  // setup configurations in OUT
  fprintf(OUT, "CONFIG\n");
  IN = safe_fopen_read(configInput);

	
	while ((ch = fgetc(IN)) != EOF)
    fprintf(OUT, "%c", ch);
  fclose(IN);
  if(tracktype==3)
    fprintf(OUT, "TrackType: %d;\nDeleteTempFiles: 1;\nEND;\nINPUT\n",tracktype);
  else
    fprintf(OUT, "TrackType: %d;\nDeleteTempFiles: 1;\nEND;\nINPUT\n",tracktype);
	
  // setup system in OUT
  IN = safe_fopen_read(funcInput);

  while ((ch = fgetc(IN)) != EOF)
    fprintf(OUT, "%c", ch);
  fclose(IN);
  fprintf(OUT, "END;\n");
  fclose(OUT);
	
  return;
}




