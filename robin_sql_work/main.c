#include "main.h"
#include <string.h>
#include "../src/assignment3.h"

typedef struct QueryStatement {
	a3_Triple triple;
	int var_loc;
} Query;


int main(int argc, char **argv) {


	int num_lines = 2;
	int num_var = 1;


	Query *queries = malloc(sizeof(Query) * 2);

	strcpy(queries[0].triple.sub, "?city");
	strcpy(queries[0].triple.prd, "rdf:type");
	strcpy(queries[0].triple.obj, "schema:City");
	queries[0].var_loc = 1;

	strcpy(queries[1].triple.sub, "?city");
	strcpy(queries[1].triple.prd, "dbo:country");
	strcpy(queries[1].triple.obj, "dbr:Canada");
	queries[1].var_loc = 1;

	char SELECT[200];
	strcpy(SELECT, "SELECT ");
	char FROM[200];
	strcpy(FROM, "FROM ");
	char WHERE[1000];
	strcpy(WHERE, " WHERE ");

	// Make the SELECT statement.
	int i;
	for(i = 0; i < num_var; i++) {
		if(queries[i].var_loc == 1) {
			strcat(SELECT, sprintf("t%d.sub ", i));
			printf("%s\n", SELECT);
		}
	}

	return 0;
}