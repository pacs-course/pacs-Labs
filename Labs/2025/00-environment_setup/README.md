# Environment setup
For the laboratories of this course you will need two things:  

1. any text editor (VSCode, gedit, nano...)  
2. a unix/linux-based development environment, such as:  
   + any linux distribution (e.g. Debian/Ubuntu, RHEL/Fedora, Arch...)  
   + Windows + WSL (official WSL installation guide [here](https://learn.microsoft.com/en-us/windows/wsl/install), if in doubt stick to defaults)    
   + MacOS  
   
In all environments, it is useful to rely on containers for isolation, portability & reproducibility.  

In this guide we'll go through an example based on a [podman](https://podman.io/get-started) (or [docker](https://www.docker.com/get-started/)) container, built on top of Ubuntu packages and already containing a clone of the pacs-examples and pacs-Labs repositories.  
Podman is suggested in particular for Mac users, but it is a solution applicable to other environments too. In particular, podman appears to be better supported on extremely recent MacOs with respect to docker.    

We'll also see briefly a different solution, based on [apptainer](https://apptainer.org/docs/user/main/quick_start.html), more tailored to Linux or WSL users. Apptainer can be used for dockerhub hosted container images too, but we'll take the opportunity to show a possible alternative solution.     

In any case, previous year solutions ([2024](https://github.com/pacs-course/pacs-Labs/tree/main/Labs/2024/00-environment_setup), [2023](https://github.com/pacs-course/pacs-Labs/tree/main/Labs/2023/00-environment_setup)) are still supported for the 2025 Labs as well, and any material not covered here can be considered as extra reading.   


## 1. podman container

In following commands, podman can be exchanged with docker. Under Linux, in case of docker, the docker daemon should be started first, and sudo may be required.  

### Create a directory for shared files
```
mkdir -p /path/to/shared_files_directory
```

### Pull the image
```
podman pull --arch=amd64 docker://lucaformaggia/pacs-examples:latest
```

### Run the container
```
podman run -it --name pacs2025 --rm -v /path/to/shared_files_directory:/home/pacs/shared_files docker.io/lucaformaggia/pacs-examples:latest
```

### Permanent changes example - installing nano
```
podman run -it --name pacs2025 docker.io/lucaformaggia/pacs-examples:latest
sudo apt update && sudo apt install nano
logout
podman ps -a -s
podman commit <CONTAINER ID> pacs-examples-with-nano
podman rm pacs2025
podman run -it --name pacs2025 --rm -v /path/to/shared_files_directory:/home/pacs/shared_files localhost/pacs-examples-with-nano
```
### List images
```
podman images
```

### List containers
```
podman ps -a -s
```

### Clean everything
```
podman system prune --all
```


## 2. apptainer + mk modules example

### mk modules environment 

[mk modules](https://github.com/pjbaioni/mk) bundle a set of scientific libraries compiled under the same toolchain. Once installed, they provide the command module, that has several subcommands:

```
module load <module name> 
```

loads the requested module. This creates a set of environment variables storing relevant paths for that library (e.g. `mkEigenPrefix`, `mkEigenInc`, ...). Use

- `env | grep mk`  to obtain a list of all the environment variables relative to mk modules
- `module list`: to show a list of currently loaded modules
- `module avail`: to show a list of all available modules (loaded or not)
- `module --help`: to show a list of all the commands

### Apptainer example (linux)

[Apptainer](https://apptainer.org/docs/user/main/quick_start.html) is a container platform aimed at High-Performance Computing cluster, where it is more commonly found, possibly as [singularity](https://en.wikipedia.org/wiki/Singularity_(software)). 

### create a directory for temporary files
This step is useful to build large containers  
```
mkdir -p ~/containers
cd containers
mkdir -p ~/containers/temp
```

### pull the container, using the new temp directory 
```
export APPTAINER_TMPDIR=~/containers/temp && export APPTAINER_CACHEDIR=~/containers/cache && apptainer pull docker://quay.io/pjbaioni/mk:2024
```

### launch the container
```
apptainer shell mk_2024.sif
```

### activate and load base modules
```
source /u/sw/etc/bash.bashrc
module load gcc-glibc
module load eigen
```

## 3. Test
### Create a file test.cpp
```
#include <Eigen/Eigen>
#include <iostream>

int main(int argc, char** argv)
{
  std::cout << "Successfully included Eigen." << std::endl;
  return 0;
}
```
In case of podman/docker, it is better to store it in the shared folder.   

### podman container (ubuntu example)
Test the installation with
```
podman run -it --name pacs2025 --rm -v /path/to/shared_files_directory:/home/pacs/shared_files docker.io/lucaformaggia/pacs-examples:latest
g++ -std=c++20 -Wall -I $mkEigenInc shared_files/test.cpp  -o test && ./test && rm ./test
```

### apptainer container (mk example)
Test the installation with
```
apptainer shell /path/to/mk_2024.sif
source /u/sw/etc/bash.bashrc
module load gcc-glibc
module load eigen
g++ -std=c++20 -Wall -I $mkEigenInc test.cpp  -o test && ./test && rm ./test
```
