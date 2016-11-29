#include "queryreader.h"

a3_Prefix *prefixes;
char **var_names;
int current_prefix_count = 0;
int prefix_array_size = 0;
char line_delimiter = '.';
char sub[URL_MAX], prd[URL_MAX], obj[URL_MAX];
a3_Triple *triples;
int triple_array_size = 0;
int current_triple = 0;
int var_names_count = 0;


/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////
int qr_readquery(char *filepath) {
	FILE *ptr;
	char *buf;
	//a3_Triple triple;
	
	// Setup 
	prefixes = NULL;
	buf = malloc(BUFFSIZE);
	ptr = fopen(filepath, "r");
	if( !ptr ) {
		fprintf(stderr, "Error opening file <%s>.\n", filepath);
		return FAILURE;
	}

	// Main functionality 
	int rc;
	while( fgets(buf, BUFFSIZE, ptr) != NULL ) {
		if( qr_check_empty(buf) == FAILURE) {
			qr_trim_whitespace(buf);
			qr_parseline(buf);
		}
	}

	// Teardown 
	fclose(ptr);
	free(buf);
	return SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////
int qr_check_empty(char *line) {
	while(line[0] == ' ' || line[0] == '\t' || line[0] == '\n') { 
		if ('\n' == line[0]) {
			return SUCCESS;
		} else {
			line ++;
		}
	}
	return FAILURE;
}

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////
void qr_trim_whitespace(char *line) {
	//TODO: Trim whitespace
	while (' ' == line[0] || '\t' == line[0]) {
		line ++;
	}
	// Replace newline with null character 
	int i; 
	for(i = 0; line[i]; i++) {
		if (line[i] == '\n') {
			line[i] = '\0';
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////
int qr_parseline(char *line) {

	char *copy = malloc(strlen(line));
	strcpy(copy, line);					// Create a safe to handle copy
	
	// Check for prefix
	if(strstr(copy, "PREFIX") || strstr(copy, "prefix")) {
		qr_add_prefix(copy, current_prefix_count + 1);
		current_prefix_count++;
		free(copy);
		return PARSED_PREFIX;
	}
	//Check for SUBJECT
	else if(strstr(copy, "SELECT") || strstr(copy, "select")) {
		qr_parse_select(copy);
	} else if (!strstr(copy, "}") || qr_check_empty(copy) == FAILURE) {
		printf("test1\n");
		qr_parse_where(copy);
		printf("test2\n");

	}

	free(copy);
	return SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////
void qr_add_prefix(char *line, int entry_num) {

	// Check the array. If prefix_count is larger, double size and record. 
	if(entry_num > prefix_array_size) {
		prefixes = realloc(prefixes, 2 * entry_num * sizeof(a3_Prefix));
		prefix_array_size = 2 * entry_num;
	}

	line += 7;			// Skips the 'prefix ' portion of the line 

	char *cur_short = strtok(line, ":");
	strtok(NULL, "<");
	char *cur_uri = strtok(NULL, ">");

	strcpy(prefixes[current_prefix_count].shorthand, cur_short);
	strcpy(prefixes[current_prefix_count].uri, cur_uri); 	
}

/////////////////////////////////////////////////////////////////////////////////
// Parses the select statement and stores the names of the return values for
// future use. This does not ensure ALL the variables are accounted for - 
// it just remembers the names of which to return. TODO: handle * returns
/////////////////////////////////////////////////////////////////////////////////
void qr_addmemto_triple(int entry_num) {

	// Check the array. If triple count is larger, double size and record. 
	if(entry_num > triple_array_size) {
		triples = realloc(triples, 2 * entry_num * sizeof(a3_Triple));
		triple_array_size = 2 * entry_num;
	} 	
}

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////
void qr_parse_select(char * line) {
	
	char *count_line = strdup(line);

	// Count the number of variables
	strtok(count_line, " ");
	char *var = strtok(NULL, " ");
	while(var != NULL) {
		if(strchr(var, '?')) {
			var_names_count++;
		}
		var = strtok(NULL, " ");
	}

	var_names = malloc(var_names_count * sizeof(char *));
	int i = 0;

	// Store the names
	strtok(line, " ");
	var = strtok(NULL, " ");
	while(var != NULL) {
		if(strchr(var, '?')) {
			var_names[i] = strdup(var);
			i++;
		}
		var = strtok(NULL, " ");
	}
}
	

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////
void qr_parse_where(char * line) {

	// Dynamically reallocate memory for triples for every where clause line
	qr_addmemto_triple(current_triple + 1);

	// Line delimiter tells us how to deal with the sub, prd, obj triples
	switch(line_delimiter) {
		
		case '.' :
			line_delimiter = line[strlen(line)-1];
			qr_parse_period(line);
			break;
		case ',' :
			line_delimiter = line[strlen(line)-1];
			qr_parse_comma(line);
			break;
		case ';' :
			line_delimiter = line[strlen(line)-1];
			qr_parse_semicolon(line);
			break;
		default :
		 fprintf(stderr, "File is improperly formatted.");
	}

	strcpy(triples[current_triple].sub, sub);
	strcpy(triples[current_triple].prd, prd);
	strcpy(triples[current_triple].obj, obj);

	qr_print_triples();
}

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////

void qr_parse_period(char *line) {
	char *subj, *pred, *obj;

	subj = strtok(line, "\t ");
	line++;
	pred = strtok(NULL, "\t ");
	line++;
	obj = strtok(NULL, "\n\0");
	obj[strlen(obj) - 2] = '\0';

	qr_parse_subject(subj);
	qr_parse_predicate(pred);
	qr_parse_object(obj);
}

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////

void qr_parse_comma(char *line) {
	char *obj = strtok(line, "\n\0");
	obj[strlen(obj) -2] = '\0';

	qr_parse_object(obj);
}

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////

void qr_parse_semicolon(char *line) {
	char *pred, *obj;

	pred = strtok(line, "\t");
	line++;
	obj = strtok(NULL, "\n\0");
	obj[strlen(obj) -2] = '\0';
	qr_parse_predicate(pred);
	qr_parse_object(obj);
}

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////

void qr_parse_subject(char *line) {
	char full_URI[URL_MAX];
	char *store;
	int i;

	if(line[0] == '?') {
		strcpy(full_URI, line);

	} else if(line[0] == '<') {
		line ++;
		store = strtok(line, ">");
		strcpy(full_URI, store);
	} else {

		store = strtok(line, ":");	

		for(i = 0; i < prefix_array_size; i++) {
			if(strcmp(prefixes[i].shorthand, store) == 0) {
				strcpy(full_URI, prefixes[i].uri);
				break;
			}
		}		

		line++;
		strcat(full_URI, strtok(NULL, "\t"));
	}

	strcpy(sub, full_URI);
}

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////

void qr_parse_predicate(char *line) {
	char full_URI[URL_MAX];
	char *store;
	int i;

	if(line[0] == '?') {
		strcpy(full_URI, line);

	} else	if(line[0] == '<') {
		line ++;
		store = strtok(line, ">");
		strcpy(full_URI, store);
	} else {

		store = strtok(line, ":");
		for(i = 0; i < prefix_array_size; i++) {
			if(strcmp(prefixes[i].shorthand, store) == 0) {
				strcpy(full_URI, prefixes[i].uri);
				break;
			}
		}	
		
		line++;
		strcat(full_URI, strtok(NULL, "\t"));
	}
	strcpy(prd, full_URI);
}

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////

void qr_parse_object(char *line) {
	
	char obj_URI[URL_MAX];
	char curChar = line[0];
	char *store;
	int i;

	//Search for ^^ -- dealing with prefix call
	char *objpref;
	objpref = strchr(line, '^');
	//Search for @ -- dealing with a literal
	char *langlit;
	langlit = strchr(line, '@');
	//Search for : -- for cases where the line doesnt start with "
	char *objpage;
	objpage = strchr(line, ':');

	if ('?' == line[0]) {
		strcpy(obj_URI, line);

	} else if ('<' == curChar) {  //Regular full address
		line ++;
		store = strtok(line, ">");
		strcpy(obj_URI, store);

	} else if ('"' == curChar) { // Deal with all three cases of obj's starting with "
		// deals with ^^
		if(objpref) {

			line = strtok(line, "^^");
			// Save this value to append later
			char *lineValue = line;

			strcpy(obj_URI, lineValue);
			strcat(obj_URI, "^^");

			line = strtok(NULL, "");
			line ++;

			store = strtok(line, ":");
			for(i = 0; i < prefix_array_size; i++) {
				if(strcmp(prefixes[i].shorthand, store) == 0) {
					strcat(obj_URI, prefixes[i].uri);
					break;
				}
			}

			// Append the resource to the prefix and store it in triple
			strcat(obj_URI, strtok(NULL, "\t"));
		
		// deals with @
		} else if(langlit) {
			store = strtok(line, "@");
			strcpy(obj_URI, store);

		// deals with string literal
		} else {
			line ++;
			line = strtok(line, "\"");
			store = line;
			strcpy(obj_URI, prd);
			strcat(obj_URI, "#");
			strcat(obj_URI, store);
		}

	} else if (objpage) {

		store = strtok(line, ":");
		for(i = 0; i < prefix_array_size; i++) {
			if(strcmp(prefixes[i].shorthand, store) == 0) {
				strcpy(obj_URI, prefixes[i].uri);
				break;
			}
		}
		// Append the resource to the prefix and store it in triple 
		line++;
		strcat(obj_URI, strtok(NULL, ""));

	} else { // deals with literals
		store = line;
		strcpy(obj_URI, prd);
		printf("not here\n");
		strcat(obj_URI, store);
	}

	// Store the obj in our triple struct
	strcpy(obj, obj_URI);
}

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////
void qr_print_triples() {
	int i;
	for(i = 0; i < current_triple; i++) {
		printf("%d \n- > %s\n- > %s\n- > %s\n", i, triples[i].sub, triples[i].prd, triples[i].obj);
	}
}

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////
void qr_print_prefixes() {
	int i;
	for(i = 0; i < current_prefix_count; i++) {
		printf("%d) %s : %s\n", i, prefixes[i].shorthand, prefixes[i].uri);
	}
}

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////
int qr_printfile(char *filepath) {
	FILE *ptr;
	char *buf;

	/* Setup */
	buf = malloc(BUFFSIZE);
	ptr = fopen(filepath, "r");
	if( !ptr ) {
		fprintf(stderr, "Error opening file <%s>.\n", filepath);
		return 1;
	}

	/* Main functionality */
	while( fgets(buf, BUFFSIZE, ptr) != NULL ) {
		printf("%s", buf);
	}

	/* Teardown */
	fclose(ptr);
	free(buf);
	return SUCCESS;
}