# docker_wrapper
A wrapper for docker to work on supervisord. It stops the docker container whenever supervisord kills it.

This program launches a docker container (or actually any other command) and waits while it is executed.
If it receives SIGTERM, it will run [DOCKER_EXECUTABLE stop CONTAINER_NAME].
If it receives further SIGTERMs, it will run [DOCKER_EXECUTABLE kill CONTAINER_NAME].

It's purpose is to be a wrapper for docker so it works as expected on supervisorctl, upstart, systemd and similar tools.

It is compiled statically so you can copy it's executable and it should work anywhere on the same plataform.

#usage:

```
usage:   docker_wrapper [-v] DOCKER_EXECUTABLE [OPTIONS] run [arg...] --name CONTAINER_NAME [arg...]
example: docker_wrapper -v docker run --rm -t -i --name CONTAINER_NAME ubuntu /bin/bash
```

#sample usage:

```
root@batman:~# docker_wrapper -v docker run --rm -t -i --name CONTAINER_NAME ubuntu /bin/bash
Launching container with name [CONTAINER_NAME]. Type [kill 21729] to [docker stop] the container. Type again to [docker kill] it.
root@39734841bb3f:/# 

# [killall docker_wrapper] is typed on a different terminal

Received SIGTERM. Running [docker stop CONTAINER_NAME]
Docker returned with exit status: [0]
root@batman:~#
```

# comments
marcos AT unitron DOT com DOT br

# license: MIT

