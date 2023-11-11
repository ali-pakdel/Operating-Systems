#include <stdio.h> 
#include <string.h>
#include <stdlib.h> 
#include <errno.h> 
#include <unistd.h> 
#include <arpa/inet.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <fcntl.h>
#include <sys/time.h>

#define MAX_CLIENTS 50
#define MAX_STR 1024

#define COMP_OR_CIVIL 'c'
#define COMP 'o'
#define CIVIL 'i'
#define ELEC 'e'
#define MECH 'm'

int next_port;

char buffer[MAX_STR]; 

int setup_server(int port)
{
	struct sockaddr_in address; 
	int opt = 1;
	int main_fd;

	if( (main_fd = socket(AF_INET , SOCK_STREAM , 0)) == 0) 
	{ 
		perror("Error!\n"); 
		exit(1); 
	} 

	if( setsockopt(main_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 ) 
	{ 
		perror("Error in set sock opt!\n"); 
		exit(1); 
	} 
	
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons(port); 
		
	if (bind(main_fd, (struct sockaddr *)&address, sizeof(address))<0) 
	{ 
		perror("Error in binding!\n"); 
		exit(1); 
	} 

	if (listen(main_fd, MAX_CLIENTS) < 0) 
	{ 
		perror("Error in listening!\n"); 
		exit(1); 
	} 

	return main_fd;
}

int accept_client(int main_socket) {
    int client_fd;
    struct sockaddr_in client_address;
    int address_len = sizeof(client_address);
    client_fd = accept(main_socket, (struct sockaddr *)&client_address, (socklen_t*) &address_len);

    return client_fd;
}


char* choose_file_name()
{
	char file_name[MAX_STR];
	switch (buffer[0])
	{
	case COMP_OR_CIVIL:
		if (buffer[1] == COMP)
			strcpy(file_name, "ComputerEng.txt");
		else if (buffer[1] == CIVIL)
			strcpy(file_name, "CivilEng.txt");
		break;
	case ELEC:
		strcpy(file_name, "ElectricalEng.txt");
		break;
	case MECH:
		strcpy(file_name, "MechanicalEng.txt");
		break;
	default:
		break;
	}
}

int main(int argc , char *argv[]) 
{
	int main_port = atoi(argv[1]);
	next_port = main_port + 1;

	fd_set master_set, working_set; 

	int main_socket = setup_server(main_port);

	int addrlen , new_fd , 
	activity, i , valread , sd; 
		
	printf("Server is created, now waiting for connections\n"); 
	int topics_size[4] = {0, 0, 0, 0};
	int topic_id[4][3] = {{0}};
	
	FD_ZERO(&master_set); 
	int max_sd = main_socket;
	FD_SET(main_socket, &master_set);
	
	while(1) 
	{ 
		working_set = master_set;
		select(max_sd + 1 , &working_set , NULL , NULL , NULL);

		for (int i = 0; i <= max_sd; i++) 
		{
            if (FD_ISSET(i, &working_set)) 
			{
                if (i == main_socket) 
				{
                    new_fd = accept_client(main_socket);
                    FD_SET(new_fd, &master_set);
                    if (new_fd > max_sd)
                        max_sd = new_fd;
					printf("New client connected, with fd: %d\n", new_fd - 3);
				}
                else 
				{
                    int bytes_received;
                    bytes_received = recv(i , buffer, MAX_STR, 0);

                    if (bytes_received == 0) 
					{
                        printf("Client %d left\n", i - 3);
                        close(i);
                        FD_CLR(i, &master_set);
                        continue;
                    }
					else
					{
						if (buffer[0] <= '4' && buffer[0] >= '1')
						{
							int topic = atoi(buffer);
							printf("Clinet %d chose topic %d\n", i - 3, topic);
		
							topics_size[topic - 1]++;
							topic_id[topic - 1][topics_size[topic - 1] - 1] = i - 3;

							if(topics_size[topic - 1] == 3)
							{
								int index = 0;
								int id_turn[6];
								for (int k = 0; k < 3; k++)
								{
									id_turn[k] = topic_id[topic - 1][k]; 
								}
								int turn = 1;
								printf("New room created for topic %d on port %d\n", topic, next_port);
								next_port++;
								topics_size[topic - 1] = 0;
								for(int j = 0; j < 3; j++)
								{
									bzero(buffer, sizeof(buffer));
									snprintf(buffer, sizeof(buffer), "%d %d %d %d %d %d %d %d", next_port - 1, turn, 
											id_turn[0], id_turn[1], id_turn[2], 1, 2, 3);
										
									turn++;
									int fd = id_turn[j] + 3; 
									send(fd , buffer , strlen(buffer) , 0); 

									for (int h = 0; h < 3; h++)
									{
										topic_id[topic - 1][h] = 0;
									}
								}
							}
						}
						else if (buffer[0] == COMP_OR_CIVIL || buffer[0] == ELEC || 
								buffer[0] == MECH)
						{
							char* file_name = choose_file_name();
							int file;
							if ((file = open(file_name, O_APPEND | O_WRONLY)) < 0)
							{
								file = open(file_name, O_WRONLY | O_CREAT, S_IWUSR | S_IRUSR);
							}
							strcat(buffer, "\n");
							write(file, buffer+2, strlen(buffer) - 2);
							close(file);
						}	
					}
				}
			}
		}
	} 		
	return 0; 
}
