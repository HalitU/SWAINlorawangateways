# SWAIN_lorawan_gateways

There are two different code bases available in this repository which are provided for paper reviewers to check if they require to do so.

One is for running the clustering process for gateway locations. The sphere-sphere-earth intersection calculations are taken from https://gis.stackexchange.com/questions/48937/calculating-intersection-of-two-circles and used as a base to calculate intersections and further calculating the gateway locations.

Other is for running the NS-3 simulation environment used in our paper. Simply place that code under scratch directory under NS-3 main directory and run the code. This step might require a bit ns-3 basic knowledge beforehand.

I have also included some bash scripts to give a general idea about how some code flows work.

Since this repo is only for reference purposes I will not delve into detailed explanations.