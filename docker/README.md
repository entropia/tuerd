# Dockercontainer compiling and running the Entropia Gate configuration

## Usage
```bash
docker build -t gatetooling .
docker run -i --privileged --device=/path/to/device gatetooling:latest /bin/bash 
./deploy_club $USERNAME
```
