#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <errno.h> 
#include <unistd.h> 
#include <arpa/inet.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/time.h>
#include <sys/select.h>
#include <signal.h>

#define MAX 1024
#define ROOM_SIZE 3

#define COMP_ENG 1
#define ELEC_ENG 2
#define MECH_ENG 3
#define CIVIL_ENG 4

#define COMP_ID "co"
#define ELEC_ID "el"
#define MECH_ID "me"
#define CIVIL_ID "ci"

int udp_socket;

char buffer[MAX];

char question_answer[ROOM_SIZE][MAX];
int id_turn[6];

struct sockaddr_in bc_address; 

void init_broadcast_server(int room_port)
{
    int broadcast = 1, opt = 1;
	if ((udp_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
        perror("Error in room socket creation!\n"); 
	}
        
    if (setsockopt(udp_socket, SOL_SOCKET, SO_BROADCAST, (char*)&broadcast, sizeof(broadcast)) < 0)
        perror("Error in enabling room broadcast!\n");
    if (setsockopt(udp_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) < 0)
        perror("Error in enabling reuse!\n");

    bc_address.sin_family = AF_INET; 
    bc_address.sin_port = htons(room_port); 
    bc_address.sin_addr.s_addr = inet_addr("127.255.255.255");
    int addrlen = sizeof(bc_address);

    if (bind(udp_socket, (struct sockaddr*)&bc_address, (socklen_t)addrlen) < 0)
	{
        perror("Error in binding room socket!\n");
	}

	printf("Room is availabe for clients\n\n");
}

char* my_itoa(int val, int base){
	
	static char buf[32] = {0};
	
	int i = 30;
	
	for(; val && i ; --i, val /= base)
	
		buf[i] = "0123456789abcdef"[val % base];
	
	return &buf[i+1];
	
}

void ask_q(int client_id)
{
	int read_bytes;

	printf("Ask your question: \n");

	memset(buffer, 0, 1024);
	read_bytes = read(0, buffer, 1024);

	char* index = my_itoa(client_id, 10);

	char text[100] = "Client ";
	strcat(text, index);
	char text2[9] = " asked: ";

	strcat(text, text2);

	strcat(text, buffer);
	strcpy(buffer, text);

	if (sendto(udp_socket, &buffer, sizeof(buffer), 0, (struct sockaddr *)&bc_address, sizeof(bc_address))!= sizeof(buffer))
    {
        perror("Error in sending!\n");
    }
	printf("Your question has been sent to other clinets!\n\n");
}


void alarm_handler(int sig)
{
	printf("Your ran out of time!\n");
}

void answer_q(int client_id)
{
	printf("Answer the question: \n");

	alarm(5);
	memset(buffer, 0, 1024);
	int read_bytes = read(0, buffer, 1024);
	alarm(0);

	char* index = my_itoa(client_id, 10);
	char text[MAX] = "Client ";
	char text2[MAX];
	strcat(text, index);

	if (read_bytes == -1)
	{
		strcpy(buffer, "didn't answer!\n");
	}
	else
	{
		strcpy(text2, " answered: ");
	}
	
	strcat(text, text2);
	strcat(text, buffer);
	strcpy(buffer, text);

	if (sendto(udp_socket, &buffer, sizeof(buffer), 0, (struct sockaddr *)&bc_address, sizeof(bc_address))!= sizeof(buffer))
    {
        perror("Error in sending!\n");
    }

	printf("Your answer has been sent!\n");
}

void get_q()
{	
	memset(buffer, 0, sizeof(buffer));
	recvfrom(udp_socket, buffer, sizeof(buffer), 0, (struct sockaddr*)NULL, NULL); 
	printf("Question : %s\n", buffer);
}

int get_answer()
{
	memset(buffer, 0, sizeof(buffer));
    recvfrom(udp_socket, buffer, sizeof(buffer), 0, (struct sockaddr*)NULL, NULL); 
	int id = buffer[7] - '0';
	return id;
}

int find_star_client(int star_id)
{
	int i;
	for (i = 0; i < ROOM_SIZE; i++)
	{
		if (star_id == id_turn[i])
			return id_turn[i + ROOM_SIZE];
	}
}

void send_file_to_server(int server_fd, int room, int best_answer_id)
{
	char room_name[MAX];
	switch (room)
	{
	case COMP_ENG:
		strcat(room_name, COMP_ID);
		break;
	case ELEC_ENG:
		strcat(room_name, ELEC_ID);
		break;
	case MECH_ENG:
		strcat(room_name, MECH_ID);
		break;
	case CIVIL_ENG:
		strcat(room_name, CIVIL_ID);
		break;
	default:
		break;
	}

	char star[MAX] = "*";
	char final_text[3072];
	strcat(star, question_answer[best_answer_id]);
	strcpy(question_answer[best_answer_id], star);

	sprintf(final_text, "%s%s%s%s", room_name, question_answer[0], 
							question_answer[1], question_answer[2]);

    write(server_fd, final_text, sizeof(final_text));

	printf("File has been sent to server succssefully!\n");
}

void choose_best_answer(int sockfd, int ask_turn, int topic)
{
	printf("Which client had the best answer?\n");

	memset(buffer, 0, MAX);
	int read_bytes = read(0, buffer, MAX);

	char star_id = buffer[0];
	char* index = my_itoa(star_id - '0', 10);
	char text[100] = "Client ";
	char text2[100] = " had the best answer.\n ";

	strcat(text, index);
	strcat(text, text2);
	strcpy(buffer, text);

	if (sendto(udp_socket, &buffer, sizeof(buffer), 0, (struct sockaddr *)&bc_address, sizeof(bc_address))!= sizeof(buffer))
    {
        perror("Error in sending!\n");
    }

	memset(buffer, 0, sizeof(buffer));
	recvfrom(udp_socket, buffer, sizeof(buffer), 0, (struct sockaddr*)NULL, NULL);
	
	star_id = find_star_client(star_id - '0');
	send_file_to_server(sockfd, topic, star_id - 1);
}



void start_room(int sockfd, int room_port, int ask_turn, int client_id, int topic)
{
    init_broadcast_server(room_port); 

	int i, j;
	int asked_questions = 0;
	while (asked_questions != ROOM_SIZE)
	{
		int counter = 0, index = 1;
		if (ask_turn == 1)
		{
			ask_q(client_id);
		}
		memset(buffer, 0, sizeof(buffer));
		recvfrom(udp_socket, buffer, sizeof(buffer), 0, (struct sockaddr*)NULL, NULL);
		char txt[MAX];
		strcpy(txt, "Question: ");
		strcat(txt, buffer + 16);
		strcat(txt, "Answers:\n\n");
		strcpy(question_answer[0], txt);
		if (ask_turn != 1)
			printf("%s\n", buffer);
		int temp_id = buffer[7] - '0';
		int id, printed_ans = 0;
		for (j = 0; j < ROOM_SIZE; j++)
		{
			if (temp_id == id_turn[j])
				id = id_turn[j + ROOM_SIZE];
		}

		while (1)
		{	
			if (counter == 4)
			{
				memset(buffer, 0, sizeof(buffer));
				recvfrom(udp_socket, buffer, sizeof(buffer), 0, (struct sockaddr*)NULL, NULL);

				char text[MAX] = "Client x had the best answer.\n";
				int error_flag = 0;
				for (i = 0; i < 30; i++)
				{
					if (i == 7)
						continue;
					if (text[i] != buffer[i])
						error_flag = 1; 
				}
				if (error_flag == 0)
				{
					printf("%s\n", buffer);
					break;
				}	
			}
			if (ask_turn == id + 1 && counter != 4)
			{
				answer_q(client_id);
				counter = 4;
			}
			else if (ask_turn == id)
			{
				id = get_answer();
				printf("%s\n", buffer);
				if (printed_ans == 0)
					strcat(buffer, "\n");
				else if (printed_ans == 1)
					strcat(buffer, "\n");
				printed_ans++;
				strcpy(question_answer[index], buffer + 19);
				index++;
				counter++;
				id = 1;
			}
			else if (ask_turn == id + 2)
			{
				temp_id = get_answer();
				for (j = 0; j < ROOM_SIZE; j++)
				{
					if (temp_id == id_turn[j])
					id = id_turn[j + ROOM_SIZE];
				}
			}
			if (counter == 2)
			{
				choose_best_answer(sockfd, ask_turn, topic);
				break;
			}
				
		}
		asked_questions++;
		ask_turn--;
		if (ask_turn == 0)
			ask_turn = 3;
		for (i = 3; i < 6; i++)
		{
			id_turn[i]--;
			if (id_turn[i] == 0)
				id_turn[i] = 3;
		}
	}
}

void choosing_topic(int sockfd) 
{ 
	char buff[MAX];
    int room_port, ask_turn, client_id;

	printf("Which topic do you want to join?\n1. Computer Eng\n2. Electrical Eng\n3. Mechanical Eng\n4. Civil Eng\n");
    
	memset(buff, 0, 1024);
    read(0, buff, 1024);

	int topic_selected = buff[0] - '0';

    write(sockfd, buff, sizeof(buff));

	memset(buff, 0, 1024);
	read(sockfd, buff, sizeof(buff)); 
    
	sscanf(buff, "%d %d %d %d %d %d %d %d", &room_port, &ask_turn, &id_turn[0], &id_turn[1], &id_turn[2], 
									  &id_turn[3], &id_turn[4], &id_turn[5]);

	fflush(stdout);
	if (ask_turn == 1)
		client_id = id_turn[0];
	if (ask_turn == 2)
		client_id = id_turn[1];
	if (ask_turn == 3)
		client_id = id_turn[2];
	
    printf("You joined a new room\nRoom port: %d \nYour ID: %d\nAsking turn: %d\n\n", room_port, client_id, ask_turn);
    start_room(sockfd, room_port, ask_turn, client_id, topic_selected);
} 

int connect_to_server(int port)
{
	int socket_fd;

	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in server_addr;

	if (socket_fd == -1) 
	{ 
		perror("Error in socket!\n"); 
	}

	server_addr.sin_family = AF_INET; 
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
	server_addr.sin_port = htons(port); 

	if (connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) 
	{ 
		perror("Error in connecting to the server!\n"); 
	}

	printf("Succsessfully connected to the server\n"); 

	return socket_fd;
}

int main(int argc, char** argv) 
{ 
    signal(SIGALRM, alarm_handler);
    siginterrupt(SIGALRM, 1);

	int port = atoi(argv[1]);
	int socket_fd = connect_to_server(port);

	choosing_topic(socket_fd); 

	close(socket_fd); 
} 
