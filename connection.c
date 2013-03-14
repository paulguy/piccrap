#include "connection.h"

Connection *connection_init(int length) {
	Connection *connection;

	connection = (Connection *)malloc(sizeof(Connection));
	if(connection == NULL) {
		return(NULL);
	}
	connection->length = length;
	connection->stream = (double *)malloc(sizeof(double) * length);
	if(connection->stream == NULL) {
		free(connection);
		return(NULL);
	}

	return(connection);
}
