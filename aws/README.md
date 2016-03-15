How to benchmark on EC2
=======================

1. Install everything on your coordinator machine somewhere
2. Run `aws config`
3. Get your .pem key file into this directory
4. For each of the three `stadium_*.py` scripts, make sure all of the configuration options between `# BEGIN CONFIGURABLE` and `# END CONFIGURABLE` are correct
5. Install the requirements in `requirements.txt`
6. Push the commit you want to benchmark to the branch `benchmark` in the github repo

The scripts
-----------
Make sure to run these all from the coordinator machine

- `stadium_init.py` will launch a fleet of the specified instance. When it returns you should be able to start using it
- `stadium_test.py` will connect to the sole running fleet (it will fail if there are more than 1/none) and run the test using the parameters in the file
- `stadium_halt.py` will shut down the sole running fleet

TODO
----

- `groth` library might not be correctly installed; make sure it works for the `c4.8xlarge` target instance type. (might have to rebuild AMI? could also be lack of SSE instructions on weaker machines)
