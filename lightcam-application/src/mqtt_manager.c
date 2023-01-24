/**
 * @file
 * A simple program that subscribes to a topic.
 */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "mqtt_manager.h"
#include "posix_sockets.h"

int sockfd = -1;
pthread_t client_daemon = NULL;
struct mqtt_client client;

/**
 * @brief The client's refresher. This function triggers back-end routines to
 *        handle ingress/egress traffic to the broker.
 *
 * @note All this function needs to do is call \ref __mqtt_recv and
 *       \ref __mqtt_send every so often. I've picked 100 ms meaning that
 *       client ingress/egress traffic will be handled every 100 ms.
 */
void* client_refresher(void* client);

/**
 * @brief Safelty closes the \p sockfd and cancels the \p client_daemon before \c exit.
 */

int mqtt_manager_init(char* addr, char* port, char* topic, char* user, char* password, void *publish_callback)
{
	printf("addr:%s\n",addr);
	printf("port:%s\n",port);
	printf("topic:%s\n",topic);
	printf("user:%s\n",user);
	
	// exit if already init
	mqtt_manager_uninit();
 
    /* open the non-blocking TCP socket (connecting to the broker) */
    sockfd = open_nb_socket(addr, port);

	printf("sockfd:%d\n",sockfd);
	
    if (sockfd == -1) {
        perror("Failed to open socket: ");
        mqtt_manager_uninit();
		exit(0);
    }

    /* setup a client */
    uint8_t sendbuf[2048]; /* sendbuf should be large enough to hold multiple whole mqtt messages */
    uint8_t recvbuf[1024]; /* recvbuf should be large enough any whole mqtt message expected to be received */
    enum MQTTErrors err = mqtt_init(&client, sockfd, sendbuf, sizeof(sendbuf), recvbuf, sizeof(recvbuf), publish_callback);
	
	printf("err:%s\n",mqtt_error_str(err));

	/* Create an anonymous session */
    const char* client_id = "publishing_client";
    /* Ensure we have a clean session */
    uint8_t connect_flags = MQTT_CONNECT_CLEAN_SESSION;
    /* Send connection request to the broker. */
    err = mqtt_connect(&client, client_id, NULL, NULL, 0, user, password, connect_flags, 400);
    //err = mqtt_connect(&client, client_id, NULL, NULL, 0, NULL, NULL, connect_flags, 400);
	
	printf("err:%s\n",mqtt_error_str(err));
	
    /* check that we don't have any errors */
    if (client.error != MQTT_OK) {
        fprintf(stderr, "error: %s\n", mqtt_error_str(client.error));
        mqtt_manager_uninit();
		exit(0);
    }

    /* start a thread to refresh the client (handle egress and ingree client traffic) */
    if(pthread_create(&client_daemon, NULL, client_refresher, &client)) {
        fprintf(stderr, "Failed to start client daemon.\n");
        mqtt_manager_uninit();
		exit(0);

    }
	printf("topic:%s\n",topic);
	
    /* subscribe */
    err = mqtt_subscribe(&client, topic, 0);
	
	printf("err:%s\n",mqtt_error_str(err));
	
}

int mqtt_manager_publish(char* topic, char *message)
{
	/* get the current time */
	printf("%s published : \"%s\"\n", topic, message);
	/* publish the time */
	enum MQTTErrors err = mqtt_publish(&client, topic, message, strlen(message) + 1, MQTT_PUBLISH_QOS_0);
	
	printf("err:%s\n",mqtt_error_str(err));
	
	/* check for errors */
	if (client.error != MQTT_OK) {
		fprintf(stderr, "error: %s\n", mqtt_error_str(client.error));
		mqtt_manager_uninit();
	}
} 


void mqtt_manager_uninit()
{
    if (sockfd != -1) close(sockfd);
    if (client_daemon != NULL) pthread_cancel(client_daemon);
}


void* client_refresher(void* client)
{
    while(1)
    {
        mqtt_sync((struct mqtt_client*) client);
        usleep(100000U);
    }
    return NULL;
}