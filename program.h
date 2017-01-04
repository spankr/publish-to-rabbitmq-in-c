#ifndef __PROGRAM_H
#define __PROGRAM_H

/*
 * Important header stuff goes here.
 * Borrowed heavily from https://github.com/alanxz/rabbitmq-c
 */

#include <stdio.h>


/**
 * boolean type 0 = false, true otherwise
 *
 * \since v0.1
 */
typedef int amqp_boolean_t;

/**
 * Parameters used to connect to the RabbitMQ broker
 *
 * \since v0.2
 */
struct amqp_connection_info {
  char *user;                 /**< the username to authenticate with the broker, default on most broker is 'guest' */
  char *password;             /**< the password to authenticate with the broker, default on most brokers is 'guest' */
  char *host;                 /**< the hostname of the broker */
  char *vhost;                /**< the virtual host on the broker to connect to, a good default is "/" */
  int port;                   /**< the port that the broker is listening on, default on most brokers is 5672 */
  amqp_boolean_t ssl;
};

#endif