# Grandet
A Unified, Economical Object Store for Web Applications.

# Release Notes
We have removed some directories.
## examples/
We removed the examples directories because there may be license conflicts.
## include/boost
We removed this directory. Originally we put boost threadpool headers there.

# Build
## To build Grandet server
* Create `build` directory
* run `cmake ..`
* run `make`
* run `server/server`

## To build Grandet SDK
* run `make sdk` in the build directory
* Find the SDKs in build/sdk

# Install
Use `setup.sh` to install grandet on a new machine. This would automatically install dependencies and configure grandet.

# Usage
* The `profiler` grabs the profiles of different storage options and saves them into `profile.json`.
* `console` provides control to the Grandet server and the underlying key-value storage.

# Extension
* To support a new storage option, inherit `Actor` class and implement the methods.
* To create a new prediction algorithm, inherit `Estimator` class and implement the methods.
* To create SDK for a new language, implement a protocol buffer wrapper and put it in the `sdk` directory.

# Note
Our paper is at http://dl.acm.org/citation.cfm?id=2987562. Please contact [Yang Tang](https://github.com/ytang) if you have any questions. Thanks!


