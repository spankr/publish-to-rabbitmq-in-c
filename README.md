# Publish to RabbitMQ With C

The purpose of this example is quick & dirty. It's meant to serve as a low-level example on how to establish a tcp connection to a local RabbitMQ server,
perform the handshake, publish a message to a pre-existing queue, and then shut down the connection gracefully.

I wrote this as a guide for writing embedded logic on a microcontroller. That microcontroller only needs to "report" on a status when it changes, 
aka an alert.

My goal was not elegance. My goal was to create an MVP (minimally viable product) that is purely a learning tool...so yes, there is repeated code 
and its not as [clean] as it could be.

## My Environment

I am running a [RabbitMQ] server on a local [Docker] container. The [rabbitmq/3-management][1] image, to be precise.

Default Rabbit MQ setup:

* Port ```5672```
* Username ```guest```
* Password ```guest```
* Virtual Host ```/```

I created a queue named, ```lee.test.q```.

I am using [CMake] as my build utility but it's a bit of overkill for this. (That and I'm getting used to CMake) 

My files: 

1. program.c 
    * The main program lives here. So does virtually all of the code.
1. program.h 
    * The header file. Includes, some defines and some structure definitions for the most part. I also included some function prototypes for visibility.
1. converters.c 
    * When playing with arrays of bytes, sometimes you have to extract out short and integer numbers. I have some conversion routines in here to help me out.

## Specs
I had to do a lot of pouring over [AMQP specifications][2] and cross-referencing things because they just weren't clear enough for me, right off the bat.
RabbitMQ has some nice documentation on the various [AMQP methods][4] I needed but to really track down the details of the data structures and values,
I had to use the [XML specification][3].

* [AMQP Protocol Specification][2]
* [AMQP XML Specification][3]
* [RabbitMQ AMQP 0-9-1 Quick Reference][4]
* [Simple Authentication and Security Layer (SASL)][SASL]

## The Job
The job that this code performs is to publish the message, _"Hello, there!"_ to the ```lee.test.q``` queue on my local RabbitMQ server. I foolishly thought
it was a simple matter of opening a TCP socket and pushing some bytes to it. Oh how naive that thought was!

Turns out, my steps were more along the lines of:

1. Open a TCP socket to the server
1. Push a Protocol message to get things started: ```{'A','M','Q','P',0,0,9,1}```
1. Get back a **Start** message
1. Respond with a **Start OK** message
1. Get a **Tune** message
1. Respond with a **Tune OK** message
1. Push an **Open** message
1. Push a message to **Open Channel** for Channel 1.
   * Channel 0 is the _system_ channel for things like establishing connections.
1. Push a message that we want to **Publish** some content
1. Push a **Content Header** that describes the content
1. Push the **Content**, which for us is _"Hello, there!"_
1. Push a **Close** connection message
1. Shutdown our TCP socket.


### Examining the RabbitMQ Logs
One of the nice things when troubleshooting socket communication is being able to see what is going on from both sides of the communication. I spent a **lot**
of time just guessing what the server was seeing and how it was handling things. Those guesses led to wrong assumptions and a lot of headache/time wasted.
Then Google came to the rescue with a nice little command for viewing a tail of the RabbitMQ server's logs!
```
docker logs localrabbit
```

Omg, this is helpful! Of course, I named my docker container to ```localrabbit``` so if you didn't do that, the command will be slightly different for you.
It was telling me if the server was having difficulties parsing my messages (it was) and dumped the byte arrays for me. Normal AMQP specs state that if anything
looks weird or wrong, just kill the connection. With these logs, I can now see why the server was killing my socket.

### Thank You!
No work nor assistance should go unpunis....er, unrecognized. I'll freely admit that I was only able to get this done with a lot of help from others who have
done better work than I.

1. [rabbitmq-c](https://github.com/alanxz/rabbitmq-c) library. 
   * This is a nice C library maintained by [alanxz](https://github.com/alanxz). If you are looking for a library for RabbitMQ communication, this is what you want!
1. [RabbitMQ .NET Client](https://github.com/rabbitmq/rabbitmq-dotnet-client)
   * I datamined this library for message structures when the specs failed me.
1. [AMQP-CPP](https://github.com/CopernicaMarketingSoftware/AMQP-CPP)
   * Another project I dug through for information.
1. [StackOverflow](http://stackoverflow.com/questions/3784263/converting-an-int-into-a-4-byte-char-array-c)
   * I haven't messed with bits && bytes since before StackOverflow was born. Packing/Unpacking integers to bytes escaped me one afternoon. Thank you, StackOverflow!
1. [Google]
   * I still remember what programming was like before there was Google. I don't want to go back to those days.


[RabbitMQ]: https://www.rabbitmq.com "Rabbit MQ"
[Docker]: https://www.docker.com "Docker"
[CMake]: https://cmake.org "CMake"
[clean]: https://www.google.com/webhp?sourceid=chrome-instant&ion=1&espv=2&ie=UTF-8#q=writing%20clean%20code "Writing clean code"
[SASL]: https://tools.ietf.org/html/rfc4616 "SASL Spec"
[Google]: https://www.google.com/
[1]: https://hub.docker.com/_/rabbitmq/ "RabbitMQ 3-Management Image"
[2]: https://www.rabbitmq.com/resources/specs/amqp0-9-1.pdf
[3]: https://www.rabbitmq.com/resources/specs/amqp0-9-1.xml
[4]: https://www.rabbitmq.com/amqp-0-9-1-reference.html

