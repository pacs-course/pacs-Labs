# The `Read -1` error

When running MPI on Docker, you may experience errors like the following:
```
Read -1, expected <some_number>, errno = 1 error
```
This is a known issue (see, e.g., [here](https://github.com/open-mpi/ompi/issues/4948)), that can be solved by typing the following command on the terminal:
```bash
export OMPI_MCA_btl_vader_single_copy_mechanism=none
```
You may add this command to your `.bashrc` file, too!
```bash
printf  "\nexport OMPI_MCA_btl_vader_single_copy_mechanism=none\n" >> ~/.bashrc
```